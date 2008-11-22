#include <termio.h>
#include <endian.h>

#include <string.h>
#include <pmon.h>
#include <file.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include "ext2fs.h"
#include <diskfs.h>

#include <signal.h>
#include <setjmp.h>
#include <ctype.h>

#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif

#ifdef DEBUG
#define DEBUG_IDE
#else
#undef DEBUG_IDE
#endif

extern int devio_open(int, const char *, int, int);
extern int devio_close(int);
extern int devio_read(int, void *, size_t);
extern int devio_write(int, const void *, size_t);
extern off_t devio_lseek(int, off_t, int);
extern off_t lseek(int fd, off_t offset, int whence);
extern int read (int fd, void *buf, size_t n);


static int ext2_read_file(int, void *, size_t, size_t, struct ext2_inode *);
static int ReadFromIndexBlock(int, __u32, __u32, __u8 **, size_t *, size_t *,
			      __u32 *);
static inline ext2_dirent *ext2_next_entry(ext2_dirent *);
static int ext2_entrycmp(char *, void *, int);
static int ext2_get_inode(int, unsigned long, struct ext2_inode *);
static int ext2_load_linux(int, const unsigned char *);
static int ext2_load_file_content(int, struct ext2_inode *, unsigned char *);
static int read_super_block(int);
static unsigned int find_inode(int fd, unsigned int parent_inode, const char* pathname, ext2_dirent* dirent);
static unsigned char* get_ext2_dirent_from_inode(int fd, unsigned int inode, size_t* size);
int ext2_open(int, const char *, int, int);
int ext2_close(int);
int ext2_read(int, void *, size_t);
int ext2_write(int, const void *, size_t);
off_t ext2_lseek(int, off_t, int);

__u32 RAW_BLOCK_SIZE = 1024;
__u32 EXT2_INODE_SIZE= 128;
unsigned long SECTORS_PER_BLOCK = 2;
unsigned long GROUPDESCS_PER_BLOCK = 32;
unsigned long INODES_PER_GROUP = 0;
//__u32 start_sec; //would be overflow when the partition is too big(8G) 
off_t start_sec;
#define START_PARTION start_sec*512
#define RAW_SECTOR_SIZE 512
#define EXT2_GROUP_DESC_SIZE 32
static struct ext2_inode INODE_STRUCT;
static struct ext2_inode *File_inode = &INODE_STRUCT;
static ext2_dirent File_dirent;
static int read_super_block(int fd)
{
	struct ext2_super_block *ext2_sb;
	__u8 *diskbuf;
	DiskFile* dfs;
	
	dfs = (DiskFile *)_file[fd].data;
	start_sec = dfs->part->sec_begin;

	if ((diskbuf = (__u8 *) malloc(16 * RAW_SECTOR_SIZE)) == NULL) {
#ifdef DEBUG
		printf("Can't alloc memory for the super block!\n");
#endif
		return -1;
	}
	devio_lseek(fd, start_sec * RAW_SECTOR_SIZE, 0);
	if ((devio_read(fd, diskbuf, 16 * RAW_SECTOR_SIZE)) !=
	    16 * RAW_SECTOR_SIZE) {
#ifdef DEBUG
		printf("Read the super block error!\n");
#endif
		free(diskbuf);
		return -1;
	}
	ext2_sb = (struct ext2_super_block *)(diskbuf + 1024);

	INODES_PER_GROUP = ext2_sb->s_inodes_per_group;
	EXT2_INODE_SIZE = ext2_sb->s_inode_size;
	RAW_BLOCK_SIZE = BLOCK_1KB << ext2_sb->s_log_block_size;

	SECTORS_PER_BLOCK = RAW_BLOCK_SIZE / RAW_SECTOR_SIZE;
	GROUPDESCS_PER_BLOCK = RAW_BLOCK_SIZE / EXT2_GROUP_DESC_SIZE;

	free(diskbuf);
	return 0;
}

