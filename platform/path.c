#include <string.h>

#include "port.h"

void PathMake(char *path, const char *drive, const char *dir,
	const char *fname, const char *ext)
{
	if (dir && *dir) {
		strcpy (path, dir);
		strcat (path, "/");
	} else {
		*path = 0;
	}
	strcat (path, fname);
	if (ext && *ext) {
		strcat (path, ".");
		strcat (path, ext);
	}
}

void PathSplit(const char *path, char *drive, char *dir, char *fname, char *ext)
{
	*drive = 0;

	char *slash = strrchr (path, '/');
	if (!slash)
		slash = strrchr (path, '\\');

	char *dot = strrchr (path, '.');

	if (dot && slash && dot < slash)
		dot = NULL;

	if (!slash)
	{
		strcpy (dir, "");
		strcpy (fname, path);
		if (dot)
		{
			*(fname + (dot - path)) = 0;
			strcpy (ext, dot + 1);
		}
		else
			strcpy (ext, "");
	}
	else
	{
		strcpy (dir, path);
		*(dir + (slash - path)) = 0;
		strcpy (fname, slash + 1);
		if (dot)
		{
			*(fname + (dot - slash) - 1) = 0;
			strcpy (ext, dot + 1);
		}
		else
			strcpy (ext, "");
	}
}

const char * PathBasename(const char * path)
{
	const char * p = strrchr (path, '/');

	if (p)
		return p + 1;

	return path;
}

