#ifndef _PLATFORM_GUICONF_H_
#define _PLATFORM_GUICONF_H_

START_EXTERN_C

/** Called from main() before loading config; connects to DBus. */
void GuiConfInit();
/** Called from main() before closing. */
void HgwDeinit();
/** Called from main() after loading user config; loads GUI settings. */
void HgwConfig();
/** Called from main() in the event loop; polls DBus. */
void GuiConfigPollEvents();

END_EXTERN_C

#endif