/*
 * p is at least 6 bytes before the end of page
 */
static inline ext2_dirent *ext2_next_entry(ext2_dirent * p)
{
	return (ext2_dirent *) ((char *)p + le16_to_cpu(p->rec_len));
}

/* ext2 entry name is not null terminated,so we could not use strcmp 
 * return 0 if the first 'len' characters of 'entry' match those of 's'
 */
static int ext2_entrycmp(char *s, void *entry, int len)
{
	int i;
	if (strlen(s) != len)
		return -1;
	for (i = 0; i < len; i++)
		if (*(char *)(s + i) != *(char *)((char *)entry + i))
			return -1;
	return 0;
}

/* read inode 'ino' */
static int ext2_get_inode(int fd, unsigned long ino,
			  struct ext2_inode *ext2_raw_inode_ptr)
{
	unsigned long offset, block, block_group, group_desc, desc;

	struct ext2_group_desc *ext2_gd;
	unsigned char *bh;
	off_t temp;
	/* in which block group */
	block_group = (ino - 1) / INODES_PER_GROUP;
	/* in which block */
	group_desc = block_group / GROUPDESCS_PER_BLOCK;
	/* introduction block(?), super block , then group descriptors 
	 * introduction block maybe the same as super block 
	 */
	block = 1024 / RAW_BLOCK_SIZE + 1 + group_desc;
	/* which descriptor,inside the block */
	desc = block_group % GROUPDESCS_PER_BLOCK;
#ifdef DEBUG_IDE
	printf("ext2_get_inode:ino=%d,block group=%d,block=%d,desc=%d\n", ino,
	       block_group, block, desc);
#endif
	bh = (unsigned char *)malloc(RAW_BLOCK_SIZE);
	temp = (off_t) block *RAW_BLOCK_SIZE + start_sec * 512;
	devio_lseek(fd, temp, 0);
	if (RAW_BLOCK_SIZE != devio_read(fd, bh, RAW_BLOCK_SIZE)) {
		free(bh);
		return -1;
	}
	ext2_gd = (struct ext2_group_desc *)(bh + desc * EXT2_GROUP_DESC_SIZE);
#ifdef DEBUG_IDE
	printf("ext2_group_desc -> bg_free_blocks_count=%d\n",
	       ext2_gd->bg_free_blocks_count);
	printf("ext2_group_desc -> bg_free_inodes_count=%d\n",
	       ext2_gd->bg_free_inodes_count);
	printf("ext2_group_desc -> bg_inode_table=%d\n",
	       ext2_gd->bg_inode_table);
	printf("ext2_group_desc -> bg_block_bitmap=%d\n",
	       ext2_gd->bg_block_bitmap);
	printf("ext2_group_desc -> bg_inode_bitmap=%d\n",
	       ext2_gd->bg_inode_bitmap);
#endif
	offset = ((ino - 1) % INODES_PER_GROUP) * EXT2_INODE_SIZE;
	block = ext2_gd->bg_inode_table + (offset / RAW_BLOCK_SIZE);
	offset = offset % RAW_BLOCK_SIZE;
#ifdef DEBUG_IDE
	printf("ext2_get_inode: offset is %d,block is %d\n", offset, block);
#endif

//	bh = (unsigned char *)malloc(RAW_BLOCK_SIZE);
	memset(bh, 0, RAW_BLOCK_SIZE);
	temp = (off_t) block *RAW_BLOCK_SIZE + start_sec * 512;

#ifdef DEBUG_IDE
	printf("the RAW_BLOCK_SIZE is %x\n", RAW_BLOCK_SIZE);
	printf("In ext2fs.c    The seek offset is %llx\n", temp);

#endif
	devio_lseek(fd, temp, 0);
	if (RAW_BLOCK_SIZE != devio_read(fd, bh, RAW_BLOCK_SIZE)) {
		free(bh);
		return -1;
	}

