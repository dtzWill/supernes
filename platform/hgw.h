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

#endif
