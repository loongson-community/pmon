#include <string.h>
#include <pmon.h>
#include <file.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include "ext2fs.h"
#include <diskfs.h>
/*#define DEBUG_IDE*/

#define EXT3_SUPER_MAGIC    0xEF53
#define SECTOR_SIZE	512
#define START_PARTION (start_sec * SECTOR_SIZE)


#ifdef DEBUG_IDE
static void dump_feature(struct ext2_super_block *);
static void dump_feature(struct ext2_super_block *ext2_sb)
{
	if(ext2_sb->s_feature_incompat & EXT2_FEAT_INCOMP_EXTENTS)
		printf("feature_incomp_extents\n");
	if(ext2_sb->s_feature_incompat & EXT2_FEAT_INCOMP_64BIT)
		printf("feature_incomp_64bit\n");
	if(ext2_sb->s_feature_incompat & EXT2_FEAT_INCOMP_META_BG)
		printf("feature_incomp_meta_bg\n");
	if(ext2_sb->s_feature_incompat & EXT2_FEAT_INCOMP_FLEX_BG)
		printf("feature_incomp_flex_bg\n");
	if(ext2_sb->s_feature_incompat & EXT2_FEAT_INCOMP_EA_INODE)
		printf("feature_incomp_ea_inode\n");
}
#endif
extern int devio_open(int , const char * , int , int);
extern int devio_close(int);
extern int devio_read(int , void * , size_t);
extern int devio_write(int , const void * , size_t);
extern off_t devio_lseek(int , off_t , int);


static int ext2_read_file(int , void *, size_t , size_t , struct ext2_inode *);
static int ReadFromIndexBlock(int , __u32 , __u32 , __u8 ** , size_t *, size_t * , __u32 *);
static int ReadFromIndexBlock2(int fd,__u32 start_index,__u32 end_index,__u8 **start,	size_t *size,size_t *position,__u32 *d_addr_start);
static inline ext2_dirent *ext2_next_entry(ext2_dirent *);
static int ext2_entrycmp(char *, void *, int );
static int ext2_get_inode(int , unsigned long , struct ext2_inode ** );
static int   ext2_load_linux(int , int , const unsigned char *);
static int   read_super_block(int , int);
int ext2_open(int , const char * , int , int);
int ext2_close(int);
int ext2_read(int , void * , size_t);
int ext2_write(int , const void * , size_t);
off_t ext2_lseek(int , off_t , int);

__u32 sb_block_size = 1024;
unsigned long sb_grpdecs_per_block = 32;
unsigned long sb_inodes_per_grp = 0;
unsigned long sb_ext2_inode_size = 128;
unsigned long sb_grpdesc_size = 32;
off_t start_sec;
static struct ext2_inode file_inode;
static struct ext2_inode *the_inode = &file_inode;
static int read_super_block(int fd, int index)
{
	__u8 *diskbuf, *leadbuf;
	struct ext2_super_block *ext2_sb;
	int i, find_linux_partion;
	int err;

	find_linux_partion = 0;
	diskbuf = 0;
	leadbuf = 0;
	err = -1;
	start_sec = 0;

	if(!(leadbuf = (__u8 *)malloc(SECTOR_SIZE))) {

		printf("Can't alloc memory for the super block!\n");
		goto out;
	}
	if(!(diskbuf = (__u8 *)malloc(16 * SECTOR_SIZE))) {

		printf("Can't alloc memory for the super block!\n");
		goto out;
	}
	if(index) {
		devio_lseek(fd, 0, 0);
		if(devio_read(fd, leadbuf, SECTOR_SIZE) != SECTOR_SIZE) {
			printf("Can't read the leading block from disk!\n");
			goto out;
		}
		/*
		 * search the partion table to find the linux partion
		 * with id = 83
		 * */
		for(i = 446; i < 512; i += 0x10) {
			if(leadbuf[i + 4] == 0x83) {
				start_sec = *(unsigned short *)(leadbuf + i + 8 + 2);
				start_sec <<= 16;
				start_sec += *(unsigned short *)(leadbuf + i + 8);

				devio_lseek(fd, start_sec * SECTOR_SIZE, 0);
				if(devio_read(fd, diskbuf, 16 * SECTOR_SIZE) !=
						16 * SECTOR_SIZE) {
					printf("Read the super block error!\n");
					goto out;
				}

				ext2_sb = (struct ext2_super_block *)(diskbuf + 1024);
				if(ext2_sb->s_magic == EXT3_SUPER_MAGIC)
					find_linux_partion++;
				if(index == find_linux_partion) {
					err = 0;
					goto out;
				}
			}
		}
	}


	devio_lseek(fd, start_sec * SECTOR_SIZE, 0);
	if(devio_read(fd, diskbuf, 16 * SECTOR_SIZE) != 16 * SECTOR_SIZE)
		printf("Read the super block error!\n");

	ext2_sb = (struct ext2_super_block *)(diskbuf + 1024);

	if(ext2_sb->s_magic == EXT3_SUPER_MAGIC)
		err = 0;
out:
	if(!err) {
#ifdef DEBUG_IDE
		dump_feature(ext2_sb);
#endif

		if(ext2_sb->s_feature_incompat & EXT2_FEAT_INCOMP_64BIT) {
			sb_grpdesc_size = 64;
			/* now we not support 64bit bit file system */
			printf("not support on 64bit feauture");
			err = -1;
		}

		if(ext2_sb->s_rev_level==0) //old version
		sb_ext2_inode_size = 128;
		else
		sb_ext2_inode_size = ext2_sb->s_inode_size;
		sb_inodes_per_grp = ext2_sb->s_inodes_per_group;
		sb_block_size = BLOCK_1KB << ext2_sb->s_log_block_size;
		sb_grpdecs_per_block = sb_block_size / sb_grpdesc_size;
	}
	if(leadbuf)
		free(leadbuf);
	if(diskbuf)
		free(diskbuf);
	return err;
}