	memcpy(ext2_raw_inode_ptr, bh + offset, sizeof(struct ext2_inode));
//	*ext2_raw_inode_ptr = (struct ext2_inode *)(bh + offset);
#ifdef DEBUG_IDE
	printf("inode->i_block[0]=%d,the inode->i_size=%d \n",
	       ext2_raw_inode_ptr->i_block[0],
	       ext2_raw_inode_ptr->i_size);
#endif
	free(bh);
	return 0;
}

static int ext2_load_file_content(int fd, struct ext2_inode *inode,
				  unsigned char *bh)
{
	return ext2_read_file(fd, bh, inode->i_size, 0, inode);
}

/* load linux kernel from ext2 partition
 * return 0 if success,else -1
 */
static int ext2_load_linux(int fd, const unsigned char *path)
{
	unsigned int inode;
//	ext2_dirent dirent;
	//__u32 len = 0;
	char* p;
	
	if (read_super_block(fd))
		return -1;

	p = path;
	if (path == NULL || *path == '\0')
	{
		p = "/";
	}

	memset(&File_dirent, 0, sizeof(ext2_dirent));
	if ((inode = find_inode(fd, EXT2_ROOT_INO, p, &File_dirent)) == 0)
	{
		return -1;
	}

	if (File_dirent.file_type == EXT2_FT_DIR)
	{
		printf("%s is directory!\n", p);
		return 0;
	}
	
	if (ext2_get_inode(fd, inode, File_inode))
	{
#ifdef DEBUG
		printf("load EXT2_ROOT_INO error");
#endif
		return -1;
	}

	return 0;
}

//load /dev/fs/ext2@wd0/boot/vmlinux
//path here is wd0/boot/vmlinux
int ext2_open(int fd, const char *path, int flags, int mode)
{
	//int i, index;
	//char strbuf[EXT2_NAME_LEN], *str;
	char *str;
	DiskFile* df;

	df = (DiskFile *)_file[fd].data;
	str = path;
	if (*str == '/')
	{
		str++;
	}
	
	devio_open(fd, df->devname, flags, mode);

	if (!(ext2_load_linux(fd, str)))
	{
		return fd;
	}

	return -1;
}
int ext2_close(int fd)
{
	_file[fd].posn = 0;
	return devio_close(fd);
}
static int ReadFromIndexBlock(int fd, __u32 start_block, __u32 end_block,
			      __u8 ** ReadBuf, size_t * size, size_t * position,
			      __u32 * addr_start)
