#ifndef _PLATFORM_OSSO_H_
#define _PLATFORM_OSSO_H_

#include <libosso.h>

START_EXTERN_C

extern osso_context_t *ossoContext;

static inline int OssoOk()
{
	return ossoContext != NULL;
}

/** Called from main() before loading config; connects to DBus. */
void OssoInit();
/** Called from main() before closing. */
void OssoDeinit();
/** Called from main() after loading user config; loads GUI settings. */
void OssoConfig();
/** Called from main() in the event loop; polls DBus. */
void OssoPollEvents();

END_EXTERN_C

#endif

