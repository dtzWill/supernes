#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

extern char **g_argv;

/* Call this MMU Hack kernel module after doing mmap, and before doing memset*/
int mmuhack(void)
{
	char kocmd[1024];
	int i, mmufd = open("/dev/mmuhack", O_RDWR);

	if(mmufd < 0) {
		strcpy(kocmd, "/sbin/insmod ");
		strncpy(kocmd+13, g_argv[0], 1023-13);
		kocmd[1023] = 0;
		for (i = strlen(kocmd); i > 0; i--)
			if (kocmd[i] == '/') { kocmd[i] = 0; break; }
		strcat(kocmd, "/mmuhack.o");

		printf("Installing NK's kernel module for Squidge MMU Hack (%s)...\n", kocmd);
		system(kocmd);
		mmufd = open("/dev/mmuhack", O_RDWR);
	}
	if(mmufd < 0) return 0;
	 
	close(mmufd);
	return 1;
}       


/* Unload MMU Hack kernel module after closing all memory devices*/
int mmuunhack(void)
{
	int ret;
	printf("Removing NK's kernel module for Squidge MMU Hack... "); fflush(stdout);
	ret = system("/sbin/rmmod mmuhack");
	printf("done (%i)\n", ret);

	return ret;
}