//ReadBuf  :point to the file content
//size:the real number still need to be read
//position:the file point where the read start
//return :0--successful
//        others--error
{
	__u32 remain_count;
	int re;
	off_t remain_size, addr_abosolute_start;
	if (start_block > end_block)
		return 0;
#ifdef DEBUG_IDE
	printf
	    ("I want to read data size :%u, start_block is %d,end_block is %u\n",
	     *size, start_block, end_block);
#endif
//------------------------------------------------------------------------------------  
	//Read the unaligned data within a block. 
	remain_size = RAW_BLOCK_SIZE - *position % RAW_BLOCK_SIZE;
	if (remain_size > *size) {
		remain_size = *size;
		remain_count = 0;
	} else if (remain_size == RAW_BLOCK_SIZE) {
		remain_size = 0;
		remain_count = 0;
	} else
		remain_count = 1;
	addr_start += start_block;
	//start_block starts with 0. 0-11:direct others 0-1023.???????
	addr_abosolute_start =
	    (off_t) * addr_start * RAW_BLOCK_SIZE + *position % RAW_BLOCK_SIZE;
	if (remain_count) {
		devio_lseek(fd, addr_abosolute_start + START_PARTION, 0);
		re = devio_read(fd, (__u8 *) * ReadBuf, remain_size);
		if (re != remain_size) {
			printf("Can't Read Data From the Disk \n");
			return -1;
		}
		start_block += remain_count;
		addr_start += remain_count;
		remain_count = 0;
		*ReadBuf += remain_size;
		*position += remain_size;
		*size -= remain_size;
		addr_abosolute_start =
		    (off_t) * addr_start * RAW_BLOCK_SIZE +
		    *position % RAW_BLOCK_SIZE;
		remain_size = 0;
	}
//------------------------------------------------------------------------------------
//Read the BLOCK aligned data
	while (*size && (remain_size < *size)
	       && (remain_count + start_block <= end_block)) {
		if (*(addr_start + remain_count + 1) ==
		    *(addr_start + remain_count) + 1)
			//continuous block ,so we just remember the blocks number .
		{
			if (remain_size + RAW_BLOCK_SIZE > *size)
				remain_size = *size;
			else {
				remain_count++;
				remain_size += RAW_BLOCK_SIZE;
			}
		} else {
#ifdef DEBUG_IDE
			printf("The remain size is %u ,size is%u\n",
			       remain_size, *size);
			printf("Block begin at %u,end at %u\n", *addr_start,
			       *(addr_start + remain_count));
#endif
			if (!remain_size)
				//if not continuous,we just read one block
			{

				if (*size < RAW_BLOCK_SIZE)
					remain_size = *size;
				else {
					remain_size = RAW_BLOCK_SIZE;
					remain_count = 1;
				}
			}
			devio_lseek(fd, addr_abosolute_start + START_PARTION,
				    0);
			re = devio_read(fd, (__u8 *) * ReadBuf, remain_size);
			if (re != remain_size) {
				printf("We can't read data from disk!\n");
				return -1;
			}
			start_block += remain_count;
			addr_start += remain_count;
			remain_count = 0;
			*ReadBuf += remain_size;
			*position += remain_size;
			*size -= remain_size;
			if ((*position % RAW_BLOCK_SIZE) && *size) {
				printf
				    ("Oh,My God!When I read in the aligned data,I met one unaligned position\n");
				return -1;
			}
			addr_abosolute_start =
			    (off_t) * addr_start * RAW_BLOCK_SIZE;
			remain_size = 0;
		}
	}
	if (!*size)
		return 0;	//No data need to read.Wonderful!
	if (remain_size) {
		if (remain_size > *size)
			remain_size = *size;
		devio_lseek(fd, addr_abosolute_start + START_PARTION, 0);
#ifdef DEBUG_IDE
		printf("The remain size is %u,size is %u\n", remain_size,
		       *size);
		printf("Additional Block begin at %d,end at %d\n", *addr_start,
		       *(addr_start + remain_count));
#endif
		re = devio_read(fd, (__u8 *) * ReadBuf, remain_size);
		*ReadBuf += remain_size;
		if (re != remain_size) {
			printf("We can't read data from disk!\n");
			return -1;
		}
		*position += remain_size;
		*size -= remain_size;
	}
	return 0;
}

