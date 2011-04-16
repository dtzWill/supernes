#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <zlib.h>

#include "snes9x.h"
#include "hacks.h"
#include "memmap.h"

#define kLineBufferSize 4095

static inline unsigned long parseCrc32(const char * s)
{
	return strtoul(s, 0, 16);
}

static unsigned long getGameCrc32()
{
	unsigned long crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, ROM, Memory.CalculatedSize);
	return crc;
}

static int loadHacks(char * line)
{
	int count = 0;
	bool end_of_line = false;
	char *pos = strchr(line, '|'), *start = line;
	// Title[start..pos]
	*pos = '\0';
	printf("Hacks: detected \"%s\"\n", start);

	start = pos + 1;
	pos = strchr(start, '|');
	if (!pos) {
		// If there are no flags, jump directly to hacks
		goto parse_hacks;
	}
	// Skip: Flags1[start..pos]

	start = pos + 1;
	pos = strchr(start, '|');
	if (!pos) return -1;
	// Skip: Flags2[start..pos]

	start = pos + 1;
	pos = strchr(start, '|');
	if (!pos) return -1;
	// Skip: Autoscroll1[start..pos]

	start = pos + 1;
	pos = strchr(start, '|');
	if (!pos) return -1;
	// Skip: Autoscroll2[start..pos]

	start = pos + 1;
	pos = strchr(start, '|');
	if (!pos) return -1;
	// Skip: Scale[start..pos]

	start = pos + 1;
	pos = strchr(start, '|');
	// Skip: Offset[start..pos|end_of_line]

	if (!pos) return 0; // No patches!

	start = pos + 1;

parse_hacks:
	do {
		char *end;
		unsigned long addr;
		int len;

		//start is at the "address to modify" string start

		pos = strchr(start, '=');
		if (!pos) return -1;

		addr = strtoul(start, &end, 16);
		if (end != pos) return -1;

		start = pos + 1; //start is at the "bytes to modify" string start
		pos = strchr(start, ',');
		if (pos) {
			len = (pos - start) / 2;
		} else {
			len = (strlen(start) - 1) / 2;
			end_of_line = true;
		}

		if (Settings.HacksFilter) {
			bool accept = false;
			// Only accept patches which contain opcode 42
			pos = start;
			for (int i = 0; i < len; i++) {
				if (pos[0] == '4' && pos[1] == '2') {
					accept = true;
					break;
				}
				pos += 2;
			}

			if (!accept) {
				start = pos + 1; // Go to end of individual patch
				continue; // Skip this hack.
			}
		}

		pos = start;
		for (int i = 0; i < len; i++) {
			char valStr[3] = { pos[0], pos[1], '\0' };
			unsigned char val = strtoul(valStr, 0, 16);

#ifdef DEBUG
			printf("ROM[0x%lx..0x%lx]=0x%hhx 0x%hhx 0x%hhx\n", 
				addr + i - 1, addr + i + 1,
				ROM[addr + i - 1], ROM[addr + i], ROM[addr + i + 1]);
			printf("--> ROM[0x%lx]=0x%hhx\n", addr + i, val);
#endif
			ROM[addr + i] = val;

			count++;
			pos += 2;
		}

		start = pos + 1;
	} while (!end_of_line);

	return count;
}

void S9xHacksLoadFile(const char * file)
{
	unsigned long gameCrc;
	char * line;
	FILE * fp;

	if (!Settings.HacksEnabled) goto no_hacks;
	if (!file) goto no_hacks;

	// At this point, the ROM is already loaded.
	fp = fopen(file, "r");
	if (!fp) {
		fprintf(stderr, "Can't open hacks file %s: %s\n", file, strerror(errno));
		goto no_hacks;
	}

	// Get current ROM CRC
	gameCrc = getGameCrc32();

	line = (char*) malloc(kLineBufferSize + 1);
	do {
		fgets(line, kLineBufferSize, fp);

		char *pos = strchr(line, '|');
		if (!pos) continue;
		*pos = '\0';

		if (gameCrc == parseCrc32(line)) {
			// Hit! This line's CRC matches our current ROM CRC.
			int res = loadHacks(pos + 1);
			if (res > 0) {
				printf("Hacks: searched %s for crc %lX, %d byte%s patched\n",
					file, gameCrc, res, (res == 1 ? "" : "s"));
			} else if (res < 0) {
				printf("Hacks: searched %s for crc %lX, error parsing line\n",
					file, gameCrc);
			} else {
				printf("Hacks: searched %s for crc %lX, no hacks\n",
					file, gameCrc);
			}
			goto hacks_found;
		}
	} while (!feof(fp) && !ferror(fp));

	if (ferror(fp)) {
		fprintf(stderr, "Error reading hacks file: %s\n", file);
	}

	printf("Hacks: searched %s for crc %lX; nothing found\n", file, gameCrc);

hacks_found:
	free(line);
	fclose(fp);

	return;
no_hacks:
	printf("Hacks: disabled\n");
}

