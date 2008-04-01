#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/endian.h>

#include <pmon.h>
#include <exec.h>
#include <pmon/loaders/loadfn.h>

#ifdef __mips__
#include <machine/cpu.h>
#endif

static unsigned int rd_start;
static unsigned int rd_size;
static int execed;
//#define INITRD_DEBUG

int cmd_initrd (int ac, char *av[])
{
	char path[256] = {0};
	char buf[DLREC+1] = {0};
	int flags = 0;
	int bootfd;
	int n = 0;
	ExecId id;

	if (ac != 2 && ac!=3)
		return -1;

	if(ac==3)rd_start=strtoul(av[2],0,0);
	else rd_start = 0x84000000;
	rd_size = 0;
	
	flags |= RFLAG;
	strcpy(path, av[1]);
	printf("Loading initrd image %s", path);
	
	if ((bootfd = open (path, O_RDONLY | O_NONBLOCK)) < 0) {
		perror (path);
		return EXIT_FAILURE;
	}

	dl_initialise (rd_start, flags);
	
	id = getExec("bin");
	if (id != NULL) {
		exec (id, bootfd, buf, &n, flags);
		rd_size = (dl_maxaddr - dl_minaddr) ;
		execed = 1;
	}else{
		printf("[error] this pmon can't load bin file!");
		return -2;
	}
	close(bootfd);
#ifdef INITRD_DEBUG
	printf("rd_start %x, rd_size %x\n", rd_start, rd_size);
#endif
	return 0;
}

int initrd_execed(void)
{
	return execed;
}

unsigned int get_initrd_start(void)
{
	return rd_start;
}

unsigned int get_initrd_size(void)
{
	return rd_size;
}

/*
 *  Command table registration
 *  ==========================
 */

static const Cmd GrubCmd[] =
{
	{"grub like command"},
	{"initrd",	"initrd/initramfs path",
			0,
			"load initrd/initramfs image",
			cmd_initrd, 2, 3, CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(GrubCmd, 1);
}

