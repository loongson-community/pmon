/*
 * This file is part of UBIFS.
 *
 * Copyright (C) 2006-2008 Nokia Corporation.
 *
 * (C) Copyright 2008-2010
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Authors: Artem Bityutskiy (Битюцкий Артём)
 *          Adrian Hunter
 */
#include "ubifs.h"
#include <linux/mtd/zlib.h>
#include <pmon.h>
#include <file.h>
#include <diskfs.h>
#include<mtdfile.h>
#include<linux/mtd/nand.h>
#include<sys/stat.h>
#include<sys/fcntl.h>
extern int errno;
extern LIST_HEAD(mtdfiles, mtdfile) mtdfiles;
static struct inode *inode;
static struct ubifs_info *c ;
static int FD;
static char *volname,*filename;
extern int   devio_open (int, const char *, int, int);
extern int   devio_close (int);
extern int   devio_read (int, void *, size_t);
extern int   devio_write (int, const void *, size_t);
extern off_t devio_lseek (int, off_t, int);
int   ubifs_open (int, const char *, int, int);
int   ubifs_close (int);
int   ubifs_read (int, void *, size_t);
int   ubifs_write (int, const void *, size_t);
off_t ubifs_lseek (int, off_t, int);

DECLARE_GLOBAL_DATA_PTR;

/* compress.c */

/*
 * We need a wrapper for zunzip() because the parameters are
 * incompatible with the lzo decompressor.
 */
static int gzip_decompress(const unsigned char *in, size_t in_len,
			   unsigned char *out, size_t *out_len)
{
	unsigned long len = in_len;
	return zunzip(out, *out_len, (unsigned char *)in, &len, 0, 0);
}

/* Fake description object for the "none" compressor */
static struct ubifs_compressor none_compr = {
	.compr_type = UBIFS_COMPR_NONE,
	.name = "no compression",
	.capi_name = "",
	.decompress = NULL,
};

static struct ubifs_compressor lzo_compr = {
	.compr_type = UBIFS_COMPR_LZO,
	.name = "LZO",
	.capi_name = "lzo",
	.decompress = lzo1x_decompress_safe,
};

static struct ubifs_compressor zlib_compr = {
	.compr_type = UBIFS_COMPR_ZLIB,
	.name = "zlib",
	.capi_name = "deflate",
	.decompress = gzip_decompress,
};

/* All UBIFS compressors */
struct ubifs_compressor *ubifs_compressors[UBIFS_COMPR_TYPES_CNT];

/**
 * ubifs_decompress - decompress data.
 * @in_buf: data to decompress
 * @in_len: length of the data to decompress
 * @out_buf: output buffer where decompressed data should
 * @out_len: output length is returned here
 * @compr_type: type of compression
 *
 * This function decompresses data from buffer @in_buf into buffer @out_buf.
 * The length of the uncompressed data is returned in @out_len. This functions
 * returns %0 on success or a negative error code on failure.
 */
int ubifs_decompress(const void *in_buf, int in_len, void *out_buf,
		     int *out_len, int compr_type)
{
	int err;
	struct ubifs_compressor *compr;
	if (unlikely(compr_type < 0 || compr_type >= UBIFS_COMPR_TYPES_CNT)) {
		printf("invalid compression type %d", compr_type);
		return -EINVAL;
	}
	compr = ubifs_compressors[compr_type];

	if (unlikely(!compr->capi_name)) {
		printf("%s compression is not compiled in", compr->name);
		return -EINVAL;
	}
	if (compr_type == UBIFS_COMPR_NONE) {
		memcpy(out_buf, in_buf, in_len);
		*out_len = in_len;
		return 0;
	}
	err = compr->decompress(in_buf, in_len, out_buf, (size_t *)out_len);
	if (err)
		printf("cannot decompress %d bytes, compressor %s, "
			  "error %d", in_len, compr->name, err);

	return err;
}

/**
 * compr_init - initialize a compressor.
 * @compr: compressor description object
 *
 * This function initializes the requested compressor and returns zero in case
 * of success or a negative error code in case of failure.
 */
static int __init compr_init(struct ubifs_compressor *compr)
{
	ubifs_compressors[compr->compr_type] = compr;

#ifdef CONFIG_NEEDS_MANUAL_RELOC
	ubifs_compressors[compr->compr_type]->name += gd->reloc_off;
	ubifs_compressors[compr->compr_type]->capi_name += gd->reloc_off;
	ubifs_compressors[compr->compr_type]->decompress += gd->reloc_off;
#endif

	return 0;
}

