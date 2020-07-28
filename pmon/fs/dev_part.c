/* $Id: devfs.c,v 1.1.1.1 2006/03/10 03:27:14 cpu Exp $ */

/*
 * Copyright (c) 1998-2003 Opsycon AB (www.opsycon.se)
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

/*
 *	_Very_ simplified support functions to i/o subsystem.
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/device.h>
#include <sys/queue.h>

#include <pmon.h>
#include <file.h>
#include <diskfs.h>

#include "mod_usb_storage.h"

#define SECTOR_SIZE 0x200


/* include/asm-i386/types.h */
typedef __signed__ char __s8;
typedef unsigned char __u8;
typedef __signed__ short __s16;
typedef unsigned short __u16;
typedef __signed__ int __s32;
typedef unsigned int __u32;
typedef unsigned long long __u64;

#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif

extern int errno;
static DeviceDisk* gDevice = NULL;

void DevicesInit(void );
void DeviceRelease(void);


DeviceDisk* FindDevice(const char* device_name);
extern SLIST_HEAD(DiskFileSystems,DiskFileSystem) DiskFileSystems;
												  
												  
inline static __u32 get_part_size(__u8* rb_entry)
{
	__u32 size;
	size = *(unsigned short *)(rb_entry + 12 + 2);
	size <<= 16;
	size += *(unsigned short *)(rb_entry + 12);

	return size;
}

inline static __u32 get_logical_part_offset(__u8 * rb_entry)
{
	__u32 offset;
	offset = *(unsigned short *)(rb_entry + 8 + 2);
	offset <<= 16;
	offset += *(unsigned short *)(rb_entry + 8);
	return offset;
}

static void get_filesystem(int fd, DiskPartitionTable* part)
{

	DiskFileSystem *p;
	const static char* name;
    
    if(NULL == part)
        return;  
    switch (part->part_fstype)
	        {
	            case FS_TYPE_EXT2:
                    name = "ext2";
		            break;
	            case FS_TYPE_FAT:
                    name = "fat";
		            break;
	            case FS_TYPE_NTFS:
		            name = "ntfs";
		            break;
	            case FS_TYPE_BSD:
		            name = "bsd";
		            break;
	            default:
		            name = "unknown";
		            break;
	        } 

			SLIST_FOREACH(p, &DiskFileSystems, i_next) {

			if (strcmp (name, p->fsname) == 0) {
				part->fs = p;
				return;
			}
		}
}

