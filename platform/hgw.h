#ifndef _PLATFORM_HGW_H_
#define _PLATFORM_HGW_H_

#ifdef __cplusplus
extern bool hgwLaunched;

void HgwInit();
void HgwDeinit();
void HgwConfig();
void HgwPollEvents();
#endif

#define kGConfPath "/apps/maemo/drnoksnes/"
#define kGConfRomFile kGConfPath "rom"
#define kGConfDisableAudio kGConfPath "no_audio"
#define kGConfTurboMode kGConfPath "turbo"
#define kGConfFrameskip kGConfPath "frameskip"
#define kGConfTransparency kGConfPath "transparency"
#define kGConfSpeedhacks kGConfPath "speedhacks"
#define kGConfMapping kGConfPath "mapping"
#define kGConfKeysPath kGConfPath "keys/"

#endif