/**
 * ubifs_compressors_init - initialize UBIFS compressors.
 *
 * This function initializes the compressor which were compiled in. Returns
 * zero in case of success and a negative error code in case of failure.
 */
int __init ubifs_compressors_init(void)
{
	int err;

	err = compr_init(&lzo_compr);
	if (err)
		return err;

	err = compr_init(&zlib_compr);
	if (err)
		return err;

	err = compr_init(&none_compr);
	if (err)
		return err;

	return 0;
}

/*
 * ubifsls...
 */

static int filldir(struct ubifs_info *c, const char *name, int namlen,
		   u64 ino, unsigned int d_type)
{
	struct inode *inode;
	char filetime[32];

	switch (d_type) {
	case UBIFS_ITYPE_REG:
		printf("\t");
		break;
	case UBIFS_ITYPE_DIR:
		printf("<DIR>\t");
		break;
	case UBIFS_ITYPE_LNK:
		printf("<LNK>\t");
		break;
	default:
		printf("other\t");
		break;
	}

	inode = ubifs_iget(c->vfs_sb, ino);
	if (IS_ERR(inode)) {
		printf("%s: Error in ubifs_iget(), ino=%lld ret=%p!\n",
		       __func__, ino, inode);
		return -1;
	}
	ctime_r((time_t *)&inode->i_mtime, filetime);
	printf("%9lld  %24.24s  ", inode->i_size, filetime);
	ubifs_iput(inode);

	printf("%s\n", name);

	return 0;
}

static int ubifs_printdir(struct file *file, void *dirent)
{
	int err, over = 0;
	struct qstr nm;
	union ubifs_key key;
	struct ubifs_dent_node *dent;
	struct inode *dir = file->f_path.dentry->d_inode;
	struct ubifs_info *c = dir->i_sb->s_fs_info;

	dbg_gen("dir ino %lu, f_pos %#llx", dir->i_ino, file->f_pos);

	if (file->f_pos > UBIFS_S_KEY_HASH_MASK || file->f_pos == 2)
		/*
		 * The directory was seek'ed to a senseless position or there
		 * are no more entries.
		 */
		return 0;

	if (file->f_pos == 1) {
		/* Find the first entry in TNC and save it */
		lowest_dent_key(c, &key, dir->i_ino);
		nm.name = NULL;
		dent = ubifs_tnc_next_ent(c, &key, &nm);
		if (IS_ERR(dent)) {
			err = PTR_ERR(dent);
			goto out;
		}

		file->f_pos = key_hash_flash(c, &dent->key);
		file->private_data = dent;
	}

	dent = file->private_data;
	if (!dent) {
		/*
		 * The directory was seek'ed to and is now readdir'ed.
		 * Find the entry corresponding to @file->f_pos or the
		 * closest one.
		 */
		dent_key_init_hash(c, &key, dir->i_ino, file->f_pos);
		nm.name = NULL;
		dent = ubifs_tnc_next_ent(c, &key, &nm);
		if (IS_ERR(dent)) {
			err = PTR_ERR(dent);
			goto out;
		}
		file->f_pos = key_hash_flash(c, &dent->key);
		file->private_data = dent;
	}

	while (1) {
		dbg_gen("feed '%s', ino %llu, new f_pos %#x",
			dent->name, (unsigned long long)le64_to_cpu(dent->inum),
			key_hash_flash(c, &dent->key));
		ubifs_assert(le64_to_cpu(dent->ch.sqnum) > ubifs_inode(dir)->creat_sqnum);

		nm.len = le16_to_cpu(dent->nlen);
		over = filldir(c, (char *)dent->name, nm.len,
			       le64_to_cpu(dent->inum), dent->type);
		if (over)
			return 0;

		/* Switch to the next entry */
		key_read(c, &dent->key, &key);
		nm.name = (char *)dent->name;
		dent = ubifs_tnc_next_ent(c, &key, &nm);
		if (IS_ERR(dent)) {
			err = PTR_ERR(dent);
			goto out;
		}

		kfree(file->private_data);
		file->f_pos = key_hash_flash(c, &dent->key);
		file->private_data = dent;
		cond_resched();
	}

out:
	if (err != -ENOENT) {
		ubifs_err("cannot find next direntry, error %d", err);
		return err;
	}

	kfree(file->private_data);
	file->private_data = NULL;
	file->f_pos = 2;
	return 0;
}

