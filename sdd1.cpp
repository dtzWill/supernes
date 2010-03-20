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

#include <stdio.h>
#include <dirent.h>

#include "snes9x.h"
#include "memmap.h"
#include "ppu.h"
#include "sdd1.h"
#include "display.h"

static int S9xCompareSDD1IndexEntries (const void *p1, const void *p2)
{
    return (*(uint32 *) p1 - *(uint32 *) p2);
}

static int S9xGetSDD1Dir(char * packdir)
{
	char dir[_MAX_DIR + 1];
	char drive[_MAX_DRIVE + 1];
	char name[_MAX_FNAME + 1];
	char ext[_MAX_EXT + 1];

	PathSplit(S9xGetFilename(FILE_ROM), drive, dir, name, ext);

	if (strncmp(Memory.ROMName, "Star Ocean", 10) == 0) {
        PathMake(packdir, drive, dir, "socnsdd1", 0);
        return 1;
	} else if(strncmp(Memory.ROMName, "STREET FIGHTER ALPHA2", 21) == 0) {
		PathMake(packdir, drive, dir, "sfa2sdd1", 0);
		return 1;
	} else {
		S9xMessage(S9X_WARNING, S9X_ROM_INFO,
			"WARNING: No default SDD1 pack for this ROM");
		return 0;
	}
}

void S9xLoadSDD1Data ()
{
	char packdir[_MAX_PATH + 1];

	// Unload any previous pack
	Settings.SDD1Pack = FALSE;
	Memory.FreeSDD1Data();

	if (!S9xGetSDD1Dir(packdir)) {
		printf("SDD1: Didn't found pack for this ROM\n");
		return;
	}

	printf("SDD1: Searching for pack in %s\n", packdir);
	Settings.SDD1Pack=TRUE;

	char index[_MAX_PATH + 1];
	char data[_MAX_PATH + 1];
	char patch[_MAX_PATH + 1];
	DIR *dir = opendir(packdir);

	index[0] = 0;
	data[0] = 0;
	patch[0] = 0;

	if (dir) {
		struct dirent *d;

		while ((d = readdir (dir))) {
			if (strcasecmp (d->d_name, "SDD1GFX.IDX") == 0) {
				strcpy(index, packdir);
				strcat(index, "/");
				strcat(index, d->d_name);
			} else if (strcasecmp (d->d_name, "SDD1GFX.DAT") == 0) {
				strcpy(data, packdir);
				strcat(data, "/");
				strcat(data, d->d_name);
			} else if (strcasecmp (d->d_name, "SDD1GFX.PAT") == 0) {
				strcpy(patch, packdir);
				strcat(patch, "/");
				strcat(patch, d->d_name);
			}
		}
		closedir (dir);
	}

	if (strlen (index) && strlen (data)) {
		FILE *fs = fopen (index, "rb");
		int len = 0;

		if (fs)	{
			// Index is stored as a sequence of entries, each entry being
			// 12 bytes consisting of:
			// 4 byte key: (24bit address & 0xfffff * 16) | translated block
			// 4 byte ROM offset
			// 4 byte length

			fseek (fs, 0, SEEK_END);
			len = ftell (fs);
			rewind (fs);
			Memory.SDD1Index = (uint8 *) malloc (len);
			fread (Memory.SDD1Index, 1, len, fs);
			fclose (fs);
			Memory.SDD1Entries = len / 12;
		} else {
			fprintf(stderr, "Failed to read SDD1 index file %s\n", index);
			return;
		}
		printf("SDD1: index: %s\n", PathBasename(index));

		if (!(fs = fopen (data, "rb")))	{
			fprintf(stderr, "Failed to read SDD1 data file %s\n", data);
			free ((char *) Memory.SDD1Index);
			Memory.SDD1Index = NULL;
			Memory.SDD1Entries = 0;
			return;
		} else {
			fseek (fs, 0, SEEK_END);
			len = ftell (fs);
			rewind (fs);
			Memory.SDD1Data = (uint8 *) malloc (len);
			fread (Memory.SDD1Data, 1, len, fs);
			fclose (fs);
		}
		printf("SDD1: data pack: %s\n", PathBasename(data));

		if (strlen (patch) > 0 && (fs = fopen (patch, "rb"))) {
			fclose (fs);
		}

#ifdef MSB_FIRST
		// Swap the byte order of the 32-bit value triplets on
		// MSBFirst machines.
		uint8 *ptr = Memory.SDD1Index;
		for (int i = 0; i < Memory.SDD1Entries; i++, ptr += 12) 	{
			SWAP_DWORD ((*(uint32 *) (ptr + 0)));
			SWAP_DWORD ((*(uint32 *) (ptr + 4)));
			SWAP_DWORD ((*(uint32 *) (ptr + 8)));
		}
#endif

		qsort(Memory.SDD1Index, Memory.SDD1Entries, 12,
			S9xCompareSDD1IndexEntries);
		printf("SDD1: Pack loaded succesfully\n");
	} else {
		fprintf(stderr, "SDD1: SDD1 data pack not found in '%s'\n",
			packdir);
		fprintf(stderr, "SDD1: Check if sdd1gfx files exist\n");
		printf("SDD1: Failed to load pack\n");
	}
}