int ext2_read(int fd, void *read_start, size_t size)
{
	int real_size;

	memset(read_start, 0, size);
	if ((_file[fd].posn + size) > File_inode->i_size) {
		size = File_inode->i_size - _file[fd].posn;
	}
	real_size =
	    ext2_read_file(fd, read_start, size, _file[fd].posn, File_inode);
	if ((_file[fd].posn + real_size) > File_inode->i_size) {
		real_size = File_inode->i_size - _file[fd].posn;
		_file[fd].posn = File_inode->i_size;
	} else {
		_file[fd].posn += real_size;
	}
	return real_size;
}
static int ext2_read_file(int fd, void *read_start, size_t size, size_t pos,
			  struct ext2_inode *inode)
{
	__u8 *buff, *index_buff, *start = (__u8 *) read_start;
	size_t read_size = size, position = pos;
	__u32 *addr_start, *d_addr_start, start_block;
	int re, i;
	start_block = position / RAW_BLOCK_SIZE;
	addr_start = inode->i_block;
#ifdef DEBUG_IDE
	printf("the pos is %llx,the size is %llx\n", position, read_size);
#endif
	re = ReadFromIndexBlock(fd, start_block, 11, &start, &read_size,
				&position, addr_start);
#ifdef DEBUG_IDE
	printf("The addr_start is %x,RAW_BLOCK_SIZE is %x,start_sec is %x\n",
	       *addr_start, RAW_BLOCK_SIZE, start_sec);
#endif
	if (re) {
		printf("Error in Reading from direct block\n");
		return 0;
	}
	if (!read_size) {
#ifdef DEBUG_IDE
		for (i = 0; i < size; i += RAW_BLOCK_SIZE / 4)
			printf("%4x", (__u8) * ((__u8 *) read_start + i));
		printf("\n");
#endif
		return (int)size;
	}
//////////////////////Read Index block///////////////////////////////////
	start_block = position / RAW_BLOCK_SIZE - 12;
	buff = (__u8 *) malloc(RAW_BLOCK_SIZE);
	if (!buff) {
		printf("Can't alloc memory!\n");
		return 0;
	}
	addr_start = &(inode->i_block[12]);
	devio_lseek(fd, (off_t) * addr_start * RAW_BLOCK_SIZE + START_PARTION,
		    0);
	re = devio_read(fd, buff, RAW_BLOCK_SIZE);
	if (re != RAW_BLOCK_SIZE) {
		printf("Read the iblock[12] error!\n");
		return 0;
	}
	addr_start = (__u32 *) buff;
	re = ReadFromIndexBlock(fd, start_block, RAW_BLOCK_SIZE / 4 - 1, &start,
				&read_size, &position, addr_start);
	if (re) {
		free((char *)buff);	/*spark add */
		return 0;
	}
	if (!read_size) {
#ifdef DEBUG_IDE
		for (i = 0; i < size; i += RAW_BLOCK_SIZE / 4)
			printf("%4x", (__u8) * ((__u8 *) read_start + i));
		printf("\n");
#endif
		free((char *)buff);	/* spark add */
		return (int)size;
	}
/////////////////////////////////////////Read Double index block////////////////
	addr_start = &(inode->i_block[13]);
	devio_lseek(fd, (off_t) * addr_start * RAW_BLOCK_SIZE + START_PARTION,
		    0);
	re = devio_read(fd, buff, RAW_BLOCK_SIZE);
	if (re != RAW_BLOCK_SIZE) {
		printf("Read the iblock[13] error!\n");
		free((char *)buff);	/* spark add */
		return 0;
	}
	d_addr_start = (__u32 *) buff;
	index_buff = (__u8 *) malloc(RAW_BLOCK_SIZE);
	if (!index_buff) {
		printf("Can't alloc memory!\n");
		return 0;
	}
	for (i = 0; i < RAW_BLOCK_SIZE / 4; i++) {
		devio_lseek(fd,
			    (off_t) * (d_addr_start + i) * RAW_BLOCK_SIZE +
			    START_PARTION, 0);
		re = devio_read(fd, index_buff, RAW_BLOCK_SIZE);
		if (re != RAW_BLOCK_SIZE) {
			printf("Can't read index block!\n");
			return 0;
		}
		addr_start = (__u32 *) index_buff;
		start_block =
		    position / RAW_BLOCK_SIZE - 12 - RAW_BLOCK_SIZE / 4 * (i +
									   1);
		re = ReadFromIndexBlock(fd, start_block, RAW_BLOCK_SIZE / 4 - 1,
					&start, &read_size, &position,
					addr_start);
		if (re) {
			printf("Can't read the double index block!\n");
			free((char *)buff);
			free((char *)index_buff);	/* spark add */
			return 0;
		}
		if (!read_size) {
			free((char *)buff);	/* spark add */
			free((char *)index_buff);
			return (int)size;
		}
	}
	printf("I can't read so big files,give me a email!\n");
	return 0;
}
int ext2_write(int fd, const void *start, size_t size)
{
	return 0;
}