static int ubifs_finddir(struct super_block *sb, char *dirname,
			 unsigned long root_inum, unsigned long *inum)
{
	int err;
	struct qstr nm;
	union ubifs_key key;
	struct ubifs_dent_node *dent;
	struct ubifs_info *c;
	struct file *file;
	struct dentry *dentry;
	struct inode *dir;

	file = kzalloc(sizeof(struct file), 0);
	dentry = kzalloc(sizeof(struct dentry), 0);
	dir = kzalloc(sizeof(struct inode), 0);
	if (!file || !dentry || !dir) {
		printf("%s: Error, no memory for malloc!\n", __func__);
		err = -ENOMEM;
		goto out;
	}

	dir->i_sb = sb;
	file->f_path.dentry = dentry;
	file->f_path.dentry->d_parent = dentry;
	file->f_path.dentry->d_inode = dir;
	file->f_path.dentry->d_inode->i_ino = root_inum;
	c = sb->s_fs_info;

	dbg_gen("dir ino %lu, f_pos %#llx", dir->i_ino, file->f_pos);

	/* Find the first entry in TNC and save it */
	lowest_dent_key(c, &key, dir->i_ino);
	nm.name = NULL;
	dent = ubifs_tnc_next_ent(c, &key, &nm);
	if (IS_ERR(dent)) {
		err = PTR_ERR(dent);
		goto out;
	}

	file->f_pos = key_hash_flash(c, &dent->key);
	file->private_data = dent;

	while (1) {
		dbg_gen("feed '%s', ino %llu, new f_pos %#x",
			dent->name, (unsigned long long)le64_to_cpu(dent->inum),
			key_hash_flash(c, &dent->key));
		ubifs_assert(le64_to_cpu(dent->ch.sqnum) > ubifs_inode(dir)->creat_sqnum);

		nm.len = le16_to_cpu(dent->nlen);
		if ((strncmp(dirname, (char *)dent->name, nm.len) == 0) &&
		    (strlen(dirname) == nm.len)) {
			*inum = le64_to_cpu(dent->inum);
			return 1;
		}

		/* Switch to the next entry */
		key_read(c, &dent->key, &key);
		nm.name = (char *)dent->name;
		dent = ubifs_tnc_next_ent(c, &key, &nm);
		if (IS_ERR(dent)) {
			err = PTR_ERR(dent);
			goto out;
		}

		kfree(file->private_data);
		file->f_pos = key_hash_flash(c, &dent->key);
		file->private_data = dent;
		cond_resched();
	}

out:
	if (err != -ENOENT) {
		ubifs_err("cannot find next direntry, error %d", err);
		return err;
	}

	if (file)
		free(file);
	if (dentry)
		free(dentry);
	if (dir)
		free(dir);

	if (file->private_data)
		kfree(file->private_data);
	file->private_data = NULL;
	file->f_pos = 2;
	return 0;
}

static unsigned long ubifs_findfile(struct super_block *sb, char *filename)
{
	int ret;
	char *next;
	char fpath[128];
	char symlinkpath[128];
	char *name = fpath;
	unsigned long root_inum = 1;
	unsigned long inum;
	int symlink_count = 0; /* Don't allow symlink recursion */
	char link_name[64];

	strcpy(fpath, filename);

	/* Remove all leading slashes */
	while (*name == '/')
		name++;

	/*
	 * Handle root-direcoty ('/')
	 */
	inum = root_inum;
	if (!name || *name == '\0')
		return inum;

	for (;;) {
		struct inode *inode;
		struct ubifs_inode *ui;

		/* Extract the actual part from the pathname.  */
		next = strchr(name, '/');
		if (next) {
			/* Remove all leading slashes.  */
			while (*next == '/')
				*(next++) = '\0';
		}

		ret = ubifs_finddir(sb, name, root_inum, &inum);
		if (!ret)
			return 0;
		inode = ubifs_iget(sb, inum);

		if (!inode)
			return 0;
		ui = ubifs_inode(inode);

		if ((inode->i_mode & S_IFMT) == S_IFLNK) {
			char buf[128];

			/* We have some sort of symlink recursion, bail out */
			if (symlink_count++ > 8) {
				printf("Symlink recursion, aborting\n");
				return 0;
			}
			memcpy(link_name, ui->data, ui->data_len);
			link_name[ui->data_len] = '\0';

			if (link_name[0] == '/') {
				/* Absolute path, redo everything without
				 * the leading slash */
				next = name = link_name + 1;
				root_inum = 1;
				continue;
			}
			/* Relative to cur dir */
			sprintf(buf, "%s/%s",
					link_name, next == NULL ? "" : next);
			memcpy(symlinkpath, buf, sizeof(buf));
			next = name = symlinkpath;
			continue;
		}

		/*
		 * Check if directory with this name exists
		 */

		/* Found the node!  */
		if (!next || *next == '\0')
			return inum;

		root_inum = inum;
		name = next;
	}

	return 0;
}