/*
 * p is at least 6 bytes before the end of page
 */
static inline ext2_dirent *ext2_next_entry(ext2_dirent *p)
{
	return (ext2_dirent *)((char*)p + le16_to_cpu(p->rec_len));
}

/* ext2 entry name is not null terminated,so we could not use strcmp
 * return 0 if the first 'len' characters of 'entry' match those of 's'
 */
static int ext2_entrycmp(char * s,void * entry , int len)
{
	int i;
	if (strlen(s) != len)
		return -1;
	for(i = 0; i < len; i++)
		if(*(char *)(s + i) != *(char *)((char *)entry + i))
			return -1;
	return 0;
}

/*
 * allocated a ext2_inode, and filled with inode info pointed by ino
 * out: ext2_raw_inode_ptr
 * return 0 for success
 * -1 for error */
static int ext2_get_inode(int fd, unsigned long ino, struct ext2_inode **ext2_raw_inode_ptr)
{
	unsigned long offset, block, block_group, group_desc, desc;

	struct ext2_group_desc * ext2_gd;
	unsigned char * bh;
	off_t temp;
	int err = -1;
	/* in which block group*/
	block_group = (ino - 1) / sb_inodes_per_grp;
	/* in which block */
	group_desc = block_group / sb_grpdecs_per_block ;
	/*
	 * introduction block maybe the same as super block
	 */
	block = 1024 / sb_block_size + 1 + group_desc;
	/* which descriptor,inside the block */
	desc = block_group % sb_grpdecs_per_block;
	bh = (unsigned char *)malloc(sb_block_size);
#ifdef DEBUG_IDE
	printf("ext2_get_inode:ino=%d,block group=%d,block=%d,desc=%d\n", ino, block_group, block, desc);
	printf("1 bh:%lx\n", bh);
#endif

	temp = (off_t)block * sb_block_size + start_sec * 512;
	devio_lseek(fd, temp, 0);
	if(sb_block_size != devio_read(fd, bh, sb_block_size)) {
		printf("io read error\n");
		goto out;
	}
	ext2_gd = (struct ext2_group_desc *)(bh + desc * sb_grpdesc_size);
#ifdef DEBUG_IDE
	printf("ext2_group_desc -> bg_free_blocks_count=%d\n", ext2_gd->bg_free_blocks_count);
	printf("ext2_group_desc -> bg_free_inodes_count=%d\n", ext2_gd->bg_free_inodes_count);
	printf("ext2_group_desc -> bg_inode_table=%d\n", ext2_gd->bg_inode_table);
	printf("ext2_group_desc -> bg_block_bitmap=%d\n", ext2_gd->bg_block_bitmap);
	printf("ext2_group_desc -> bg_inode_bitmap=%d\n", ext2_gd->bg_inode_bitmap);
#endif
	offset = ((ino - 1) % sb_inodes_per_grp) * sb_ext2_inode_size;
	block  = ext2_gd->bg_inode_table + (offset / sb_block_size);
	offset = offset % sb_block_size;
#ifdef DEBUG_IDE
	printf("ext2_get_inode: offset is %d,block is %d\n", offset, block);
	printf("2 bh:%lx\n", bh);
#endif
	memset(bh, 0, sb_block_size);
	/*bzero(bh, sb_block_size);*/
	temp = (off_t)block * sb_block_size + start_sec * 512;

#ifdef DEBUG_IDE
	printf("the sb_block_size is 0x%x\n", sb_block_size);
	printf("In ext2fs.c    The seek offset is %llx\n", temp);

#endif
	devio_lseek(fd, temp, 0);
	if(sb_block_size != devio_read(fd, bh, sb_block_size)) {
		printf("io read error\n");
		goto out;
	}

	*ext2_raw_inode_ptr = (struct ext2_inode *)malloc(sb_ext2_inode_size);
	if(!*ext2_raw_inode_ptr) {
		printf("no mem\n");
		goto out;
	}
	memcpy(*ext2_raw_inode_ptr, bh + offset, sb_ext2_inode_size);
	err = 0;

#ifdef DEBUG_IDE
	printf("inode->i_block[0]=%d,the inode->i_size=%d \n",
			(*ext2_raw_inode_ptr)->i_block[0], (*ext2_raw_inode_ptr)->i_size);
#endif
out:
	free(bh);
	return err;
}

