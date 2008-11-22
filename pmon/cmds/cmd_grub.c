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

//static unsigned int rd_start;
//static unsigned int rd_size;
//static int execed;
//#define INITRD_DEBUG
extern int boot_initrd(const char* path, int flags);

int cmd_initrd (int ac, char *av[])
{
	int flags = 0;
	int ret;
	
	if (ac != 2)
		return -1;

	flags |= RFLAG;

	//ret = boot_initrd(av[1], flags, NULL, NULL);
	ret = boot_initrd(av[1], flags);
	if (ret == 0)
	{
		return EXIT_SUCCESS;
	}
	
	return EXIT_FAILURE;
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
			cmd_initrd, 2, 2, CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(GrubCmd, 1);
}

