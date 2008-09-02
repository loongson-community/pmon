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
/* $Id: ramfile.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

#include <stdio.h>
#include <string.h>
#include <termio.h>
#include <fcntl.h>
#include <file.h>
#include <ctype.h>
#include <mtdfile.h>
#include <sys/unistd.h>
#include <stdlib.h>
#undef _KERNEL
#include <errno.h>
#include "logfile.h"
#include <pmon.h>

LIST_HEAD(mtdfiles, mtdfile) mtdfiles = LIST_HEAD_INITIALIZER(mtdfiles);
static int mtdidx=0;

static int mtdfile_open (int , const char *, int, int);
static int mtdfile_close (int);
static int mtdfile_read (int, void *, size_t);
static int mtdfile_write (int, const void *, size_t);
static off_t mtdfile_lseek (int, off_t, int);

int highmemcpy(long long dst,long long src,long long count);
void mycacheflush(long long addrin,unsigned int size,unsigned int rw);

/*
 * Supported paths:
 *	/dev/mtd/0@offset,size
 */

static int
   mtdfile_open (fd, fname, mode, perms)
   int	       fd;
   const char *fname;
   int         mode;
   int	       perms;
{
	mtdfile *p;
	mtdpriv *priv;
	char    *dname,namebuf[64];
	int idx=-1;
	int found = 0;
	int open_size=0;
	int open_offset=0;
	char *poffset=0,*psize;
      
	strncpy(namebuf,fname,63);
	dname = namebuf;
	if (strncmp (dname, "/dev/", 5) == 0)
		dname += 5;
	if (strncmp (dname, "mtd", 3) == 0)
		dname += 3;
	else return -1;
	
	if(dname[0]=='*') idx=-1;
	else if(dname[0]=='/')
	{
	dname++;
	 if(dname[0])
	 {
	if((poffset=strchr(dname,'@'))) *poffset++=0;
		LIST_FOREACH(p, &mtdfiles, i_next) {
		if(!strcmp(p->name,dname))
		goto foundit;
		}
	 }
	}
	else if(isdigit(dname[0])){
	if((poffset=strchr(dname,'@'))) *poffset++=0;
	idx=strtoul(dname,0,0);
	}
	else idx=-1;


		
		LIST_FOREACH(p, &mtdfiles, i_next) {
			if(idx==-1)
			 {
			 if(p->part_offset||(p->part_size!=p->mtd->size))
			  printf("mtd%d: flash:%d size:%d writesize:%d partoffset=0x%x,partsize=%d %s\n",p->index,p->mtd->index,p->mtd->size,p->mtd->writesize,p->part_offset,p->part_size,p->name);
			 else
			  printf("mtd%d: flash:%d size:%d writesize:%d %s\n",p->index,p->mtd->index,p->mtd->size,p->mtd->writesize,p->name);
			}
			else if(p->index==idx) {
				found = 1;
				break;
			}	
		}
		
		if(!found) {
			return(-1);
		}
foundit:
	
	if(poffset)
	{
	 if((psize=strchr(poffset,',')))
	 {
	  *psize++=0;
	  open_size=strtoul(psize,0,0);
	 }
	 open_offset=strtoul(poffset,0,0);
	}
	
	p->refs++;
	priv=malloc(sizeof(struct mtdpriv));
	priv->file=p;
	priv->open_size=open_size?open_size:p->part_size;
	priv->open_offset=open_offset;

	_file[fd].posn = 0;
	_file[fd].data = (void *)priv;
	
	return (fd);
}

/** close(fd) close fd */
static int
   mtdfile_close (fd)
   int             fd;
{
	mtdpriv *priv;

	priv = (mtdpriv *)_file[fd].data;
	priv->file->refs--;
	free(priv);
	   
	return(0);
}

/** read(fd,buf,n) read n bytes into buf from fd */
static int
   mtdfile_read (fd, buf, n)
   int fd;
   void *buf;
   size_t n;
{
	mtdfile        *p;
	mtdpriv *priv;

	priv = (mtdpriv *)_file[fd].data;
	p = priv->file;

	if (_file[fd].posn + n > priv->open_size)
		n = priv->open_size - _file[fd].posn;

	p->mtd->read(p->mtd,_file[fd].posn+priv->open_offset+p->part_offset,n,&n,buf);

	_file[fd].posn += n;
      
	return (n);
}


static int
   mtdfile_write (fd, buf, n)
   int fd;
   const void *buf;
   size_t n;
{
	mtdfile        *p;
	mtdpriv *priv;
	struct erase_info erase;
	unsigned int start_addr;

	priv = (mtdpriv *)_file[fd].data;
	p = priv->file;

	if (_file[fd].posn + n > priv->open_size)
		n = priv->open_size - _file[fd].posn;

	start_addr= _file[fd].posn+priv->open_offset+p->part_offset;

	erase.mtd = p->mtd;
	erase.callback = 0;
	erase.addr = (start_addr+mtd->erasesize-1)&~(mtd->erasesize-1);
	erase.len = n;
	erase.priv = 0;

	if(erase.addr>=start_addr && erase.addr<start_addr+erase.len)
	{
	p->mtd->erase(p->mtd,&erase);
	}
	p->mtd->write(p->mtd,start_addr,n,&n,buf);
	_file[fd].posn += n;

	return (n);
}

/*************************************************************
 *  mtdfile_lseek(fd,offset,whence)
 */
static off_t
mtdfile_lseek (fd, offset, whence)
	int             fd, whence;
	off_t            offset;
{
	mtdfile        *p;
	mtdpriv *priv;

	priv = (mtdpriv *)_file[fd].data;
	p = priv->file;


	switch (whence) {
		case SEEK_SET:
			_file[fd].posn = offset;
			break;
		case SEEK_CUR:
			_file[fd].posn += offset;
			break;
		case SEEK_END:
			_file[fd].posn = priv->open_size + offset;
			break;
		default:
			errno = EINVAL;
			return (-1);
	}
	return (_file[fd].posn);
}



int add_mtd_device(struct mtd_info *mtd,int offset,int size,char *name)
{
	struct mtdfile *rmp;
	int len=sizeof(struct mtdfile);
	if(name)len+=strlen(name);

	rmp = (struct mtdfile *)malloc(len);
	if (rmp == NULL) {
		fprintf(stderr, "Out of space adding mtdfile");
		return(NULL);
	}

	bzero(rmp, len);
	rmp->mtd =mtd;
	rmp->index=mtdidx++;
	rmp->part_offset=offset;
	if(size) rmp->part_size=size;
	else rmp->part_size=mtd->size;
	if(name)strcpy(rmp->name,name);
	LIST_INSERT_HEAD(&mtdfiles, rmp, i_next);
	return 0;
}

int del_mtd_device (struct mtd_info *mtd)
{
	struct mtdfile *rmp;

	LIST_FOREACH(rmp, &mtdfiles, i_next) {
		if(rmp->mtd==mtd) {
			if(rmp->refs == 0) {
				LIST_REMOVE(rmp, i_next);
				free(rmp);
				return(0);
			} else {
				return(-1);
			}
		}
	}
	return(-1);
}



static FileSystem mtdfs =
{
	"mtd", FS_MEM,
	mtdfile_open,
	mtdfile_read,
	mtdfile_write,
	mtdfile_lseek,
	mtdfile_close,
	NULL
};

static void init_fs __P((void)) __attribute__ ((constructor));

static void
   init_fs()
{
	/*
	 * Install ram based file system.
	 */
	filefs_init(&mtdfs);

}