off_t ext2_lseek(int fd, off_t offset, int where)
{
	_file[fd].posn = offset;
	return offset;
}

int is_ext2fs(int fd, off_t start)
{
	struct ext2_super_block *ext2_sb;
    __u8 *diskbuf;

    if ((diskbuf = (__u8 *) malloc(16 * RAW_SECTOR_SIZE)) == NULL) {
#ifdef DEBUG
        printf("Can't alloc memory for the super block!\n");
#endif
        return 0;
    }

    lseek(fd, start * RAW_SECTOR_SIZE, 0);
    if ((read(fd, diskbuf, 16 * RAW_SECTOR_SIZE)) !=
        16 * RAW_SECTOR_SIZE) {
#ifdef DEBUG
        printf("Read the super block error!\n");
#endif
        free(diskbuf);
        return 0;
    }
    ext2_sb = (struct ext2_super_block *)(diskbuf + 1024);
	if (ext2_sb->s_magic != 0xef53)
	{
    	free(diskbuf);
		return 0;
	}

    free(diskbuf);
	return 1;
}

static unsigned char* get_ext2_dirent_from_inode(int fd, unsigned int inode, size_t* size)
{
	char *bh = NULL;
	struct ext2_inode ext2_raw_inode;

	if (ext2_get_inode(fd, inode, &ext2_raw_inode))
	{
		printf("get inode error\n");
		return NULL;
	}

	bh = (unsigned char *)malloc(RAW_BLOCK_SIZE +
		ext2_raw_inode.i_size);
	if (bh == NULL)
	{
#ifdef DEBUG
		printf("Error in allocting memory for file content!\n");
#endif
		return NULL;
	}
	
	ext2_load_file_content(fd, &ext2_raw_inode, bh);

	*size = ext2_raw_inode.i_size;
	return bh;
}

static unsigned int get_inode_from_name(int fd, __u32 inode, const char* path, ext2_dirent* dirent)
{
	ext2_dirent *de = NULL;
	unsigned char *bh = NULL;
	unsigned char s[EXT2_NAME_LEN];
	__u32 size = 0;
		
	bh = get_ext2_dirent_from_inode(fd, inode, &size);
	if (bh == NULL)
	{
#ifdef DEBUG
		printf("Error in allocting memory for file content!\n");
#endif
		return 0;
	}
	
	de = (ext2_dirent *) bh;

	for (; ((unsigned char *)de < bh + size) &&
			 (de->rec_len > 0) && (de->name_len > 0);
		 de = ext2_next_entry(de))
	{
		strncpy(s, de->name, de->name_len);
		s[de->name_len] = '\0';	//*(de->name+de->name_len)='\0';

		if (!ext2_entrycmp(path, de->name, de->name_len))
		{
			free(bh);
			memcpy(dirent, de, sizeof(ext2_dirent));
			return de->inode;
		}
	}
	free(bh);
	return 0;
}

static unsigned int find_inode(int fd, unsigned int parent_inode, const char* pathname, ext2_dirent* dirent)
{
	//struct ext2_inode ext2_raw_inode;
	//unsigned int inode, pinode;
	unsigned int inode;

	
	//int find = 0;
	char path[1024];
	char* p = NULL;
	char* p1;

	if (pathname == NULL  || *pathname == '\0')
	{
		return parent_inode;
	}

	strcpy(path, pathname);
	p1 = path;
	inode = parent_inode;
	while (1)
	{
		p = strchr(p1, '/');
		if (p != NULL)
		{
			*p = '\0';
		}

		inode = get_inode_from_name(fd, inode, p1, dirent);
		if (inode == 0)
		{
			return 0;
		}

		if (p == NULL)
		{
			return inode;
		}

		p1 = p + 1;
	}
	return 0;
}

