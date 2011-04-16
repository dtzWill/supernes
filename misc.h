#ifndef _MISC_H_
#define _MISC_H_

#include "port.h"

START_EXTERN_C
extern void memcpy16(void *dest, const void *src, int count);
extern void memcpy16bswap(void *dest, const void *src, int count);
extern void memcpy32(void *dest, const void *src, int count);
extern void memset32(void *dest, int c, int count);
END_EXTERN_C

#endif