int ubifs_ls(char *filename)
{
	struct ubifs_info *c = ubifs_sb->s_fs_info;
	struct file *file;
	struct dentry *dentry;
	struct inode *dir;
	void *dirent = NULL;
	unsigned long inum;
	int ret = 0;

	c->ubi = ubi_open_volume(c->vi.ubi_num, c->vi.vol_id, UBI_READONLY);
	inum = ubifs_findfile(ubifs_sb, filename);
	if (!inum) {
		ret = -1;
		goto out;
	}

	file = kzalloc(sizeof(struct file), 0);
	dentry = kzalloc(sizeof(struct dentry), 0);
	dir = kzalloc(sizeof(struct inode), 0);
	if (!file || !dentry || !dir) {
		printf("%s: Error, no memory for malloc!\n", __func__);
		ret = -ENOMEM;
		goto out_mem;
	}

	dir->i_sb = ubifs_sb;
	file->f_path.dentry = dentry;
	file->f_path.dentry->d_parent = dentry;
	file->f_path.dentry->d_inode = dir;
	file->f_path.dentry->d_inode->i_ino = inum;
	file->f_pos = 1;
	file->private_data = NULL;
	ubifs_printdir(file, dirent);

out_mem:
	if (file)
		free(file);
	if (dentry)
		free(dentry);
	if (dir)
		free(dir);

out:
	ubi_close_volume(c->ubi);
	return ret;
}

/*
 * ubifsload...
 */

/* file.c */

static inline void *kmap(struct page *page)
{
	return page->addr;
}

static int read_block(struct inode *inode, void *addr, unsigned int block,
		      struct ubifs_data_node *dn)
{
	struct ubifs_info *c = inode->i_sb->s_fs_info;
	int err, len, out_len;
	union ubifs_key key;
	unsigned int dlen;

	data_key_init(c, &key, inode->i_ino, block);
	err = ubifs_tnc_lookup(c, &key, dn);
	if (err) {
		if (err == -ENOENT)
			/* Not found, so it must be a hole */
			memset(addr, 0, UBIFS_BLOCK_SIZE);
		return err;
	}

	ubifs_assert(le64_to_cpu(dn->ch.sqnum) > ubifs_inode(inode)->creat_sqnum);

	len = le32_to_cpu(dn->size);
	if (len <= 0 || len > UBIFS_BLOCK_SIZE)
		goto dump;

	dlen = le32_to_cpu(dn->ch.len) - UBIFS_DATA_NODE_SZ;
	out_len = UBIFS_BLOCK_SIZE;
	err = ubifs_decompress(&dn->data, dlen, addr, &out_len,
			       le16_to_cpu(dn->compr_type));
	if (err || len != out_len)
		goto dump;

	/*
	 * Data length can be less than a full block, even for blocks that are
	 * not the last in the file (e.g., as a result of making a hole and
	 * appending data). Ensure that the remainder is zeroed out.
	 */
	if (len < UBIFS_BLOCK_SIZE)
		memset(addr + len, 0, UBIFS_BLOCK_SIZE - len);

	return 0;

dump:
	ubifs_err("bad data node (block %u, inode %lu)",
		  block, inode->i_ino);
	dbg_dump_node(c, dn);
	return -EINVAL;
}

static int do_readpage(struct ubifs_info *c, struct inode *inode,
		       struct page *page)
{
	void *addr;
	int err = 0, i;
	unsigned int block, beyond;
	struct ubifs_data_node *dn;
	loff_t i_size = inode->i_size;

	dbg_gen("ino %lu, pg %lu, i_size %lld",
		inode->i_ino, page->index, i_size);

	addr = kmap(page);

	block = page->index << UBIFS_BLOCKS_PER_PAGE_SHIFT;
	beyond = (i_size + UBIFS_BLOCK_SIZE - 1) >> UBIFS_BLOCK_SHIFT;
	if (block >= beyond) {
		/* Reading beyond inode */
		memset(addr, 0, PAGE_CACHE_SIZE);
		goto out;
	}