void S9xSetSDD1MemoryMap (uint32 bank, uint32 value)
{
    bank = 0xc00 + bank * 0x100;
    value = value * 1024 * 1024;

    int c;

    for (c = 0; c < 0x100; c += 16)
    {
	uint8 *block = &Memory.ROM [value + (c << 12)];
	int i;

	for (i = c; i < c + 16; i++)
	    Memory.Map [i + bank] = block;
    }
}

void S9xResetSDD1 ()
{
    memset (&Memory.FillRAM [0x4800], 0, 4);
    for (int i = 0; i < 4; i++)
    {
	Memory.FillRAM [0x4804 + i] = i;
	S9xSetSDD1MemoryMap (i, i);
    }
}

void S9xSDD1PostLoadState ()
{
    for (int i = 0; i < 4; i++)
	S9xSetSDD1MemoryMap (i, Memory.FillRAM [0x4804 + i]);
}

#ifndef _SNESPPC
static int S9xCompareSDD1LoggedDataEntries (const void *p1, const void *p2)
#else
static int _cdecl S9xCompareSDD1LoggedDataEntries (const void *p1, const void *p2)
#endif
{
    uint8 *b1 = (uint8 *) p1;
    uint8 *b2 = (uint8 *) p2;
    uint32 a1 = (*b1 << 16) + (*(b1 + 1) << 8) + *(b1 + 2);
    uint32 a2 = (*b2 << 16) + (*(b2 + 1) << 8) + *(b2 + 2);

    return (a1 - a2);
}

void S9xSDD1SaveLoggedData ()
{
    if (Memory.SDD1LoggedDataCount != Memory.SDD1LoggedDataCountPrev)
    {
	qsort (Memory.SDD1LoggedData, Memory.SDD1LoggedDataCount, 8,
	       S9xCompareSDD1LoggedDataEntries);

	const char * sdd1_dat_file = S9xGetFilename(FILE_SDD1_DAT);
	FILE *fs = fopen (sdd1_dat_file, "wb");

	if (fs)
	{
	    fwrite (Memory.SDD1LoggedData, 8,
		    Memory.SDD1LoggedDataCount, fs);
	    fclose (fs);
#if defined(__linux)
	    chown (sdd1_dat_file, getuid (), getgid ());
#endif
	}
	Memory.SDD1LoggedDataCountPrev = Memory.SDD1LoggedDataCount;
    }
}

void S9xSDD1LoadLoggedData ()
{
    FILE *fs = fopen (S9xGetFilename(FILE_SDD1_DAT), "rb");

    Memory.SDD1LoggedDataCount = Memory.SDD1LoggedDataCountPrev = 0;

    if (fs)
    {
	int c = fread (Memory.SDD1LoggedData, 8, 
		       MEMMAP_MAX_SDD1_LOGGED_ENTRIES, fs);

	if (c != EOF)
	    Memory.SDD1LoggedDataCount = Memory.SDD1LoggedDataCountPrev = c;
	fclose (fs);
    }
}
