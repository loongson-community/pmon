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
/* $Id: diskfs.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

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

static int diskfs_open (int , const char *, int, int);
static int diskfs_close (int);
static int diskfs_read (int, void *, size_t);
static int diskfs_write (int, const void *, size_t);
static off_t diskfs_lseek (int, off_t, int);

int filename_path_transform(char *fname,char *tname)
{
	char buff[20];
	char *p, *p1;
	char dev[20];
	char id;
	DeviceDisk* pdev;
    char filepath[100];
    int part_no;
    DiskPartitionTable *ppart;
    int part_index ;

    fs_info("fname:%s tname:%s\n",fname,tname);

    p = strchr(fname,',');
    id = *(p+1);

    fs_info("id:%c\n",id);

    p = fname;
    if (*p != '(')
    {
        return -1;
    }


    p1 = strchr(fname,')');
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
    strncpy(dev, fname+1, 19);
    p = strchr(dev,',');
    if (p == NULL)
    {
        return -1;
    }
    *p = '\0';
    p +=1;
	while (*p != '\0' && (*p == ' ' || *p == '\t'))
	{
		p++;
	}    
    part_no = id - 0x30;
    
    p = strchr(fname,'/');
    p +=1;

    sprintf(filepath,"%s",p);
    memset(buff, 0, sizeof(buff));    

    
	pdev = FindDevice(dev); 

    if(NULL==pdev){
        printf("Find device failed NULL==pdev.\n");
        return -1;

    }
    //sprintf(buff, "%s%c", dev, 'a' + atoi(id));
    if(FS_TYPE_ISO9660 == pdev->dev_fstype){
        sprintf(buff, "%s", dev);
    }else{
        //sprintf(buff, "%s%c", dev, 'a' + atoi(&id));
        sprintf(buff, "%s%c", dev, 'a' + (id - '0'));
    }

    //part_index = atoi(&id);
    part_index = id - '0';

    fs_info("pdev->dev_fstype:0x%x buff:%s filepath:%s part_index:%d\n",
        pdev->dev_fstype, buff, filepath, part_index);
    
    if (FS_TYPE_ISO9660 == pdev->dev_fstype){
       // "/dev/fs/iso9660@buff/...."
       sprintf(tname,"/dev/fs/iso9660@%s/%s",buff,filepath);
       return 0;    
    }

    if (FS_TYPE_FAT == pdev->dev_fstype){
        sprintf(tname,"/dev/fs/fat@%s/%s",buff,filepath);
        return 0;
    }
    if (FS_TYPE_EXT2 == pdev->dev_fstype){
        sprintf(tname,"/dev/fs/ext2@%s/%s",buff,filepath);
        return 0;
    } 
    
    if (FS_TYPE_COMPOUND == pdev->dev_fstype){
        ppart = pdev->part[part_index];
        if (FS_TYPE_UNKNOWN == ppart->part_fstype){
            printf("filename_path_transfrom unknown fs type for the given partition.\n");
            return -1;
        }
        if (FS_TYPE_FAT == ppart->part_fstype){
            sprintf(tname,"/dev/fs/fat@%s/%s",buff,filepath);
            return 0;
        }
        if (FS_TYPE_EXT2 == ppart->part_fstype){
            sprintf(tname,"/dev/fs/ext2@%s/%s",buff,filepath);
            return 0;
        }           
    }
    fs_info("tname:%s",tname);
    return -1;
}
/*
 * Supported paths:
 *	/dev/fs/msdos@wd0/bsd
 *	/dev/fs/iso9660@cd0/bsd
 */


static int
   diskfs_open (fd, fname, mode, perms)
   int	       fd;
   const char *fname;
   int         mode;
   int	       perms;
{
	DiskFileSystem *p;
	DiskFile *dfp;
	char *dname;
	char *fsname = NULL;
	char *devname = NULL;
	int i;
	
	dname = (char *)fname;

	if (strncmp (dname, "/dev/fs/", 8) == 0) {
		dname += 8;

		/* Support /dev/fs/fat or /dev/fs/ram also */
		if (strncmp (dname, "fat", 3) == 0)
			return (-1);
		else if (strncmp (dname, "ram", 3) == 0)
			return (-1);
	}
	else
		return (-1);

	 devname=dname;

	for (i=0; i < strlen(dname); i++) {
		if (dname[i] == '@') {
			fsname = dname;
			dname[i] = 0;
			devname = &dname[i+1];
			break;
		}
	}
	if (fsname != NULL && fsname[0]) {
		int found = 0;
		SLIST_FOREACH(p, &DiskFileSystems, i_next) {
			if (strcmp (fsname, p->fsname) == 0) {
				found = 1;
				break;
			}
		}
		if (found == 0)
			return (-1);
	dfp = (DiskFile *)malloc(sizeof(DiskFile));
	if (dfp == NULL) {
		fprintf(stderr, "Out of space for allocating a DiskFile");
		return(-1);
	}

	dfp->dfs = p;
	
	_file[fd].posn = 0;
	_file[fd].data = (void *)dfp;
	if(p->open)
		return ((p->open)(fd,devname,mode,perms));	
	return -1;	
	} else {
	char path[100];
	strncpy(path,devname,100);
	dfp = (DiskFile *)malloc(sizeof(DiskFile));
	if (dfp == NULL) {
		fprintf(stderr, "Out of space for allocating a DiskFile");
		return(-1);
	}

		SLIST_FOREACH(p, &DiskFileSystems, i_next) {
	dfp->dfs = p;
	
	_file[fd].posn = 0;
	_file[fd].data = (void *)dfp;
	strncpy(devname,path,100);
	if(p->open && ((p->open)(fd,devname,mode,perms)==fd))
	return fd;
		}
		free(dfp);
		return -1;
	}
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

	if (p->dfs->read)
		return ((p->dfs->read) (fd, buf, n));
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
	diskfs_close,
	NULL
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