/*
 * load linux kernel from ext2 partition
 * return 0 if success,else -1
 */

static int ext2_load_linux(int fd,int index, const unsigned char *path)
{
	struct ext2_inode *ext2_raw_inode;
	ext2_dirent *de;
	unsigned char *bh;
	int i, bh_size;
	unsigned int inode;
	int find = 1;
	unsigned char s[EXT2_NAME_LEN];
	unsigned char pathname[EXT2_NAME_LEN], *pathnameptr;
	unsigned char *directoryname;
	int showdir, lookupdir;

	showdir = 0;
	lookupdir = 0;
	bh = 0;
	bh_size = 0;

	if(read_super_block(fd,index))
		return -1;

	if((path[0]==0) || (path[strlen(path)-1] == '/'))
		lookupdir = 1;

	strncpy(pathname,path,sizeof(pathname));
	pathnameptr = pathname;
	for(inode = EXT2_ROOT_INO; find; ) {
		for(i = 0; pathnameptr[i] && pathnameptr[i] != '/'; i++);

		pathnameptr[i] = 0;

		directoryname = (unsigned char *)pathnameptr;
		pathnameptr = (unsigned char *)(pathnameptr + i + 1);

		if(!strlen(directoryname) && lookupdir)
			showdir = 1;
		if(ext2_get_inode(fd, inode, &ext2_raw_inode)) {
			printf("load EXT2_ROOT_INO error");
			return -1;
		}
		if (!bh || bh_size < sb_block_size + ext2_raw_inode->i_size)
		{
			if(bh) free(bh);
			bh = (unsigned char *)malloc(sb_block_size + ext2_raw_inode->i_size);
			bh_size = sb_block_size + ext2_raw_inode->i_size;
		}

		if(!bh) {
			printf("Error in allocting memory for file content!\n");
			return -1;
		}
		if(ext2_read_file(fd, bh, ext2_raw_inode->i_size, 0,
					ext2_raw_inode) != ext2_raw_inode->i_size)
			return -1;
		de = (ext2_dirent *)bh;
		find = 0;

		for ( ; ((unsigned char *) de < bh + ext2_raw_inode->i_size) &&
				(de->rec_len > 0) && (de->name_len > 0); de = ext2_next_entry(de)) {
			strncpy(s,de->name,de->name_len);
			s[de->name_len]='\0';//*(de->name+de->name_len)='\0';
#ifdef DEBUG_IDE
			printf("entry:name=%s,inode=%d,rec_len=%d,name_len=%d,file_type=%d\n",s,de->inode,de->rec_len,de->name_len,de->file_type);
#endif
			if(showdir)
				printf("%s%s",s,((de->file_type)&2)?"/ ":" ");

			if (!ext2_entrycmp(directoryname, de->name, de->name_len)) {
				if(de->file_type == EXT2_FT_REG_FILE || de->file_type == EXT2_FT_UNKNOWN) {
					if (ext2_get_inode(fd, de->inode, &the_inode)) {
						printf("load EXT2_ROOT_INO error");
						free(bh);
						return -1;
					}
					free(bh);
					return 0;
				}
				find = 1;
				inode = de->inode;
				break;
			}
		}
		if(!find) {
			free(bh);
			if(!lookupdir)
				printf("Not find the file or directory!\n");
			else
				printf("\n");
			return -1;
		}
	}
	return -1;
}



