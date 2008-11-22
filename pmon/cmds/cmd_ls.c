/*
 * Copyright (c) 2007 SUNWAH HI-TECH  (www.sw-linux.com.cn)
 * Wing Sun	<chunyang.sun@sw-linux.com>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Opsycon AB, Sweden.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

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
#include <diskfs.h>
#include <file.h>
#include <pmon/loaders/loadfn.h>

#ifdef __mips__
#include <machine/cpu.h>
#endif

#define MAX_LINES 10
extern DeviceDisk* FindDevice(const char* device_name);
extern DiskPartitionTable* FindPartitionFromDev(DiskPartitionTable* table, const char* device);

int cmd_list (int ac, char *av[])
{
	DiskPartitionTable* pPart;
	int fd;
	char buff[1025];
	char *p, *p1;
	char dev[50];
	char id[50];
	//int part_id;
	DeviceDisk* pdev;
	DiskFile* dfs;
	
	p = av[1];
	if (*p != '(')
	{
		return -1;
	}
	
	p1 = strchr(av[1], ')');
	if (p1 == NULL)
	{
		return -1;
	}

	p1 += 1;
	if (*p1 != '\0' && *p1 != '/')
	{
		return -1;
	}
		
	memset(dev, 0, sizeof(dev));
	strncpy(dev, av[1] + 1, 49);
	p = strchr(dev, ',');
	if (p == NULL)
	{
		return -1;
	}
	*p = '\0';
	p += 1;
	while (*p != '\0' && (*p == ' ' || *p == '\t'))
	{
		p++;
	}
	
	memset(id, 0, sizeof(id));
	strncpy(id, p, 49);
	p = strchr(id, ')');
	*p = '\0';
	p += 1;
	
	while (*p != '\0' && (*p == ' ' || *p == '\t'))
	{
		p++;
	}

	if (p[strlen(p) - 1] == '/')
	{
		p[strlen(p) - 1] = '\0';
	}
	
	if (*p == '\0')
	{
		p = "";
	}

	pdev = FindDevice(dev);
	if (pdev == NULL)
	{
		printf("%s don't find\n", dev);
		return -1;
	}

	sprintf(buff, "%s%c", dev, 'a' + atoi(id));
    pPart = FindPartitionFromDev(pdev->part, buff);
    if (pPart == NULL)
    {
        printf("find partiton [%s] error\n", buff);
        return -1;
    }

    if (pPart->fs == NULL || pPart->fs->open_dir == NULL)
    {
        return 0;
    }

	dfs = (DiskFile *)malloc(sizeof(DiskFile));
	if (dfs == NULL)
	{
		return -1;
	}

	dfs->dfs = pPart->fs;
	dfs->part = pPart;
	dfs->devname = pdev->device_name;
	
	sprintf(buff, "/dev/disk/%s", dev);
    fd = open(buff, O_RDONLY);
    if (fd == -1)
    {
        printf("open error\n");
        return -1;
    }

	if (*p == '/')
	{
		p++;
	}
	_file[fd].data = (void *)dfs;
	sprintf(buff, "%s%c%s", dev, 'a' + atoi(id), p);
	if (pPart->fs->open_dir != NULL && (pPart->fs->open_dir)(fd, p) != fd)
	{
		printf("open error1\n");
		close(fd);
		return -1;
	}

	free(_file[fd].data);
	_file[fd].data = NULL;
	close(fd);
	return 0;
}

/*
 *  Command table registration
 *  ==========================
 */

static const Cmd ListCmd[] =
{
	{"RAYS Commands for PMON 2000"},
	{"dir",	"(device,part id)/path ",
			0,
			"Display file content.",
			cmd_list, 2, 2, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void
init_cmd()
{
	cmdlist_expand(ListCmd, 1);
}

