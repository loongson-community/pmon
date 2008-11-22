/*
 * Copyright (c) 2000 Opsycon AB  (www.opsycon.se)
 * Copyright (c) 2002 Patrik Lindergren (www.lindergren.com)
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
 *	This product includes software developed by Opsycon AB.
 *	This product includes software developed by Patrik Lindergren.
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
/* $Id: diskfs.c,v 1.1.1.1 2006/06/29 06:43:25 cpu Exp $ */

#include <stdio.h>
#include <string.h>
#include <termio.h>
#include <fcntl.h>
#include <file.h>
#include <ctype.h>
#include <diskfs.h>
#include <sys/unistd.h>
#include <stdlib.h>
#undef _KERNEL
#include <errno.h>
#include <pmon.h>

SLIST_HEAD(DiskFileSystems,DiskFileSystem)DiskFileSystems=SLIST_HEAD_INITIALIZER(DiskFileSystems);


extern DeviceDisk* FindDevice(const char* device_name);
extern DiskPartitionTable* FindPartitionFromDev(DiskPartitionTable* table, const char* device);
extern DiskPartitionTable* FindPartitionFromID(DiskPartitionTable* table, int index);

static int diskfs_open (int , const char *, int, int);
static int diskfs_close (int);
static int diskfs_read (int, void *, size_t);
static int diskfs_write (int, const void *, size_t);
static off_t diskfs_lseek (int, off_t, int);

/*
 * Supported paths:
 *	/dev/fs/msdos@wd0/bsd
 *	/dev/fs/iso9660@cd0/bsd
 */

static DiskFile* GetDiskFile(const char* fname, char* filename)
{
	DeviceDisk* pdev;
	DiskPartitionTable* part;
	DiskFile *dfp;
	char *dname;
	char *fsname = NULL;
	char *devname = NULL;
	int i;
	char* p;
	
	dname = (char *)fname;

	if (strncmp (dname, "/dev/fs/", 8) == 0)
	{
		dname += 8;
		for (i=0; i < strlen(dname); i++)
		{
			if (dname[i] == '@')
			{
				fsname = dname;
				dname[i] = 0;
				devname = &dname[i+1];
				break;
			}
		}

		if (devname == NULL || *devname == '\0')
		{
			printf("device format error11111111111[%s]\n", devname);
			return NULL;
		}

		p = strchr(devname, '/');
		if (p == NULL)
		{
			strcpy(filename, "");
		}
		else
		{
			strcpy(filename, p + 1);
			*p = '\0';
		}

		pdev = FindDevice(devname);
		if (pdev == NULL)
		{
			printf("Couldn't find device [%s]\n", devname);
			return NULL;
		}

		part = FindPartitionFromDev(pdev->part, devname);
		if (part == NULL || part->fs == NULL)
		{
			printf("Couldn't find partition [%s]\n", devname);
			return NULL;
		}

		if (fsname != NULL && part->fs != NULL)
		{
			if (strcmp(fsname, part->fs->fsname) != 0)
			{
				printf("file system isn't same[%s][%s]\n", fsname, part->fs->fsname);
				return NULL;
			}
		}
	}
	else if (*dname == '(')
	{
		//char c;
		
		devname = dname + 1;
		p = strchr(devname, '/');
		if (p == NULL)
		{
			strcpy(filename, "");
		}
		else
		{
			strcpy(filename, p + 1);
		}

		p = strchr(dname, ',');
		if (p == NULL)
		{
			printf("device format is error[%s]\n", dname);
			return NULL;
		}
		*p = '\0';
		pdev = FindDevice(devname);
		if (pdev == NULL)
		{
			printf("Couldn't find device [%s]\n", devname);			
			return NULL;
		}

		fsname = p + 1;
		p = strchr(fsname, ')');
		if (p == NULL)
		{
			return NULL;
		}
		*p = '\0';
		if (isspace(*fsname))
		{
			return NULL;
		}

		part = FindPartitionFromID(pdev->part, atoi(fsname) + 1);
		if (part == NULL || part->fs == NULL)
		{
			printf("Don't find partition [%s]\n", devname);
			return NULL;
		}
	}
	else
	{
		printf("device format is error[%s]\n", dname);
		return NULL;
	}

	dfp = (DiskFile *)malloc(sizeof(DiskFile));
	if (dfp == NULL) {
		fprintf(stderr, "Out of space for allocating a DiskFile");
		return NULL;
	}

	dfp->devname = pdev->device_name;
	dfp->dfs = part->fs;
	dfp->part = part;

	return dfp;
}

