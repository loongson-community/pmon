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
/* $Id: diskfs.h,v 1.1.1.1 2006/09/14 01:59:06 root Exp $ */

#ifndef __DISKFS_H__
#define __DISKFS_H__

#include <sys/queue.h>
#define FS_TYPE_EXT2  0x0
#define FS_TYPE_SWAP  0x1
#define FS_TYPE_FAT   0x2
#define FS_TYPE_ISO9660 0x3
#define FS_TYPE_NTFS 0x4
#define FS_TYPE_BSD 0x5
#define FS_TYPE_COMPOUND 0xFE
#define FS_TYPE_UNKNOWN 0xFF 

//#define FS_SHOW_INFO
#ifdef FS_SHOW_INFO
#define fs_info(format, arg...) printf("FS_INFO: " format "\n", ## arg)
#else
#define fs_info(format, arg...) do {} while(0)
#endif

typedef struct DiskFileSystem {
	char *fsname;
	int (*open) __P((int, const char *, int, int));
	int (*read) __P((int , void *, size_t));
	int (*write) __P((int , const void *, size_t));
	off_t (*lseek) __P((int , off_t, int));
	int (*close) __P((int));
	int (*ioctl) __P((int , unsigned long , ...));
	SLIST_ENTRY(DiskFileSystem)	i_next;
} DiskFileSystem;
#if 0
typedef struct DiskFile {
	char *devname;
	DiskFileSystem *dfs;
} DiskFile;
#endif

int diskfs_init(DiskFileSystem *fs);

typedef struct DiskPartitionTable { 
	struct DiskPartitionTable* Next;
	struct DiskPartitionTable* logical;
	unsigned char bootflag;
	unsigned char tag;
	unsigned char id;
	unsigned int sec_begin;
	unsigned int size;
	unsigned int sec_end;
	unsigned int part_fstype;
	DiskFileSystem* fs;
}DiskPartitionTable;


#define MAX_PARTS 8
typedef struct DeviceDisk {
	struct DeviceDisk* Next;
	char device_name[20];
	unsigned int dev_fstype;
//	DiskPartitionTable* part;
	DiskPartitionTable* part[MAX_PARTS];
}DeviceDisk;
typedef struct DiskFile {
	char *devname;
	DiskFileSystem *dfs;
	DiskPartitionTable* part;
	int part_index;
} DiskFile;



#endif /* __DISKFS_H__ */