static int read_primary_part_table(int fd, __u32 mbr_sec_off, DiskPartitionTable **table)
{
	__u8 *leadbuf;
    __u8 *partbuf;
	int i;
	__u8 tag;
	DiskPartitionTable* part;
	int cnt = 0;
	int id = 1;
	__u32 size;
	__u32 sec_off;
    int part_index = 0;

	if (table == NULL)
	{
		return 0;
	}
	if ((leadbuf = (__u8 *) malloc(SECTOR_SIZE)) == NULL)
	{
		printf("Can't alloc memory for the super block!\n");
		return -1;
	}
	if ((partbuf = (__u8 *) malloc(SECTOR_SIZE)) == NULL)
	{
		printf("Can't alloc memory for the partition block!\n");
		return -1;
	}

	lseek(fd, (off_t)mbr_sec_off * SECTOR_SIZE, 0);
	if ((read(fd, leadbuf, SECTOR_SIZE)) != SECTOR_SIZE)
	{
		free(leadbuf);
		printf("Can't read the leading block from disk!\n");
		return -1;
	}
	if (leadbuf[510] != 0x55 || leadbuf[511] != 0xaa)
    {
		D(printf("Not a MBR paetation table\n"));
        return -1; /*check mbr magic failed */
    }

	//search the partion table to find the partition with id=0x83 and 0x05
	for (cnt = 0, i = 446; i < 510; i += 0x10)
	{
		tag = leadbuf[i + 4];
		sec_off = get_logical_part_offset(leadbuf + i);
		size = get_part_size(leadbuf + i);
		if (tag == 0 && sec_off == 0 && size == 0)
		{
			id++;
			continue;
		}
		
		part = (DiskPartitionTable *)malloc(sizeof(DiskPartitionTable));
		if (part == NULL)
		{
		    printf("Can't alloc memory for the part pointer!\n");
            continue;
		}
		memset(part, 0, sizeof(DiskPartitionTable));
		part->tag = tag;
		part->id = id;
		part->bootflag = 0;
		part->sec_begin = sec_off;
		part->size = size;
		part->sec_end = part->size + part->sec_begin;
		part->fs = NULL;
        //do the fix ,because Partition table do not tell
        //the file system type of partition so correctly.
    	lseek(fd, (off_t)sec_off * SECTOR_SIZE, 0);
	    if ((read(fd, partbuf, SECTOR_SIZE)) != SECTOR_SIZE)
	    {
		    free(partbuf);
		    printf("Can't read the leading block from disk!\n");
		    return 0;
	    }
        if ((0xEB==partbuf[0]&&0x90==partbuf[2])||(0xE9==partbuf[0])){
            part->tag = 0xB;//FAT32
        }else{
            lseek(fd, (off_t)sec_off * SECTOR_SIZE + 0x400, 0);
	        if ((read(fd, partbuf, SECTOR_SIZE)) != SECTOR_SIZE)
	        {
		        free(partbuf);
		        printf("Can't read the leading block from disk!\n");
		        return 0;
	        }
            if((0x53==partbuf[0x38])&&(0xEF==partbuf[0x39])){
                part->tag = 0x83;//EXT2
            }
        } 

        fs_info("part->tag:0x%x\n",part->tag);
        
	    switch (part->tag)
	    {
	    case 0x83:
		    //name = "linux";
		    part->part_fstype = FS_TYPE_EXT2;
		    break;
	    case 0x82:
		    //name = "swap";
		    part->part_fstype = FS_TYPE_SWAP;
		    break;
	    case 0x04:
	    case 0x06:
	    case 0x0E:
	    case 0x1E:
	    case 0x14:
	    case 0x16:
		    //name = "fat16";
		    //break;
	    case 0x0B:
	    case 0x0C:
	    case 0x1B:
	    case 0x1C:
		    //name = "fat32";
		    part->part_fstype = FS_TYPE_FAT;
		    break;
	    case 0x07:
	    case 0x17:
	    case 0x86:
	    case 0x87:
		    //name = "ntfs";
		    part->part_fstype = FS_TYPE_NTFS;
		    break;
	    case 0xA5:
		    //name = "bsd";
		    part->part_fstype = FS_TYPE_BSD;
		    break;
	    default:
		    //name = "unknown";
		    part->part_fstype = FS_TYPE_UNKNOWN;
		    break;
	    }   
		
		/* file system */
		get_filesystem(fd, part);
		table[part_index] = part;
        part_index++;
		cnt++;
		id++;

		if (part_index >= MAX_PARTS)
			break;
	}

    // Second chance flygoat: Why???
    if (table[0] == NULL){
        if ((leadbuf[0]==0xEB)&&  (leadbuf[2]==0x90))
        {
		    part = (DiskPartitionTable *)malloc(sizeof(DiskPartitionTable));
		    memset(part, 0, sizeof(DiskPartitionTable));
            part->tag = 0xB;//FAT32
            part->part_fstype = FS_TYPE_FAT;
		    part->fs = NULL;
		    part->bootflag = 0;
		    part->sec_begin = 0;
            part->id = 1;;
		    cnt++;
            table[0] = part;
		    get_filesystem(fd, part);
        }    
    }
	free(leadbuf);
    free(partbuf);
	return cnt;
}

static int dev_part_read(int fd, DiskPartitionTable** ppTable)
{
	int cnt;
	DiskPartitionTable* table = NULL;
	DiskPartitionTable* p;
	int number = 0;


	cnt = read_primary_part_table(fd, 0, ppTable);
	if (cnt <= 0)
	{
		return 0;
	}
	return cnt;
}

static const char* get_part_type_name(int tag)
{
	const static char* name;
	
	switch (tag) {
	case 0x83:
		name = "linux";
		break;
	case 0x82:
		name = "swap";
		break;
	case 0x04:
	case 0x06:
	case 0x0E:
	case 0x1E:
	case 0x14:
	case 0x16:
		name = "fat16";
		break;
	case 0x0B:
	case 0x0C:
	case 0x1B:
	case 0x1C:
		name = "fat32";
		break;
	case 0x07:
	case 0x17:
	case 0x86:
	case 0x87:
		name = "ntfs";
		break;
	case 0xA5:
		name = "bsd";
		break;
	default:
		name = "unknown";
		break;
	}

	return name;
}

