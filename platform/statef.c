#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <zlib.h>

#include "port.h"

static gzFile gfd;

static int zlib_open(const char *fname, const char *mode)
{
	gfd = gzopen(fname, mode);
	if (!gfd) return 0;

	return 1;
}

static int zlib_read(void *p, int l)
{
	return gzread(gfd, p, l);
}

static int zlib_write(void *p, int l)
{
	return gzwrite(gfd, p, l);
}

static void zlib_close()
{
	gzclose(gfd);
}

int  (*statef_open)(const char *fname, const char *mode) = zlib_open;
int  (*statef_read)(void *p, int l) = zlib_read;
int  (*statef_write)(void *p, int l) = zlib_write;
void (*statef_close)() = zlib_close;