	dn = kmalloc(UBIFS_MAX_DATA_NODE_SZ, GFP_NOFS);
	if (!dn)
		return -ENOMEM;

	i = 0;
	while (1) {
		int ret;

		if (block >= beyond) {
			/* Reading beyond inode */
			err = -ENOENT;
			memset(addr, 0, UBIFS_BLOCK_SIZE);
		} else {
				ret = read_block(inode, addr, block, dn);
				if (ret) {
					err = ret;
					if (err != -ENOENT)
						break;
				}
			}
		if (++i >= UBIFS_BLOCKS_PER_PAGE)
			break;
		block += 1;
		addr += UBIFS_BLOCK_SIZE;
	}
	if (err) {
		if (err == -ENOENT) {
			/* Not found, so it must be a hole */
			dbg_gen("hole");
			goto out_free;
		}
		ubifs_err("cannot read page %lu of inode %lu, error %d",
			  page->index, inode->i_ino, err);
		goto error;
	}

out_free:
	kfree(dn);
out:
	return 0;

error:
	kfree(dn);
	return err;
}
int ubifs_load(char *filename, u32 addr,u32 size)
{
	struct page page;
	int err = 0;
	int i;
	char *tmpbuf=malloc(PAGE_SIZE);
	if(!tmpbuf)
	{
		puts("there is no enough memory!!");
		return -1;
	}
	int startaddr =_file[FD].posn;
	int endaddr = startaddr+size;
	if(endaddr > inode->i_size)
	{
		puts("read beyond the file's size!!");
		return -1;
	}
	int inds=startaddr/PAGE_SIZE;
	int offs=startaddr%PAGE_SIZE;
	int inde=endaddr/PAGE_SIZE;
	int offe=endaddr%PAGE_SIZE;
	page.addr = (void *)tmpbuf;
	page.index = inds;
	page.inode = inode;
	if(inds==inde)
        {
			err = do_readpage(c, inode, &page);
               	 	if (err)
			 {
				puts("read error");
                        	return -1;

                        }
			memcpy(addr,tmpbuf+offs,size);
			return 0;
        }

	for (i = inds; i <= inde; i++) {
		err = do_readpage(c, inode, &page);
		if (err)
			break;
		/*the first requested block*/
		if(i==inds)
		{
		 	memcpy(addr,tmpbuf+offs,PAGE_SIZE-offs);	
			addr+=PAGE_SIZE-offs;
		}
		/*the last requested block*/
		else if(i==inde)
		{
			memcpy(addr,tmpbuf,offe+1);
			addr+=(offe+1);
		}
		else
		{
			memcpy(addr,tmpbuf,PAGE_SIZE);
			addr+=PAGE_SIZE;
		}
		page.index++;
		
	}

	if (err)
	{
		printf("Error reading file '%s'\n", filename);
		return -1;
	}
	return 0;

}