/*
 * for cmd: load /dev/fs/ext2@wd0/boot/vmlinux
 * the path we got here is wd0/boot/vmlinux
 * */
int ext2_open(int fd,const char *path,int flags,int mode)
{
	int i, index;
	char strbuf[EXT2_NAME_LEN], *str;
	char *p;

	strncpy(strbuf,path,sizeof(strbuf));

	for(i = 0; strbuf[i]&&(strbuf[i]!='/'); i++) ;

	if(!strbuf[i]){
		printf("the DEV Name  is expected!\n");
		return -1;
	}
	strbuf[i] = 0;
	p = &strbuf[strlen(strbuf)-1];
	if((p[0]>='a') && (p[0]<='z')) {
		index=p[0]-'a'+1;
		p[0]=0;
	}
	else if(p[0]=='A'||!strcmp(strbuf,"fd0"))
		index=0;
	else
		index=1;

	/* extract the device name */
	if(devio_open(fd,strbuf,flags,mode) < 0)
		return -1;
#ifdef DEBUG_IDE
	printf("Open the device %s ok\n", strbuf);
#endif
	str = strbuf + i + 1;
	if(!(ext2_load_linux(fd, index, str)))
		return fd;
	if((str[0] != 0) && (str[strlen(str)-1] != '/'))
		printf("we can't locate root directory in super block!\n");
	return -1;
}

int ext2_close(int fd)
{
	_file[fd].posn = 0;
	return devio_close(fd);
}

/*
 * ReadBuf: point to the file content
 * size: the real number still need to be read
 * position: the file point where the read start
 * return:
 * 0 -- successful
 * others -- error
 * */