static void zhuan(__u32 block, char* str, int danwei)
{
	switch(danwei)
	{
	case 1:
		sprintf(str, "%d", block / 2);
		break;
	case 2:
		sprintf(str, "%d", block / 2048);
		break;
	case 3:
		sprintf(str, "%d", block / (2048 * 1024));
		break;
	default:
		sprintf(str, "%d", block);
		break;
	}
}

static void fs_type_string(DiskPartitionTable* part, char* fs_name)
{
	if (part == NULL)
	{
		strcpy(fs_name, "unknown");
	}
	else
	{
		if (part->fs == NULL)
			strcpy(fs_name, "unknown");
		else
			strcpy(fs_name, part->fs->fsname);
	}
}

/* danwei 0-512B, 1-1K, 2-1M, 3-1G */
void PrintPartitionTable(DeviceDisk* dev, int danwei)
{
	DiskPartitionTable *p;
	char system[20];
	char sec_begin[20];
	char size[20];
	char fssystem[25];
	char sec_end[20];
	char label[20];
	int i;
	
	const char* fmt = "%-12s%-12s%-12s%-12s%-12s%-12s\n";

	printf("block size: %s\n", danwei == 0 ? "512B" : danwei == 1 ? "1K" : danwei == 2 ? "1M" : "1G");
	printf(fmt, "Name", "Start", "Size", "End", "File Sytem", "System");
	printf("------------------------------------------------------------------------\n");
	for (i = 0; i < MAX_PARTS; i++)
	{
		p = dev->part[i];
		if (!p)
			break;

		sprintf(label, "(%s,%d)", dev->device_name, p->id - 1);
		sprintf(system, "%s", get_part_type_name(p->tag));
		zhuan(p->sec_begin, sec_begin, danwei);
		zhuan(p->size, size, danwei);
		zhuan(p->sec_end, sec_end, danwei);

		fs_type_string(p, fssystem);

		printf(fmt, label, sec_begin, size, sec_end, fssystem, system);
	}
}

