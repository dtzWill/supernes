/*
 * Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 *
 * (c) Copyright 1996 - 2001 Gary Henderson (gary.henderson@ntlworld.com) and
 *                           Jerremy Koot (jkoot@snes9x.com)
 *
 * Super FX C emulator code 
 * (c) Copyright 1997 - 1999 Ivar (ivar@snes9x.com) and
 *                           Gary Henderson.
 * Super FX assembler emulator code (c) Copyright 1998 zsKnight and _Demo_.
 *
 * DSP1 emulator code (c) Copyright 1998 Ivar, _Demo_ and Gary Henderson.
 * C4 asm and some C emulation code (c) Copyright 2000 zsKnight and _Demo_.
 * C4 C code (c) Copyright 2001 Gary Henderson (gary.henderson@ntlworld.com).
 *
 * DOS port code contains the works of other authors. See headers in
 * individual files.
 *
 * Snes9x homepage: http://www.snes9x.com
 *
 * Permission to use, copy, modify and distribute Snes9x in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Snes9x is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Snes9x or software derived from Snes9x.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so everyone can benefit from the modifications
 * in future versions.
 *
 * Super NES and Super Nintendo Entertainment System are trademarks of
 * Nintendo Co., Limited and its subsidiary companies.
 */
#ifndef _PORT_H_
#define _PORT_H_

/*
This port.h is really a right-of-passage for anyone trying to port the emulator 
	to another platform.  It must have started out as a set of defines for a
	single platform, and instead of using define blocks as new platforms were
	added, individual coders simply added exceptions and sprinkled #ifdef and #ifndef
	statements throughout the original list.

I can't take it anymore, it's too convoluted.  So I've commented out the entire
	section, and preemptively rewritten the first #define segment the way god intended,
	with a single define-block for each target platform.
*/

//Title
#define TITLE "DrNokSnes"

//Required Includes
#include "pixform.h"
#include <zlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>

//Types Defined
typedef uint8_t			bool8;
typedef uint8_t			uint8;
typedef uint16_t		uint16;
typedef uint32_t		uint32;
typedef int8_t			int8;
typedef int16_t			int16;
typedef int32_t			int32;
typedef int64_t			int64;

//For Debugging Purposes:

typedef uint8_t			bool8_32;
typedef uint8_t			uint8_32;
typedef uint16_t		uint16_32;
typedef int8_t			int8_32;
typedef int16_t			int16_32;

//Defines for Extern C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#define START_EXTERN_C EXTERN_C {
#define END_EXTERN_C }
#else
#define EXTERN_C extern
#define START_EXTERN_C
#define END_EXTERN_C
#endif

//Path Defines
#define _MAX_DIR PATH_MAX
#define _MAX_DRIVE 1
#define _MAX_FNAME NAME_MAX
#define _MAX_EXT NAME_MAX
#define _MAX_PATH PATH_MAX

// Boolean constants (may already be defined)
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Config -> Defines
#if CONF_BUILD_ASM_SPC700
#define ASM_SPC700		1
#else
#undef ASM_SPC700
#endif

// Configuration defines I think I know what they're for
#define SUPER_FX		1
#define USE_SA1			1
#define CPU_SHUTDOWN	1
//#define NETPLAY_SUPPORT	1
#define ZLIB			1
#define UNZIP_SUPPORT	1
#define NO_INLINE_SET_GET 1

//Misc Items
#define VAR_CYCLES
//#define SPC700_SHUTDOWN
#define LSB_FIRST
#define PIXEL_FORMAT RGB565
#define CHECK_SOUND()
#define ZeroMemory(a,b) memset((a),0,(b))
#define PACKING __attribute__ ((packed))
#define ALIGN_BY_ONE  __attribute__ ((aligned (1), packed))
#define LSB_FIRST
#undef  FAST_LSB_WORD_ACCESS

// Language abstractions
#define FASTCALL
#define STATIC static
#define INLINE inline

START_EXTERN_C
// Path functions
void PathMake(char *path, const char *drive, const char *dir,
	const char *fname, const char *ext);
void PathSplit(const char *path, char *drive, char *dir, char *fname, char *ext);
/** A simplified basename function returning a pointer inside the src string */
const char * PathBasename(const char * path);
END_EXTERN_C

// Stream functions, used when opening ROMs and snapshots.
#ifdef ZLIB
#include <zlib.h>
#define STREAM gzFile
#define READ_STREAM(p,l,s) gzread (s,p,l)
#define WRITE_STREAM(p,l,s) gzwrite (s,p,l)
#define GETS_STREAM(p,l,s) gzgets(s,p,l)
#define GETC_STREAM(s) gzgetc(s)
#define OPEN_STREAM(f,m) gzopen (f,m)
#define REOPEN_STREAM(f,m) gzdopen (f,m)
#define FIND_STREAM(f)	gztell(f)
#define REVERT_STREAM(f,o,s)  gzseek(f,o,s)
#define CLOSE_STREAM(s) gzclose (s)
#else
#define STREAM FILE *
#define READ_STREAM(p,l,s) fread (p,1,l,s)
#define WRITE_STREAM(p,l,s) fwrite (p,1,l,s)
#define GETS_STREAM(p,l,s) fgets(p,l,s)
#define GETC_STREAM(s) fgetc(s)
#define OPEN_STREAM(f,m) fopen (f,m)
#define REOPEN_STREAM(f,m) fdopen (f,m)
#define FIND_STREAM(f)	ftell(f)
#define REVERT_STREAM(f,o,s)	 fseek(f,o,s)
#define CLOSE_STREAM(s) fclose (s)
#endif

#endif