struct ubi_device *ubi;
static void display_ubi_info(struct ubi_device *ubi)
{
        ubi_msg("MTD device name:            \"%s\"", ubi->mtd->name);
        ubi_msg("MTD device size:            %llu MiB", ubi->flash_size >> 20);
        ubi_msg("physical eraseblock size:   %d bytes (%d KiB)",
                        ubi->peb_size, ubi->peb_size >> 10);
        ubi_msg("logical eraseblock size:    %d bytes", ubi->leb_size);
        ubi_msg("number of good PEBs:        %d", ubi->good_peb_count);
        ubi_msg("number of bad PEBs:         %d", ubi->bad_peb_count);
        ubi_msg("smallest flash I/O unit:    %d", ubi->min_io_size);
        ubi_msg("VID header offset:          %d (aligned %d)",
                        ubi->vid_hdr_offset, ubi->vid_hdr_aloffset);
        ubi_msg("wear-leveling threshold:    %d", CONFIG_MTD_UBI_WL_THRESHOLD);
        ubi_msg("number of internal volumes: %d", UBI_INT_VOL_COUNT);
        ubi_msg("number of user volumes:     %d",
                        ubi->vol_count - UBI_INT_VOL_COUNT);
        ubi_msg("available PEBs:             %d", ubi->avail_pebs);
        ubi_msg("total number of reserved PEBs: %d", ubi->rsvd_pebs);
        ubi_msg("number of PEBs reserved for bad PEB handling: %d",
                        ubi->beb_rsvd_pebs);
        ubi_msg("max/mean erase counter: %d/%d", ubi->max_ec, ubi->mean_ec);
}
static void ubi_dump_vol_info( struct ubi_volume *vol)
{
        ubi_msg("volume information dump:");
        ubi_msg("vol_id          %d", vol->vol_id);
        ubi_msg("reserved_pebs   %d", vol->reserved_pebs);
        ubi_msg("alignment       %d", vol->alignment);
        ubi_msg("data_pad        %d", vol->data_pad);
        ubi_msg("vol_type        %d", vol->vol_type);
        ubi_msg("name_len        %d", vol->name_len);
        ubi_msg("usable_leb_size %d", vol->usable_leb_size);
        ubi_msg("used_ebs        %d", vol->used_ebs);
        ubi_msg("used_bytes      %lld", vol->used_bytes);
        ubi_msg("last_eb_bytes   %d", vol->last_eb_bytes);
        ubi_msg("corrupted       %d", vol->corrupted);
        ubi_msg("upd_marker      %d", vol->upd_marker);

        if (vol->name_len <= UBI_VOL_NAME_MAX &&
                strnlen(vol->name, vol->name_len + 1) == vol->name_len) {
                ubi_msg("name            %s", vol->name);
        } else {
                ubi_msg("the 1st 5 characters of the name: %c%c%c%c%c",
                                vol->name[0], vol->name[1], vol->name[2],
                                vol->name[3], vol->name[4]);
        }
        printf("\n");
}
static void display_volume_info(struct ubi_device *ubi)
{
        int i;

        for (i = 0; i < (ubi->vtbl_slots + 1); i++) {
                if (!ubi->volumes[i])
                        continue;       /* Empty record */
                ubi_dump_vol_info(ubi->volumes[i]);
        }
}
static int verify_mkvol_req( struct ubi_device *ubi,
                             struct ubi_mkvol_req *req)
{
        int n, err = EINVAL;

        if (req->bytes < 0 || req->alignment < 0 || req->vol_type < 0 ||
            req->name_len < 0)
                goto bad;

        if ((req->vol_id < 0 || req->vol_id >= ubi->vtbl_slots) &&
            req->vol_id != UBI_VOL_NUM_AUTO)
                goto bad;

        if (req->alignment == 0)
                goto bad;

        if (req->bytes == 0) {
                printf("No space left in UBI device!\n");
                err = ENOMEM;
                goto bad;
        }

        if (req->vol_type != UBI_DYNAMIC_VOLUME &&
            req->vol_type != UBI_STATIC_VOLUME)
                goto bad;

        if (req->alignment > ubi->leb_size)
                goto bad;

        n = req->alignment % ubi->min_io_size;
        if (req->alignment != 1 && n)
                goto bad;

        if (req->name_len > UBI_VOL_NAME_MAX) {
                printf("Name too long!\n");
                err = ENAMETOOLONG;
                goto bad;
        }

        return 0;
bad:
        return err;
}
static struct ubi_volume *ubi_find_volume(char *volume)
{
        struct ubi_volume *vol = NULL;
        int i;

        for (i = 0; i < ubi->vtbl_slots; i++) {
                vol = ubi->volumes[i];
                if (vol && !strcmp(vol->name, volume))
                        return vol;
        }

        printf("Volume %s not found!\n", volume);
        return NULL;
}

static int ubi_remove_vol(char *volume)
{
        int err, reserved_pebs, i;
        struct ubi_volume *vol;

        vol = ubi_find_volume(volume);
        if (vol == NULL)
                return ENODEV;

        printf("Remove UBI volume %s (id %d)\n", vol->name, vol->vol_id);

        if (ubi->ro_mode) {
                printf("It's read-only mode\n");
                err = EROFS;
                goto out_err;
        }

        err = ubi_change_vtbl_record(ubi, vol->vol_id, NULL);
        if (err) {
                printf("Error changing Vol tabel record err=%x\n", err);
                goto out_err;
        }
        reserved_pebs = vol->reserved_pebs;
        for (i = 0; i < vol->reserved_pebs; i++) {
                err = ubi_eba_unmap_leb(ubi, vol, i);
                if (err)
                        goto out_err;
        }

        kfree(vol->eba_tbl);
        ubi->volumes[vol->vol_id]->eba_tbl = NULL;
        ubi->volumes[vol->vol_id] = NULL;

        ubi->rsvd_pebs -= reserved_pebs;
        ubi->avail_pebs += reserved_pebs;
        i = ubi->beb_rsvd_level - ubi->beb_rsvd_pebs;
        if (i > 0) {
                i = ubi->avail_pebs >= i ? i : ubi->avail_pebs;
                ubi->avail_pebs -= i;
                ubi->rsvd_pebs += i;
                ubi->beb_rsvd_pebs += i;
                if (i > 0)
                        ubi_msg("reserve more %d PEBs", i);
        }
        ubi->vol_count -= 1;

        return 0;
out_err:
        ubi_err("cannot remove volume %s, error %d", volume, err);
        if (err < 0)
                err = -err;
        return err;
}