int dev_part_detect(DeviceDisk* dev, const char* dev_name,int fd, int usb_cd_index)
{
 	__u8 *leadbuf;

	if ((leadbuf = (__u8 *) malloc(SECTOR_SIZE*4)) == NULL)
	{
		printf("Can't alloc memory for the super block!\n");
		return -1;
	}

	lseek(fd, 16*SECTOR_SIZE*4, 0);
	if ((read(fd, leadbuf, SECTOR_SIZE*4)) != (SECTOR_SIZE*4))
	{
        lseek(fd,0,0);
        free(leadbuf);
		printf("Reading block failed!\n");
		return 0;
	}
    lseek(fd,0,0);
	if (leadbuf[1] == 'C' && leadbuf[2] == 'D' && leadbuf[3] == '0' && leadbuf[4] == '0' && leadbuf[5] == '1')
    {   
        printf("find iso9660 file system on %s\n ",dev_name);
        dev->dev_fstype = FS_TYPE_ISO9660;
        free(leadbuf);
        return 1; 
    }
    free(leadbuf);
    return 0;    
}
static int _DevPartOpen(DeviceDisk* dev, const char* dev_name)
{
	int fd;
	int cnt = 0;
	char path[256];
    int part_det_ret;	
    int ret;

	strcpy(path, dev_name);
	if (strncmp(dev_name, "/dev/", 5) != 0)
	{
		sprintf(path, "/dev/disk/%s", dev_name);
	}

	fd = open(path, O_RDONLY | O_NONBLOCK, 0);
	if (fd < 0)
	{
		printf("open %s", path);
		perror(" ");
		return -1;
	}
    //deal with no part case,such as iso9660 file system on usb0,
    //usb0 is cd disk 
    if(strncmp(dev_name, "cd", 2) == 0){
        //dev->part = NULL;
        char *p_name = dev_name+2;
        int ide_cd_index = *p_name - '0';
#if defined(LOONGSON_3A2H) || defined(LS7A) || defined(LOONGSON_2K)
        if(1){
#else
        if(is_ide_cd_ready(ide_cd_index)){
#endif
            dev->dev_fstype = FS_TYPE_ISO9660;
        }
        close(fd);
        return -2;
    }
    #if NMOD_USB_STORAGE
    if (strncmp(dev_name,"usb",3) == 0){
        char *p_name = dev_name+3;
        int usb_cd_index = *p_name - '0';

        ret = dev_part_detect(dev, dev_name, fd,usb_cd_index);
        switch(ret){
            case -1:
                return -1;
            case 1:
                return -2;
            default:
                break;
        }

	    cnt = dev_part_read(fd, dev->part);
	    if (cnt <= 0){
		    printf("no partitions\n");
	        dev->dev_fstype = FS_TYPE_UNKNOWN;
	    } else {
            if(dev->part[0]) {
                dev->dev_fstype = dev->part[0]->part_fstype;
            }
            if (dev->part[1]) {
                dev->dev_fstype = FS_TYPE_COMPOUND; /* flygoat: Why??? */
            }
        }
		close(fd);
		PrintPartitionTable(dev, 2);
        return cnt;
    }       
    #endif

    cnt = dev_part_read(fd, dev->part);
    if (cnt <= 0) {
        printf("no partitions\n");
	    dev->dev_fstype = FS_TYPE_UNKNOWN;
    } else {
        if(dev->part[0])
            dev->dev_fstype = dev->part[0]->part_fstype;

        if (dev->part[1])
            dev->dev_fstype = FS_TYPE_COMPOUND; /* flygoat: Why?? */
    }
    close(fd);
	PrintPartitionTable(dev, 2);

    return cnt;
}

void DevicesInit(void)
{
	DeviceDisk *dev_disk, *p = NULL;
	struct device *dev, *next_dev;
    int part_open_ret;

	if (gDevice != NULL)
	{
		return ;
	}
	
	for (dev  = TAILQ_FIRST(&alldevs); dev != NULL; dev = next_dev)
	{
		next_dev = TAILQ_NEXT(dev, dv_list);
		if(dev->dv_class != DV_DISK)
		{
			continue;
		}
		dev_disk = (DeviceDisk *)malloc(sizeof(DeviceDisk));
		if (dev_disk == NULL)
		{
            printf("DeviceInit malloc DeviceDisk failed.\n");
            continue;
		}
		memset(dev_disk, 0, sizeof(DeviceDisk));
		//strcpy(dev_disk->device_name, &dev->dv_xname);
		strcpy(dev_disk->device_name, dev->dv_xname);
        dev_disk->dev_fstype = FS_TYPE_UNKNOWN; 

		if (strncmp(dev_disk->device_name, "loopdev", 7) != 0)
			part_open_ret = _DevPartOpen(dev_disk, dev_disk->device_name);
       /* 
        if(-1 == part_open_ret){
			//free(dev_disk);
			continue;
		}
		*/
		if (gDevice == NULL)
		{
			gDevice = dev_disk;
			p = gDevice;
		}
		else
		{
			p->Next = dev_disk;
			p = dev_disk;
		}
	}
}

static int _DevPartClose(DiskPartitionTable* table[])
{
	DiskPartitionTable *p;
	int i;

	for (i = 0; i < MAX_PARTS; i++) {
		p = table[i];
		if (!p)
			break;

		free(p);
	}

}

void DeviceRelease()
{
	DeviceDisk *p, *p1;
	
	if (gDevice == NULL)
	{
		return ;
	}

	p = gDevice;
	while (p != NULL)
	{
		p1 = p->Next;
		_DevPartClose(&p->part);
		free(p);
		p = p1;
	}

	if (gDevice != NULL) {
		gDevice = NULL;
	}
}

DeviceDisk* FindDevice(const char* device_name)
{
	DeviceDisk* pdev;

	for (pdev = gDevice; pdev != NULL; pdev = pdev->Next)
	{
		if (strncmp(pdev->device_name, device_name, strlen(pdev->device_name)) == 0)
		{
			return pdev;
		}	
	}

	return NULL;
}

DiskPartitionTable* FindPartitionFromID(DiskPartitionTable* table, int index)
{
	DiskPartitionTable *p, *p1;
	int i;

	/* W.I.P. */

	return NULL;
}

/* notes: device is wd0a wd0b ... */
DiskPartitionTable* FindPartitionFromDev(DiskPartitionTable* table, const char* device)
{

	/* W.I.P. */

	return FindPartitionFromID(table, 0);
}

int is_usb_cd_iso9660_fs(const char* device_name)
{
	DeviceDisk* pdev;

	pdev = FindDevice(device_name);
    if (pdev && (pdev->dev_fstype == FS_TYPE_ISO9660))
        return 1;
    else
        return 0;
}

int is_fs_unknow(const char* device_name)
{
	DeviceDisk* pdev;

	pdev = FindDevice(device_name);
    if (pdev && (pdev->dev_fstype == FS_TYPE_UNKNOWN))
        return 1;
    else
        return 0;
}

