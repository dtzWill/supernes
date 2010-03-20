#include <stdio.h>

#include "snes9x.h"

#include <glib.h>
#include <libosso.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "platform.h"
#include "osso.h"
#include "../gui/gconf.h"

static GMainContext *mainContext;
static GMainLoop *mainLoop;
osso_context_t *ossoContext;

// Older versions of glib don't have this.
#ifndef g_warn_if_fail
#define g_warn_if_fail(expr) \
	if G_UNLIKELY(expr) { \
		g_warning("Non critical assertion failed at %s:%d \"%s\"", \
			__FILE__, __LINE__, #expr); \
	}
#endif
#if ! GLIB_CHECK_VERSION(2,14,0)
#define g_timeout_add_seconds(interval, function, data) \
	g_timeout_add((interval) * 1000, function, data)
#endif

static volatile enum {
	STARTUP_COMMAND_INVALID = -1,
	STARTUP_COMMAND_UNKNOWN = 0,
	STARTUP_COMMAND_RUN,
	STARTUP_COMMAND_CONTINUE,
	STARTUP_COMMAND_RESTART,
	STARTUP_COMMAND_QUIT
} startupCommand;

static void loadSafeKeymap();
static void loadPlayer1Keymap(GConfClient* gcc);
static void loadPlayer2Keymap(GConfClient* gcc);

/** The dbus application service callback. Usually called by the launcher only. */
static gint ossoAppCallback(const gchar *interface, const gchar *method,
  GArray *arguments, gpointer data, osso_rpc_t *retval)
{
	retval->type = DBUS_TYPE_BOOLEAN;

	if (startupCommand == STARTUP_COMMAND_UNKNOWN) {
		// Only if we haven't received the startup command yet.
		printf("Osso: Startup method is: %s\n", method);

		if (strcmp(method, "game_run") == 0) {
			startupCommand = STARTUP_COMMAND_RUN;
			retval->value.b = TRUE;
		} else if (strcmp(method, "game_continue") == 0) {
			startupCommand = STARTUP_COMMAND_CONTINUE;
			retval->value.b = TRUE;
		} else if (strcmp(method, "game_restart") == 0) {
			startupCommand = STARTUP_COMMAND_RESTART;
			retval->value.b = TRUE;
		} else if (strcmp(method, "game_close") == 0) {
			// A bit weird, but could happen
			startupCommand = STARTUP_COMMAND_QUIT;
			retval->value.b = TRUE;
		} else {
			startupCommand = STARTUP_COMMAND_INVALID;
			retval->value.b = FALSE;
		}
	} else {
		if (strcmp(method, "game_close") == 0) {
			printf("Osso: quitting because of D-Bus close message\n");
			S9xDoAction(kActionQuit);
			retval->value.b = TRUE;
		} else {
			retval->value.b = FALSE;
		}
	}

	return OSSO_OK;
}

static gboolean ossoTimeoutCallback(gpointer data)
{
	if (startupCommand == STARTUP_COMMAND_UNKNOWN) {
		// Assume that after N seconds we're not going to get a startup reason.
		startupCommand = STARTUP_COMMAND_INVALID;
	}

	return FALSE; // This is a timeout, don't call us ever again.
}

static void ossoHwCallback(osso_hw_state_t *state, gpointer data)
{
	if (state->shutdown_ind) {
		// Shutting down. Try to quit gracefully.
		S9xDoAction(kActionQuit);
	}
	if (Config.saver && state->system_inactivity_ind) {
		// Screen went off, and power saving is active.
		S9xDoAction(kActionQuit);
	}
}

/** Called from main(), initializes Glib & libosso stuff if needed. */
void OssoInit()
{
	char *dbusLaunch = getenv("DRNOKSNES_DBUS");

	if (!dbusLaunch || dbusLaunch[0] != 'y') {
		// Not launched from GUI, so we don't assume GUI features.
		ossoContext = 0;
		return;
	}

	g_type_init();
	g_set_prgname("drnoksnes");
	g_set_application_name("DrNokSnes");
	mainContext = g_main_context_default();
	mainLoop = g_main_loop_new(mainContext, FALSE);
	ossoContext = osso_initialize("com.javispedro.drnoksnes", "1", 0, 0);

	if (!ossoContext) {
		fprintf(stderr, "Error initializing libosso\n");
		exit(2);
	}

	// At this point, we still don't know what the startup command is
	startupCommand = STARTUP_COMMAND_UNKNOWN;

	osso_return_t ret;
	ret = osso_rpc_set_default_cb_f(ossoContext, ossoAppCallback, 0);
	g_warn_if_fail(ret == OSSO_OK);

	osso_hw_state_t hwStateFlags = { FALSE };
	hwStateFlags.shutdown_ind = TRUE;
	hwStateFlags.system_inactivity_ind = TRUE;
	ret = osso_hw_set_event_cb(ossoContext, &hwStateFlags, ossoHwCallback, 0);
	g_warn_if_fail(ret == OSSO_OK);

	printf("Osso: Initialized libosso\n");
}

static osso_return_t invokeLauncherMethod(const char *method, osso_rpc_t *retval)
{
	// The launcher seems to assume there is at least one parameter,
	// even if the method doesn't actually require one.
	return osso_rpc_run(ossoContext, "com.javispedro.drnoksnes.startup",
		"/com/javispedro/drnoksnes/startup", "com.javispedro.drnoksnes.startup",
		method, retval, DBUS_TYPE_INVALID);
}

void OssoDeinit()
{
	if (!OssoOk()) return;

	// Send a goodbye message to the launcher
	osso_return_t ret;
	osso_rpc_t retval = { 0 };
	if (Config.snapshotSave) {
		// If we saved game state, notify launcher to enter "paused" status.
		ret = invokeLauncherMethod("game_pause", &retval);
	} else {
		ret = invokeLauncherMethod("game_close", &retval);
	}
	if (ret != OSSO_OK) {
		printf("Osso: failed to notify launcher\n");
	}
	osso_rpc_free_val(&retval);

	osso_deinitialize(ossoContext);
	g_main_loop_unref(mainLoop);
	g_main_context_unref(mainContext);

	ossoContext = 0;
}

/** Called after loading the config file, loads settings from gconf. */
void OssoConfig()
{
	if (!OssoOk()) return;

	GConfClient *gcc = gconf_client_get_default();

	// GUI only allows fullscreen
	Config.fullscreen = true;

	// Get ROM filename from Gconf
	gchar *romFile = gconf_client_get_string(gcc, kGConfRomFile, 0);
	if (romFile && strlen(romFile) > 0) {
		S9xSetRomFile(romFile);
	} else {
		printf("Exiting gracefully because there's no ROM in Gconf\n");
		OssoDeinit();
		exit(0);
	}

	// Read most of the non-player specific settings
	Config.saver = gconf_client_get_bool(gcc, kGConfSaver, 0);
	Config.enableAudio = gconf_client_get_bool(gcc, kGConfSound, 0);
	Settings.TurboMode = gconf_client_get_bool(gcc, kGConfTurboMode, 0);
	Settings.Transparency = gconf_client_get_bool(gcc, kGConfTransparency, 0);
	Settings.DisplayFrameRate = gconf_client_get_bool(gcc, kGConfDisplayFramerate, 0);

	int frameskip = gconf_client_get_int(gcc, kGConfFrameskip, 0);
	Settings.SkipFrames = (frameskip > 0 ? frameskip : AUTO_FRAMERATE);

	gchar *scaler = gconf_client_get_string(gcc, kGConfScaler, 0);
	if (scaler && strlen(scaler) > 0) {
		free(Config.scaler);
		Config.scaler = strdup(scaler);
	}
	g_free(scaler);

	int speedhacks = gconf_client_get_int(gcc, kGConfSpeedhacks, 0);
	if (speedhacks <= 0) {
		Settings.HacksEnabled = FALSE;
		Settings.HacksFilter = FALSE;
	} else if (speedhacks == 1) {
		Settings.HacksEnabled = TRUE;
		Settings.HacksFilter = TRUE;
	} else {
		Settings.HacksEnabled = TRUE;
		Settings.HacksFilter = FALSE;
	}

	if (Settings.HacksEnabled && !Config.hacksFile) {
		// Provide a default speedhacks file
		gchar *romDir = g_path_get_dirname(romFile);
		if (asprintf(&Config.hacksFile, "%s/snesadvance.dat", romDir)
				< 0) {
			Config.hacksFile = 0; // malloc error.
		}
		g_free(romDir);
	}

	g_free(romFile);

	// Read player 1 controls
	gchar key[kGConfPlayerPathBufferLen];
	gchar *relKey = key + sprintf(key, kGConfPlayerPath, 1);

	//  keyboard
	strcpy(relKey, kGConfPlayerKeyboardEnable);
	if (gconf_client_get_bool(gcc, key, NULL)) {
		Config.joypad1Enabled = true;
		loadPlayer1Keymap(gcc);
	} else {
		// We allow controls to be enabled from the command line
		loadSafeKeymap();
	}

	//  touchscreen
	strcpy(relKey, kGConfPlayerTouchscreenEnable);
	if (gconf_client_get_bool(gcc, key, NULL)) {
		Config.touchscreenInput = 1;

		strcpy(relKey, kGConfPlayerTouchscreenShow);
		if (gconf_client_get_bool(gcc, key, NULL)) {
			Config.touchscreenShow = true;
		}
	}

	// Read player 2 controls
	relKey = key + sprintf(key, kGConfPlayerPath, 2);

	//  keyboard
	strcpy(relKey, kGConfPlayerKeyboardEnable);
	if (gconf_client_get_bool(gcc, key, NULL)) {
		Config.joypad2Enabled = true;
		loadPlayer2Keymap(gcc);
	}

	//  touchscreen
	strcpy(relKey, kGConfPlayerTouchscreenEnable);
	if (gconf_client_get_bool(gcc, key, NULL)) {
		Config.touchscreenInput = 2;

		strcpy(relKey, kGConfPlayerTouchscreenShow);
		if (gconf_client_get_bool(gcc, key, NULL)) {
			Config.touchscreenShow = true;
		}
	}

	// Time to read the startup command from D-Bus

	// Timeout after 3 seconds, and assume we didn't receive any.
	guint timeout = g_timeout_add_seconds(3, ossoTimeoutCallback, 0);
	g_warn_if_fail(timeout > 0);

	// Iterate the event loop since we want to catch the initial dbus messages
	while (startupCommand == STARTUP_COMMAND_UNKNOWN) {
		// This is not busylooping since we are blocking here
		g_main_context_iteration(mainContext, TRUE);
	}

	// The command we received from the launcher will tell us if we have to
	// load a snapshot file.
	switch (startupCommand) {
		case STARTUP_COMMAND_RUN:
		case STARTUP_COMMAND_RESTART:
			Config.snapshotLoad = false;
			Config.snapshotSave = true;
			break;
		case STARTUP_COMMAND_CONTINUE:
			Config.snapshotLoad = true;
			Config.snapshotSave = true;
			break;
		case STARTUP_COMMAND_QUIT:
			Config.snapshotLoad = false;
			Config.snapshotSave = false;
			Config.quitting = true;
			break;
		default:
			Config.snapshotLoad = false;
			Config.snapshotSave = false;
			break;
	}

	g_object_unref(G_OBJECT(gcc));
}

/** This is called periodically from the main loop.
	Iterates the GLib loop to get D-Bus events.
 */
void OssoPollEvents()
{
	if (!OssoOk()) return;

	g_main_context_iteration(mainContext, FALSE);
}

typedef struct ButtonEntry {
	const char * gconf_key;
	unsigned long mask;
	bool is_action;
} ButtonEntry;

/** This arrays contains generic info about each of the mappable buttons the
	GUI shows */
static const ButtonEntry buttons[] = {
#define HELP(...)
#define P(x) SNES_##x##_MASK
#define A(x) kAction##x
#define BUTTON(description, slug, actions, d, f) \
	{ G_STRINGIFY(slug), actions, false },
#define ACTION(description, slug, actions, d, f) \
	{ G_STRINGIFY(slug), actions, true },
#define LAST \
	{ 0 }
#include "../gui/buttons.inc"
#undef HELP
#undef P
#undef A
#undef BUTTON
#undef ACTION
#undef LAST
};

/** This loads a keymap for player 1 that will allow him to exit the app. */
static void loadSafeKeymap()
{
	// Map quit to fullscreen, escape and task switch.
	Config.action[72] = kActionQuit;
	Config.action[9] = kActionQuit;
	Config.action[71] = kActionQuit;
}

static void loadPlayer1Keymap(GConfClient* gcc)
{
	// Build player 1 keyboard gconf key relative path
	gchar key[kGConfPlayerPathBufferLen];
	gchar *relKey = key + sprintf(key,
		kGConfPlayerPath kGConfPlayerKeyboardPath "/", 1);

	// If the user does not map fullscreen or quit
	bool quit_mapped = false;

	printf("Hgw: Using gconf key mappings\n");
	// Thus ignoring config file key mappings
	ZeroMemory(Config.joypad1Mapping, sizeof(Config.joypad1Mapping));
	ZeroMemory(Config.action, sizeof(Config.action));

	int i, scancode;
	for (i = 0; buttons[i].gconf_key; i++) {
		strcpy(relKey, buttons[i].gconf_key);
		scancode = gconf_client_get_int(gcc, key, NULL);

		if (scancode <= 0 || scancode > 255) continue;

		if (buttons[i].is_action) {
			Config.action[scancode] |= buttons[i].mask;
			if (buttons[i].mask & (kActionQuit | kActionToggleFullscreen)) {
				quit_mapped = true;
			}
		} else {
			Config.joypad1Mapping[scancode] |= buttons[i].mask;
		}
	}

#if MAEMO && !CONF_EXIT_BUTTON
	// Safeguards
	if (!quit_mapped) {
		// Newbie user won't know how to quit game.
		if (!Config.joypad1Mapping[72] && !Config.action[72]) {
			// Fullscreen key is not mapped, map
			Config.action[72] = kActionQuit;
			quit_mapped = true;
		}
		if (!quit_mapped && !Config.joypad1Mapping[9] && !Config.action[9]) {
			// Escape key is not mapped, map
			// But only if we couldn't map quit to fullscreen. Some people
			// actually want Quit not to be mapped.
			Config.action[9] = kActionQuit;
			quit_mapped = true;
		}
		if (!quit_mapped) {
			// Force mapping of fullscreen to Quit if can't map anywhere else.
			Config.joypad1Mapping[72] = 0;
			Config.action[72] = kActionQuit;
		}
	}

	// If task switch key is not mapped, map it to Quit by default.
	if (!Config.action[71] && !Config.joypad1Mapping[71]) {
		Config.action[71] = kActionQuit;
	}
#endif
}

// This version is simpler since we don't need safeguards.
static void loadPlayer2Keymap(GConfClient* gcc)
{
	// Build player 2 keyboard gconf key relative path
	gchar key[kGConfPlayerPathBufferLen];
	gchar *relKey = key + sprintf(key,
		kGConfPlayerPath kGConfPlayerKeyboardPath "/", 2);

	// Ignore config file key mappings
	ZeroMemory(Config.joypad2Mapping, sizeof(Config.joypad2Mapping));

	int i, scancode;
	for (i = 0; buttons[i].gconf_key; i++) {
		if (buttons[i].is_action) continue;

		strcpy(relKey, buttons[i].gconf_key);
		scancode = gconf_client_get_int(gcc, key, NULL);

		// Ignore out of range values
		if (scancode <= 0 || scancode > 255) continue;

		Config.joypad2Mapping[scancode] |= buttons[i].mask;
	}
}