static int ubi_volume_write(char *volume, void *buf, size_t size)
{
        int err = 1;
        int rsvd_bytes = 0;
        struct ubi_volume *vol;

        vol = ubi_find_volume(volume);
        if (vol == NULL)
                return ENODEV;

        rsvd_bytes = vol->reserved_pebs * (ubi->leb_size - vol->data_pad);
        if (size < 0 || size > rsvd_bytes) {
                printf("size > volume size! Aborting!\n");
                return EINVAL;
        }

        err = ubi_start_update(ubi, vol, size);
        if (err < 0) {
                printf("Cannot start volume update\n");
                return -err;
        }

        err = ubi_more_update_data(ubi, vol, buf, size);
        if (err < 0) {
                printf("Couldnt or partially wrote data\n");
                return -err;
                                                            
        }

        if (err) {
                size = err;

                err = ubi_check_volume(ubi, vol->vol_id);
                if (err < 0)
                        return -err;

                if (err) {
                        ubi_warn("volume %d on UBI device %d is corrupted",
                                        vol->vol_id, ubi->ubi_num);
                        vol->corrupted = 1;
                }

                vol->checked = 1;
                ubi_gluebi_updated(vol);
        }

        printf("%d bytes written to volume %s\n", size, volume);

        return 0;
}

static int ubi_volume_read(char *volume, char *buf, size_t size)
{
        int err, lnum, off, len, tbuf_size;
        size_t count_save = size;
        void *tbuf;
        unsigned long long tmp;
        struct ubi_volume *vol;
        loff_t offp = 0;

        vol = ubi_find_volume(volume);
        if (vol == NULL)
                return ENODEV;

        printf("Read %d bytes from volume %s to %p\n", size, volume, buf);

        if (vol->updating) {
                printf("updating");
                return EBUSY;
        }
        if (vol->upd_marker) {
                printf("damaged volume, update marker is set");
                return EBADF;
        }
        if (offp == vol->used_bytes)
                return 0;

        if (size == 0) {
                printf("No size specified -> Using max size (%lld)\n", vol->used_bytes);
                size = vol->used_bytes;
        }

        if (vol->corrupted)
                printf("read from corrupted volume %d", vol->vol_id);
        if (offp + size > vol->used_bytes)
                count_save = size = vol->used_bytes - offp;

        tbuf_size = vol->usable_leb_size;
        if (size < tbuf_size)
                tbuf_size = ALIGN(size, ubi->min_io_size);
        tbuf = malloc(tbuf_size);
        if (!tbuf) {
                printf("NO MEM\n");
                return ENOMEM;
        }
        len = size > tbuf_size ? tbuf_size : size;

        tmp = offp;
        off = do_div(tmp, vol->usable_leb_size);
        lnum = tmp;
        do {
                if (off + len >= vol->usable_leb_size)
                        len = vol->usable_leb_size - off;
                                                                                     
                err = ubi_eba_read_leb(ubi, vol, lnum, tbuf, off, len, 0);
                if (err) {
                        printf("read err %x\n", err);
                        err = -err;
                        break;
                }
                off += len;
                if (off == vol->usable_leb_size) {
                        lnum += 1;
                        off -= vol->usable_leb_size;
                }

                size -= len;
                offp += len;

                memcpy(buf, tbuf, len);

                buf += len;
                len = size > tbuf_size ? tbuf_size : size;
        } while (size);

        free(tbuf);
        return err;
}
static int ubi_create_vol(char *volume, int size, int dynamic)
{
        struct ubi_mkvol_req req;
        int err;

        if (dynamic)
                req.vol_type = UBI_DYNAMIC_VOLUME;
        else
                req.vol_type = UBI_STATIC_VOLUME;

        req.vol_id = UBI_VOL_NUM_AUTO;
        req.alignment = 1;
        req.bytes = size;

        strcpy(req.name, volume);
        req.name_len = strlen(volume);
        req.name[req.name_len] = '\0';
        req.padding1 = 0;
        /* It's duplicated at drivers/mtd/ubi/cdev.c */
        err = verify_mkvol_req(ubi, &req);
        if (err) {
                printf("verify_mkvol_req failed %d\n", err);
                return err;
        }
        printf("Creating %s volume %s of size %d\n",
                dynamic ? "dynamic" : "static", volume, size);
        /* Call real ubi create volume */
        return ubi_create_volume(ubi, &req);
}



