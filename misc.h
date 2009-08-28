#ifndef _MISC_H_
#define _MISC_H_

#include "port.h"

START_EXTERN_C
extern void memcpy16(unsigned short *dest, unsigned short *src, int count);
extern void memcpy16bswap(unsigned short *dest, void *src, int count);
extern void memcpy32(uint32_t *dest, uint32_t *src, int count);
extern void memset32(uint32_t *dest, int c, int count);
END_EXTERN_C

#endif