static int ReadFromIndexBlock(int fd, __u32 start_block, __u32 end_block, __u8 **ReadBuf, size_t *size, size_t *position, __u32 *addr_start)
{
	__u32 remain_count;
	int re;
	off_t remain_size, addr_abosolute_start;
	if(start_block > end_block)
		return 0;
#ifdef DEBUG_IDE
	printf("I want to read data size :%u, start_block is %d,end_block is %u\n",
			*size , start_block, end_block);
#endif
	/*Read the unaligned data within a block. */
	remain_size = sb_block_size - *position % sb_block_size;
	if(remain_size > *size) {
		remain_size = *size;
		remain_count = 0;
	}
	else if(remain_size == sb_block_size) {
		remain_size = 0;
		remain_count = 0;
	}
	else
		remain_count = 1;

	addr_start += start_block;

	/*  start_block starts with 0. 0-11:direct others 0-1023.*/
	addr_abosolute_start =
		(off_t)*addr_start * sb_block_size + *position % sb_block_size;
	if(remain_count) {
		devio_lseek(fd, addr_abosolute_start + START_PARTION, 0);
		re = devio_read(fd, (__u8 *)*ReadBuf, remain_size);
		if(re != remain_size) {
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
			(off_t)*addr_start * sb_block_size + *position % sb_block_size;
		remain_size = 0;
	}
	/*Read the BLOCK aligned data*/
	while(*size && remain_size < *size &&
			(remain_count+start_block <= end_block)) {
		if( *(addr_start + remain_count + 1) ==
				*(addr_start + remain_count) + 1) {
			if(remain_size + sb_block_size > *size)
				remain_size = *size;
			else {
				remain_count++;
				remain_size += sb_block_size;
			}
		} else {
#ifdef DEBUG_IDE
			printf("The remain size is %u ,size is%u\n", remain_size, *size);
			printf("Block begin at %u, end at %u\n",
					*addr_start, *(addr_start + remain_count));
#endif
			if(!remain_size) {
				/*if not continuous,we just read one block*/

				if(*size < sb_block_size)
					remain_size =* size;
				else {
					remain_size = sb_block_size;
					remain_count = 1;
				}
			}
			if(*addr_start == 0 && remain_count == 1) {
				memset((__u8 *)*ReadBuf,0,remain_size);
				re=remain_size;
			} else {
				devio_lseek(fd, addr_abosolute_start + START_PARTION, 0);
				re = devio_read(fd, (__u8 *)*ReadBuf, remain_size);
			}
			if(re!=remain_size) {
				printf("We can't read data from disk!\n");
				return -1;
			}
			start_block += remain_count;
			addr_start += remain_count;
			remain_count = 0;
			*ReadBuf += remain_size;
			*position += remain_size;
			*size -= remain_size;
			if((*position % sb_block_size) && *size) {
				printf("Oh,My God!When I read in the aligned data,I met one unaligned position\n");
				return -1;
			}
			addr_abosolute_start = ((off_t)*addr_start) * sb_block_size;
			remain_size = 0;
		}
	}

	/*No data need to read.Wonderful!*/
	if(!*size)
		return 0;
	if(remain_size) {
		if(remain_size > *size)
			remain_size = *size;
		devio_lseek(fd, addr_abosolute_start + START_PARTION, 0);
#ifdef DEBUG_IDE
		printf("The remain size is %u,size is %u\n", remain_size, *size);
		printf("Additional Block begin at %d,end at %d\n", *addr_start, *(addr_start + remain_count));
#endif
		re = devio_read(fd, (__u8 *)*ReadBuf, remain_size);
		*ReadBuf += remain_size;
		if(re != remain_size) {
			printf("We can't read data from disk!\n");
			return -1;
		}
		*position += remain_size;
		*size -= remain_size;
	}
	return 0;
}

int ext2_read(int fd, void *read_start,size_t size)
{
	int real_size;

	if((_file[fd].posn + size) > the_inode->i_size) {
		size = the_inode->i_size - _file[fd].posn;
	}

	memset(read_start, 0, size);

	real_size = ext2_read_file(fd, read_start, size, _file[fd].posn, the_inode);
	if((_file[fd].posn + real_size) > the_inode->i_size) {
		real_size = the_inode->i_size - _file[fd].posn;
		_file[fd].posn = the_inode->i_size;
	} else
		_file[fd].posn += real_size;

	return real_size;
}

/*
 * return the extent node for file_block of file pointed
 * by extent_hdr
 * fd : file desc to block device
 * buff: the buffer provided by caller with 1 block size
 * extent_hdr: the root node of the extent tree of the file
 * file_block: the file block number of the file
 */
static struct ext4_extent_hdr * get_extent_node(int fd, __u8 *buff,
		struct ext4_extent_hdr *extent_hdr, __u32 file_block)
{
	struct ext4_extent_idx *idx;
	unsigned long long block;
	int i;

	while(1) {
		idx = (struct ext4_extent_idx *)(extent_hdr + 1);
#ifdef	DEBUG_IDE
		printf("eh_depth:%d, eh_entries:%d, eh_max:%d, block:%d\n",
				extent_hdr->eh_depth, extent_hdr->eh_entries,
				extent_hdr->eh_max, file_block);
#endif
		if(le16_to_cpu(extent_hdr->eh_magic) != EXT4_EXT_MAGIC)
			return 0;

		if(!extent_hdr->eh_depth)
			return extent_hdr;
		i = -1;
		do {
			i++;
			if(i >= le16_to_cpu(extent_hdr->eh_entries))
				break;
#ifdef	DEBUG_IDE
			printf("ei_hi:%d, ei_block:%d, ei_lo:%d\n",
					idx[i].ei_leaf_hi, idx[i].ei_block,
					idx[i].ei_leaf_lo);
#endif

		} while(file_block >= le32_to_cpu(idx[i].ei_block));

		if(--i < 0)
			return 0;

		block = le16_to_cpu(idx[i].ei_leaf_hi);
		block = (block << 32) | le32_to_cpu(idx[i].ei_leaf_lo);

		devio_lseek(fd, block * sb_block_size + START_PARTION, 0);
		if(devio_read(fd, buff, sb_block_size) != sb_block_size)
			return 0;
		else
			extent_hdr = (struct ext4_extent_hdr *)buff;

	}


}

/*
 * return the block nr for file logical number file_block
 * fd: the device holding the filesystem
 * idx: the root extent header for the filesystem
 * file_block: the logical file logical number
 */
static long long read_extent_block(int fd, struct ext4_extent_hdr *idx, __u8  *ext_buff,
		__u32 file_block, int *leftblks)
{
	unsigned long long blk;
	struct ext4_extent_hdr *leaf_node;
	struct ext4_extent *extent;
	int i = -1;
	__u32 next_block = file_block + 1;


	leaf_node = get_extent_node(fd, ext_buff, idx, file_block);
	if(!leaf_node) {
		printf("leaf error\n");
		return -1;
	}
#ifdef DEBUG_IDE
	printf("depth:%d, entry:%d, max%d, block:%d\n", leaf_node->eh_depth,
			leaf_node->eh_entries, leaf_node->eh_max,
			file_block);
#endif


	extent = (struct ext4_extent *)(leaf_node + 1);
	do {
		++i;
		if(i >= le16_to_cpu(leaf_node->eh_entries))
			break;
#ifdef DEBUG_IDE
		printf("ent:%d, ee_block:%d, ee_len:%d, ee_start_lo %d\n",
				i, extent[i].ee_block, extent[i].ee_len,
				extent[i].ee_start_lo);
#endif
		next_block = le32_to_cpu(extent[i].ee_block);
	} while(file_block >= next_block);


	if(--i >= 0) {
		file_block -= extent[i].ee_block;
#ifdef DEBUG_IDE
		printf("block:%d, ee_block:%d, ee_len:%d, ee_start_lo %d\n",
				file_block, extent[i].ee_block, extent[i].ee_len,
				extent[i].ee_start_lo);
#endif

		blk = le16_to_cpu(extent[i].ee_start_hi);
		blk = blk << 32 |
			le32_to_cpu(extent[i].ee_start_lo);
#ifdef DEBUG_IDE
		printf("blk:%ld, ret %ld\n", blk, file_block + blk);
#endif
		if(file_block >= le16_to_cpu(extent[i].ee_len)) {
		/* find a hole */
			*leftblks = next_block - file_block -  extent[i].ee_block;
			return -2;
		}
		else
		*leftblks = le16_to_cpu(extent[i].ee_len) - file_block;
		return file_block + blk;
	}

	*leftblks = next_block - file_block;
	return -2;
}


/*
 * read file with extent feature
 */

static int ext2_read_file1(int fd, void *read_start,
		size_t size, size_t pos, struct ext2_inode *inode)
{
	struct ext4_extent_hdr *extent_hdr;

	long long blk;
	size_t off;
	int i;
	int leftblks;
	__u32 blk_start = pos / sb_block_size;
	__u32 blk_end = (pos + size + sb_block_size - 1) / sb_block_size;
	 __u8  *ext_buff;
	off = 0;
#ifdef DEBUG_IDE
	printf("size:%d, pos:%d flags:0x%x\n", size, pos, inode->i_flags);
#endif

	extent_hdr = (struct ext4_extent_hdr *)(inode->i_block);
	ext_buff = (unsigned char *)malloc(sb_block_size);
	if (!ext_buff) {
		printf("no mem!\n");
		return -1;
	}

	for(i = blk_start, leftblks = 0; i < blk_end; i++) {
		int skip;
		int len;
		int ret;

		if (!leftblks)
			blk = read_extent_block(fd, extent_hdr, ext_buff, i, &leftblks);
		else if (blk >= 0)
		   blk++;
		leftblks--;
#ifdef DEBUG_IDE
		printf("blk:%lld, file_block:%d\n", blk, i);
#endif
		if(blk == -1)
		{
			goto out;
		}

		if(i == pos / sb_block_size) {
			skip = pos % sb_block_size;
			len = (size <= (sb_block_size - skip) ?
					size : sb_block_size - skip);
		} else {
			skip = 0;
			len = (size <= sb_block_size ?
					size : sb_block_size);

		}

		if(!len)
			break;


		if(blk>=0)
		{
			devio_lseek(fd, blk * sb_block_size + START_PARTION + skip, 0);
		ret = devio_read(fd, read_start + off, len);
 		}
		else
		{
			 memset(read_start + off, 0, len);
			 ret = len;
		}
#ifdef DEBUG_IDE
		printf("ret:%d, size:%d, off:%d, skip:%d, len:%d \n",
				ret, size, off, skip, len);
#endif
		if(ret < 0)
		{
			goto out;
		}
		if(ret != len)
		{
			off = ret + off;
			goto out;
		}

		size -= len;
		off += len;
	}
out:

	free(ext_buff);
#ifdef DEBUG_IDE
	printf("size:%d, off:%d\n", size, off);
#endif
	return off;
}
static int ext2_read_file0(int fd, void *read_start, size_t size, size_t pos,
		struct ext2_inode *inode)
{
	__u32 *addr_start, *d_addr_start, start_block;
	int re, i;
	__u8 *buff, *index_buff, *start = (__u8 *)read_start;
	size_t read_size = size, position = pos;

	start_block = position / sb_block_size;
	addr_start = inode->i_block;
#ifdef DEBUG_IDE
	printf("the pos is %llx,the size is %llx\n", position, read_size);
#endif
	re = ReadFromIndexBlock(fd, start_block, 11, &start, &read_size,
			&position, addr_start);
#ifdef DEBUG_IDE
	printf("The addr_start is 0x%x, sb_block_size is 0x%x, start_sec is 0x%x\n",
			*addr_start, sb_block_size, start_sec);
#endif
	if(re) {
		printf("Error in Reading from direct block\n");
		return 0;
	}
	if(!read_size) {
#ifdef DEBUG_IDE
		for(i = 0; i < size; i += sb_block_size/4)
			printf("%4x",(__u8)*((__u8 *)read_start + i));
		printf("\n");
#endif
		return (int)size;
	}
	start_block = position / sb_block_size - 12;
	buff = (__u8 *)malloc(sb_block_size);
	if(!buff) {
		printf("Can't alloc memory!\n");
		return 0;
	}
	addr_start = &(inode->i_block[12]);
	devio_lseek(fd, (off_t)*addr_start * sb_block_size + START_PARTION, 0);
	re = devio_read(fd, buff, sb_block_size);
	if(re != sb_block_size) {
		printf("Read the iblock[12] error!\n");
		return 0;
	}
	addr_start = (__u32 *)buff;
	re = ReadFromIndexBlock(fd, start_block, sb_block_size/4-1, &start,
			&read_size, &position, addr_start);
	if(re) {
		free((char*)buff);	/*spark add*/
		return 0;
	}
	if(!read_size) {
#ifdef DEBUG_IDE
		for(i = 0; i < size; i += sb_block_size/4)
			printf("%4x",(__u8)*((__u8 *)read_start + i));
		printf("\n");
#endif
		free((char*)buff);              /* spark add */
		return (int)size;
	}

	/* Read Double index block */
	addr_start = &(inode->i_block[13]);
	devio_lseek(fd,(off_t)*addr_start * sb_block_size + START_PARTION, 0);
	re = devio_read(fd, buff, sb_block_size);
	if(re != sb_block_size) {
		printf("Read the iblock[13] error!\n");
		free((char*)buff);	/* spark add */
		return 0;
	}
	d_addr_start = (__u32 *)buff;
	index_buff = (__u8 *)malloc(sb_block_size);
	if(!index_buff) {
		printf("Can't alloc memory!\n");
		return 0;
	}
	for(i = 0;i < sb_block_size/4; i++) {
		devio_lseek(fd, (off_t)*(d_addr_start+i) * sb_block_size + START_PARTION, 0);
		re = devio_read(fd, index_buff, sb_block_size);
		if(re != sb_block_size) {
			printf("Can't read index block!\n");
			return 0;
		}
		addr_start = (__u32 *)index_buff;
		start_block = position/sb_block_size - 12 - sb_block_size/4 * (i + 1);
		re = ReadFromIndexBlock(fd, start_block, sb_block_size/4 - 1, &start, &read_size,
				&position, addr_start);
		if(re) {
			printf("Can't read the double index block!\n");
			free((char*)buff);
			free((char*)index_buff);	/* spark add */
			return 0;
		}
		if(!read_size) {
			free((char*)buff);		/* spark add */
			free((char*)index_buff);
			return (int)size;
		}
	}
	//triple

	addr_start=&(inode->i_block[14]);
	devio_lseek(fd,(off_t)*addr_start*sb_block_size+START_PARTION,0);
	re=devio_read(fd,buff,sb_block_size);
	if(re!=sb_block_size)
	{
		printf("Read the iblock[13] error!\n");
		free((char*)buff);	/* spark add */
		return 0;
	}
	d_addr_start=(__u32 *)buff;
	index_buff=(__u8 *)malloc(sb_block_size);
	if(!index_buff)
	{
		printf("Can't alloc memory!\n");
		return 0;
	}

		start_block=(position/sb_block_size-12-sb_block_size/4-sb_block_size/4*sb_block_size/4)/(sb_block_size/4*sb_block_size/4);
	for(i=start_block;i<sb_block_size/4-1;i++)
	{	
		devio_lseek(fd,(off_t)*(d_addr_start+i)*sb_block_size+START_PARTION,0);
		re=devio_read(fd,index_buff,sb_block_size);
		if(re!=sb_block_size)
		{
			printf("Can't read index block!\n");
			return 0;
		}
		addr_start=(__u32 *)index_buff;
		start_block=(position/sb_block_size-12-sb_block_size/4-sb_block_size/4*sb_block_size/4-sb_block_size/4*sb_block_size/4*i)/(sb_block_size/4);
		re=ReadFromIndexBlock2(fd,start_block,sb_block_size/4-1,&start,&read_size,&position,addr_start);
		if(re)
		{
			printf("Can't read the double index block!\n");
			free((char*)buff);
			free((char*)index_buff);	/* spark add */
			return 0;
		}
		if(!read_size) {
			free((char*)buff);		/* spark add */
			free((char*)index_buff);
			return (int)size;
		}
	}
	return 0;
}

static int ReadFromIndexBlock2(int fd,__u32 start_index,__u32 end_index,__u8 **start,	size_t *size,size_t *position,__u32 *d_addr_start)
{
//addr_start is a array
//start_block1 is arrys's index
	__u32 start_block;
	__u32 *addr_start;
	__u8 *index_buff;
	int i, re;

	if(start_index>end_index)
		return 0;

	index_buff=(__u8 *)malloc(sb_block_size);
	if(!index_buff)
	{
		printf("Can't alloc memory!\n");
		return 0;
	}

	for(i=start_index;i<end_index;i++)
	{	
		devio_lseek(fd,(off_t)*(d_addr_start+i)*sb_block_size+START_PARTION,0);
		re=devio_read(fd,index_buff,sb_block_size);
		if(re!=sb_block_size)
		{
			printf("Can't read index block!\n");
			return 0;
		}
		addr_start=(__u32 *)index_buff;
		start_block= (*position/sb_block_size-12)%(sb_block_size/4);
		re=ReadFromIndexBlock(fd,start_block,sb_block_size/4-1,start,size,position,addr_start);
		if(re)
		{
			printf("Can't read the double index block!\n");
			free((char*)index_buff);	/* spark add */
			return 0;
		}
		if(!*size) {
			free((char*)index_buff);
			return 0;
		}
	}
	return 0;
}

static int ext2_read_file(int fd, void *read_start, size_t size, size_t pos,
		struct ext2_inode *inode)
{
	int use_extent;
	use_extent = inode->i_flags & EXT4_EXTENTS_FL;

	if(!use_extent)
		return ext2_read_file0(fd, read_start, size, pos, inode);
	else
		return ext2_read_file1(fd, read_start, size, pos, inode);

}

int ext2_write(int fd,const void *start,size_t size)
{
	return 0;
}
off_t ext2_lseek(int fd,off_t offset,int where)
{
	_file[fd].posn = offset;
	return offset;
}
static DiskFileSystem diskfile = {
	"ext2",
	ext2_open,
	ext2_read,
	ext2_write,
	ext2_lseek,
	ext2_close,
	NULL
};
static void init_diskfs(void) __attribute__ ((constructor));
static void init_diskfs()
{
	diskfs_init(&diskfile);
}
