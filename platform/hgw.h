#ifndef _PLATFORM_HGW_H_
#define _PLATFORM_HGW_H_

#ifdef __cplusplus
/** True if we were launched from GUI. */
extern bool hgwLaunched;

/** Called from main() before loading config; connects to DBus. */
void HgwInit();
/** Called from main() before closing. */
void HgwDeinit();
/** Called from main() after loading user config; loads GUI settings. */
void HgwConfig();
/** Called from main() in the event loop; polls DBus. */
void HgwPollEvents();
#endif

#define kGConfPath "/apps/maemo/drnoksnes"
#define kGConfRomFile kGConfPath "/" "rom"
#define kGConfSound kGConfPath "/" "sound"
#define kGConfTurboMode kGConfPath "/" "turbo"
#define kGConfFrameskip kGConfPath "/" "frameskip"
#define kGConfTransparency kGConfPath "/" "transparency"
#define kGConfScaler kGConfPath "/" "scaler"
#define kGConfDisplayFramerate kGConfPath "/" "display-framerate"
#define kGConfDisplayControls kGConfPath "/" "display-controls"
#define kGConfSpeedhacks kGConfPath "/" "speedhacks"
#define kGConfMapping kGConfPath "/" "mapping"
#define kGConfKeysPath kGConfPath "/" "keys"


#endif