/*the only supported path is: /dev/ubifs@mtdx/volname/filename
*@mtdx:the MTD device number:x 
*@volname:the volume name:volname
*@filename:file name
*/                                                                                   

ubifs_open(int fd, const char *path, int flags, int mode)
{
	struct mtd_info *mt;
	const char *opath;
	int ubinum;
	int err = 0;
	int mtdx;
        unsigned long inum;
	char *poffset,*poffset2;
	FD=fd;
	/*  Try to get to the physical device */
	opath = path;
	if (strncmp(opath, "/dev/", 5) == 0)
		opath += 5;
	else
		return -1;
	if (strncmp(opath, "ubifs@mtd", 9) == 0)
		opath += 9;
	else
		return -1;
	if(isdigit(opath[0]))
	{
		if((poffset=strchr(opath,'/')))
			 *poffset++=0;
         	mtdx=strtoul(opath,0,0);
	}
	else
		return -1;
	if((poffset2=strchr(poffset,'/')))
	{
		*poffset2++=0;
		volname=poffset;
	}
	else
		return -1;
	filename=poffset2;
	mtdfile *p;
        LIST_FOREACH(p, &mtdfiles, i_next)
        {
		if(p->mtd->index==mtdx)
			break;
	}	
	if(!p)
	{
		puts("there is no MTD device numberd %d!!\n",mtdx);
		return -1;
	}
	mt=p->mtd;
	if((ubinum=ubi_attach_mtd_dev(mt, 0, 0))<0)
	{
		puts("ubi_attach_mtd_dev error!!");
		return -1;
	}
	if(ubifs_init())
	{
                puts("ubifs initialized failed!!");
        	return -1;
	}
	if(ubifs_mount(volname))
	{
                puts("ubifs mounted failed!!");
		return -1;
	}
        c = ubifs_sb->s_fs_info;
        c->ubi = ubi_open_volume(c->vi.ubi_num, c->vi.vol_id, UBI_READONLY);
        /* ubifs_findfile will resolve symlinks, so we know that we get
         * the real file here */
        //printf("c->vi.ubi_num=%d,c->vi.vol_id=%d,c->vi.vol_size=%d,c->vi.vol_name=%s\n",c->vi.ubi_num,c->vi.vol_id,c->vi.size,c->vi.name);
        inum = ubifs_findfile(ubifs_sb, filename);
        if (!inum) {
		puts("inum not found");
		return -1;
        }
        /*
         * Read file inode
         */
        inode = ubifs_iget(ubifs_sb, inum);
        if (IS_ERR(inode)) {
                printf("%s: Error reading inode %ld!\n", __func__, inum);
		return -1;
        }
	return (fd);
}


int ubifs_read(int fd, void *addr, size_t size)
{
	void *buf=addr;
	int rev=ubifs_load(fd,buf,size);
	if(rev==-1)
	{
		puts("ubifs_load error!!");
		return -1;
	}
	_file[FD].posn+=size;
	return size;
}
off_t
ubifs_lseek(int fd, off_t offset, int whence)
{
	switch (whence) {
                case SEEK_SET:
                        _file[fd].posn = offset;
                        break;
                case SEEK_CUR:
                        _file[fd].posn += offset;
                        break;
                case SEEK_END:
                        _file[fd].posn = inode->i_size+offset;
                        break;
                default:
                        errno = EINVAL;
                        return (-1);
        }
        return (_file[fd].posn);
}
int
ubifs_write(int fd, const void *start, size_t size)
{
        return 0;
}
int
ubifs_close(int fd)
{
	ubi_detach_mtd_dev(0, 1); 
	ubifs_sb = NULL;
	ubifs_iput(inode);
        return (0);
}


/*
 *  File system registration info.
 */
static FileSystem ubifs = {
        "ubifs", FS_FILE,
        ubifs_open,
        ubifs_read,
        ubifs_write,
        ubifs_lseek,
        ubifs_close,
        NULL
};

static void init_fs(void) __attribute__ ((constructor));

static void
init_fs()
{
        filefs_init(&ubifs);
}