static int
   diskfs_open (fd, fname, mode, perms)
   int	       fd;
   const char *fname;
   int         mode;
   int	       perms;
{
	DiskFile *dfp;
	char filename[256];

	strcpy(filename, fname);
#if 0
	dname = (char *)fname;

	if (strncmp (dname, "/dev/fs/", 8) == 0)
		dname += 8;
	else
		return (-1);

	for (i=0; i < strlen(dname); i++) {
		if (dname[i] == '@') {
			fsname = dname;
			dname[i] = 0;
			devname = &dname[i+1];
			break;
		}
	}
	if (fsname != NULL) {
		int found = 0;
		SLIST_FOREACH(p, &DiskFileSystems, i_next) {
			if (strcmp (fsname, p->fsname) == 0) {
				found = 1;
				break;
			}
		}
		if (found == 0)
			return (-1);
	} else {
		/*
		 * Boot block booting
		 */
	}
	dfp = (DiskFile *)malloc(sizeof(DiskFile));
	if (dfp == NULL) {
		fprintf(stderr, "Out of space for allocating a DiskFile");
		return(-1);
	}

	dfp->dfs = p;
#else

	dfp = GetDiskFile(fname, filename);
	if (dfp == NULL)
	{
		return -1;
	}
#endif
	_file[fd].posn = 0;
	_file[fd].data = (void *)dfp;
	if(dfp->dfs->open)
		return ((dfp->dfs->open)(fd, filename, mode, perms));	
	return -1;	
}

/** close(fd) close fd */
static int
   diskfs_close (fd)
   int             fd;
{
	DiskFile *p;

	p = (DiskFile *)_file[fd].data;

	if (p->dfs->close)
		return ((p->dfs->close) (fd));
	else
		return (-1);
}

/** read(fd,buf,n) read n bytes into buf from fd */
static int
   diskfs_read (fd, buf, n)
   int fd;
   void *buf;
   size_t n;
{
	DiskFile        *p;

	p = (DiskFile *)_file[fd].data;

	if (p->dfs->read){
		return ((p->dfs->read) (fd, buf, n));
	}
	else {
		return (-1);
	}
}

static int
   diskfs_write (fd, buf, n)
   int fd;
   const void *buf;
   size_t n;
{
	DiskFile        *p;

	p = (DiskFile *)_file[fd].data;

	if (p->dfs->write)
		return ((p->dfs->write) (fd, buf, n));
	else
		return (-1);
}

static off_t
diskfs_lseek (fd, offset, whence)
	int             fd, whence;
	off_t            offset;
{
	DiskFile        *p;

	p = (DiskFile *)_file[fd].data;

	if (p->dfs->lseek)
		return ((p->dfs->lseek) (fd, offset, whence));
	else
		return (-1);
}

static FileSystem diskfs =
{
	"fs",FS_FILE,
	diskfs_open,
	diskfs_read,
	diskfs_write,
	diskfs_lseek,
	//NULL,
	diskfs_close,
	NULL,
	{NULL}
};

static void init_fs __P((void)) __attribute__ ((constructor));

static void
   init_fs()
{
	/*
	 * Install diskfs based file system.
	 */
	filefs_init(&diskfs);
}
int diskfs_init(DiskFileSystem *dfs)
{
	SLIST_INSERT_HEAD(&DiskFileSystems,dfs,i_next);
	return (0);
}