static void list_filename(int fd, unsigned int inode, const ext2_dirent* dirent)
{
	ext2_dirent *de = NULL;
	unsigned char* bh = NULL;
	struct ext2_inode file_info;
	char* s = NULL;
	__u32 len = 0;
	char size[20];
	char* buf = NULL;
	int siz = 0;
	int ln = 0;
	int i, j;

	s = malloc(EXT2_NAME_LEN);
	if (s == NULL)
	{
		return ;
	}
	
	if (inode != EXT2_ROOT_INO)
	{
		if (dirent->file_type != EXT2_FT_DIR)
		{
			strncpy(s, dirent->name, dirent->name_len);
			s[dirent->name_len] = '\0';	//*(de->name+de->name_len)='\0';
			if (ext2_get_inode(fd, inode, &file_info))
			{
				free(s);
				return ;
			}
			sprintf(size, "%d", file_info.i_size);
			printf("%-40s%-15s%s\n", s, "<FILE>", size);
			free(s);
			return ;
		}
	}
	
	buf = malloc(LINESZ + 8);
	if (buf == NULL)
	{
		free(s);
		return ;
	}

	bh = get_ext2_dirent_from_inode(fd, inode, &len);
	if (bh == NULL)
	{
		free(s);
		free(buf);
		return ;
	}

	de = (ext2_dirent *)bh;
	
	siz = moresz;
	ioctl(STDIN, CBREAK, NULL);
	ln = siz;
	i = 0;
	j = 0;
	for (; ((unsigned char *)de < bh + len) &&
			 (de->rec_len > 0) && (de->name_len > 0);
		 de = ext2_next_entry(de))
	{
		strncpy(s, de->name, de->name_len);
		s[de->name_len] = '\0';	//*(de->name+de->name_len)='\0';

		switch (de->file_type)
		{
		case EXT2_FT_DIR:
			strcpy(size, "<DIR>");
			j += sprintf(buf + j, "%-40s%-15s", s, size);
			break;
		case EXT2_FT_UNKNOWN:
			strcpy(size, "<unknown>");
			j += sprintf(buf + j, "%-40s%-15s", s, size);
			break;
		default:
			if (ext2_get_inode(fd, de->inode, &file_info))
			{
				continue;
			}
			sprintf(size, "%d", file_info.i_size);
			j += sprintf(buf + j, "%-40s%-15s%s", s, "<FILE>", size);
			break;
		}

		if ((i % 2) != 0)
		{
			if (more(buf, &ln, siz))
			{
				free(bh);
				free(s);
				free(buf);
				return ;
			}
			j = 0;
		}
		else
		{
			j += sprintf(buf + j, "\n");
		}
		i++;
	}

	if (i % 2 != 0)
	{
		printf("%s\n", buf);
	}
	free(bh);
	free(s);
	free(buf);
}

static int ext2_load_linux_dir(int fd, const char* path)
{
	ext2_dirent dirent;
	//int i;
	unsigned int inode;
	char* p;

	if (read_super_block(fd))
		return -1;

	if (path == NULL || *path == '\0')
	{
		p = "";
	}
	else
	{
		p = path;
	}

	memset(&dirent, 0, sizeof(ext2_dirent));
	inode = find_inode(fd, EXT2_ROOT_INO, p, &dirent);
	if (inode == 0)
	{
		printf("don't find\n");
		return -1;
	}

	list_filename(fd, inode, &dirent);
	
	return 0;
}

static int ext2_open_dir(int fd, const char* path)
{
	char* str;
	
	str = path;
	if (*str == '/')
	{
		str++;
	}
	
	if (!ext2_load_linux_dir(fd, str))
	{
		return fd;
	}
	
	return -1;
}

static DiskFileSystem diskfile = {
	"ext2",
	ext2_open,
	ext2_read,
	ext2_write,
	ext2_lseek,
	is_ext2fs,
	ext2_close,
	NULL,
	ext2_open_dir,
};
static void init_diskfs(void) __attribute__ ((constructor));
static void init_diskfs()
{
	diskfs_init(&diskfile);
}
