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
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/list.h>
#if 0
#define MTD_FLAGS_CHAR      0x1 /*main+oob,all data */
#define MTD_FLAGS_BLOCK     0x2 /*main;but no OOB */
#define MTD_FLAGS_RAW       0x4 /*raw mtd,no check bad part*/
#define MTD_FLAGS_CHAR_MARK 0x8 /*main+oob,but bad_mark auto*/
#define MTD_FLAGS_GOOD      0x10 /*use good part info*/
#endif
#define min(a,b) (((a)<(b))?(a):(b))
#define prefetch(x) x

LIST_HEAD(mtdfiles, mtdfile) mtdfiles = LIST_HEAD_INITIALIZER(mtdfiles);
static struct mtdfile *ptail;
static int mtdidx=0;
static struct mtdfile *goodpart0=0;

static int mtdfile_open (int , const char *, int, int);
static int mtdfile_close (int);
static int mtdfile_read (int, void *, size_t);
static int mtdfile_write (int, const void *, size_t);
static off_t mtdfile_lseek (int, off_t, int);

int highmemcpy(long long dst,long long src,long long count);
void mycacheflush(long long addrin,unsigned int size,unsigned int rw);

#define TO_MIN(x,y,z)   min(min((p->mtd->erasesize - ((z) & (p->mtd->erasesize-1))),(x)),(y))
static creat_part_trans_table(mtdfile *p)
{
    struct mtd_info *mtd=p->mtd;
    struct nand_chip *this = (struct nand_chip*)mtd->priv;
    int start = p->part_offset/mtd->erasesize;
    int end = (p->part_offset + p->part_size)/mtd->erasesize;
    int tmp=0,len,good;
    int *table =NULL;
    if(p->trans_table) free(p->trans_table);
    len = (end-start+1)*sizeof(int); 
    table = (int*)malloc(len);
    for(tmp=0,good=start;tmp<(end-start);tmp++){
        for(;good<end;good++){
            if(!(this->bbt[good>>2] & (0x3<<((good%4 )<<1))))break;
        }
        if(good==end)break;
        table[tmp]=good++;
    }
    p->part_size = tmp*mtd->erasesize; 
    p->trans_table = table;
}

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
    int idx=-1,flags=0;
    int found = 0;
    unsigned long long open_size=0;
    unsigned long long  open_offset=0;
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
#if 0
            if((poffset=strchr(dname,'c'))){ 
                *poffset++=0;
                mtd2mtdc=1;
            }
#endif
            if((poffset=strchr(dname,'@'))) *poffset++=0;
            LIST_FOREACH(p, &mtdfiles, i_next) {
                if(!strcmp(p->name,dname))
                    goto foundit;
            }
        }
    }
    else if(isdigit(dname[0])){
	 int i;
	 for(i=0; isdigit(dname[i]); i++);
	 for(i=0; dname[i] && dname[i] != '/' && !poffset;i++)
         {
             switch(dname[i])
             {
                 case 'c':
                     if(!flags){ 
                         flags |= MTD_FLAGS_CHAR;break;
                     }else
                         break;
                 case 'y': 
                     if(!flags){
                         flags |= MTD_FLAGS_CHAR_MARK; break;
                     }else
                         break;
                 case 'b': 
                     if(!flags){
                         flags |= MTD_FLAGS_BLOCK; break;
                     }else
                         break;
                 case 'r': 
                     if(!flags){
                         flags |= MTD_FLAGS_RAW; break;
                     }else
                         break;
                     //	   case 'g': flags |= FLAGS_GOOD;break;
                 case '@': poffset = &dname[i+1];break;
                 default:
                           break;

             }
         }
        idx=strtoul(dname,0,0);
    }
    else idx=-1;
    LIST_FOREACH(p, &mtdfiles, i_next) {
        if(idx==-1)
        {
#define ftype(x) ((x==MTD_NANDFLASH)?"nand":(x==MTD_NORFLASH)?"nor":"others")
            if(p->part_offset||(p->part_size!=p->mtd->size))
                printf("mtd%d: flash:%s type:%s size:0x%llx writesize:0x%x erasesize:0x%x partoffset=0x%llx,partsize=0x%llx %s\n",p->index,p->mtd->name,ftype(p->mtd->type),p->mtd->size,p->mtd->writesize,p->mtd->erasesize,p->part_offset,p->part_size,p->name);
            else
                printf("mtd%d: flash:%s type:%s size:0x%llx writesize:0x%x erasesize:0x%x %s\n",p->index,p->mtd->name, ftype(p->mtd->type), p->mtd->size,p->mtd->writesize,p->mtd->erasesize, p->name);
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
            if(flags & MTD_FLAGS_RAW || flags & MTD_FLAGS_CHAR ||flags & MTD_FLAGS_CHAR_MARK){
                open_size=strtoull(psize,0,0);
            }else
            open_size=strtoull(psize,0,0);
        }
        open_offset=strtoull(poffset,0,0);
    }

    p->refs++;
    priv=malloc(sizeof(struct mtdpriv));
    priv->flags = flags;
    priv->file=p;
    priv->open_size=open_size?open_size:(p->part_size-open_offset);
    priv->open_size_real=open_size?open_size:(p->part_size_real-open_offset);
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
    int maxlen,newpos,retlen,left;
    struct mtd_oob_ops ops;
    priv = (mtdpriv *)_file[fd].data;
    p = priv->file;
    if(priv->flags & MTD_FLAGS_CHAR || priv->flags & MTD_FLAGS_RAW ||priv->flags & MTD_FLAGS_CHAR_MARK){
        n -= p->mtd->oobsize;
    }
    left = n;
    if (_file[fd].posn + n > priv->open_size)
        n = priv->open_size - _file[fd].posn;
    if(priv->flags & MTD_FLAGS_CHAR_MARK ||priv->flags & MTD_FLAGS_CHAR|| priv->flags & MTD_FLAGS_RAW ){
        if(n<=0)
            return 0;
        ops.mode = MTD_OOB_RAW;
        ops.ooblen = p->mtd->oobsize;
        ops.ooboffs = 0;
        ops.oobbuf = buf + p->mtd->writesize;
        ops.datbuf = buf; 
        while(left)
        {
            newpos=file_to_mtd_pos(fd,&maxlen);
            ops.len = min(left,maxlen);
            ops.len = min(ops.len,p->mtd->writesize);
            p->mtd->read_oob(p->mtd,newpos,&ops);
            if(ops.retlen<=0)break;
            _file[fd].posn += ops.retlen;
            left -= ops.retlen;
        }
        return (n-left+p->mtd->oobsize);
    }else{
        while(left)
        {
            newpos=file_to_mtd_pos(fd,&maxlen);
            p->mtd->read(p->mtd,newpos,TO_MIN(left,maxlen,newpos),&retlen,buf);
            if(retlen<=0)break;
            _file[fd].posn += retlen;
            buf += retlen;
            left -=retlen;
        }
        return (n-left);
    }
}

static void mark_bad(struct mtd_info *mtd,loff_t addr)
{
	unsigned char buf[mtd->writesize+mtd->oobsize];
	struct mtd_oob_ops ops_r;
	struct erase_info erase;
	loff_t page_addr;
	int i, ret = 0 ;
	erase.mtd = mtd;
	erase.callback = 0;
	erase.addr = (addr+mtd->erasesize-1)&~(mtd->erasesize-1);
	erase.len = (1+mtd->erasesize-1)&~(mtd->erasesize-1);
	erase.priv = 0;
	mtd->erase(mtd,&erase);
	addr = (addr)&~(mtd->erasesize-1);
	ops_r.mode = MTD_OOB_RAW;
	ops_r.len = mtd->writesize;
	ops_r.retlen = 0;
	ops_r.ooblen = mtd->oobsize;
	ops_r.ooboffs = 0;
	ops_r.datbuf = buf;
	ops_r.oobbuf = ( buf + mtd->writesize );


	/*first read after erase*/
	memset(buf,0x00,mtd->writesize+mtd->oobsize);
	ret = mtd->write_oob(mtd,addr,&ops_r); 
}

static	void  copy_block(int fd,mtdfile *p,unsigned int old_start_addr,unsigned int *maxlen)
{
	unsigned char data_buffer[p->mtd->writesize+p->mtd->oobsize];
	unsigned int finished=1;
	unsigned int new_start_addr,new_block_addr,old_block_addr;
	unsigned int page_addr=0;
	struct erase_info erase;
	struct mtd_oob_ops ops;
	struct nand_chip *chip;
	struct mtd_info *mtd;
	chip = (struct nand_chip*)(p->mtd->priv);
	mtd=p->mtd;
	while(finished)
	{
		finished = 0;
		new_start_addr =file_to_mtd_pos(fd,maxlen);
		new_block_addr =(new_start_addr)&~(p->mtd->erasesize-1);
		old_block_addr =(old_start_addr)&~(p->mtd->erasesize-1);
		printf("old_block_addr %x , new_start_addr %x, new_block_addr %x\n",old_block_addr,new_start_addr,new_block_addr); 	
		for(page_addr=0;(page_addr+old_block_addr)< old_start_addr;page_addr+=p->mtd->writesize){

			//printf("i am here *************   page_addr %x\n",page_addr);
			ops.ooblen = p->mtd->oobsize;
			ops.ooboffs = 0;
			ops.oobbuf = data_buffer + p->mtd->writesize;
			ops.datbuf = data_buffer; 
			ops.len = p->mtd->writesize;
			//p->mtd->read_oob(p->mtd,page_addr+old_block_addr,&ops);


			erase.mtd = p->mtd;
			erase.callback = 0;
			erase.addr = (new_start_addr+p->mtd->erasesize-1)&~(p->mtd->erasesize-1);
			erase.len = (1+p->mtd->erasesize-1)&~(p->mtd->erasesize-1);
			erase.priv = 0;

#if 1
			struct mtd_oob_ops ops_w,ops_r;
			ops_w.mode = MTD_OOB_RAW;
			ops_w.len = mtd->writesize;
			ops_w.retlen = 0;
			ops_w.ooblen = mtd->oobsize;
			ops_w.ooboffs = 0;
			ops_w.datbuf = data_buffer;
			ops_w.oobbuf = ( data_buffer + mtd->writesize );


			ops_r.mode = MTD_OOB_RAW;
			ops_r.len = mtd->writesize;
			ops_r.retlen = 0;
			ops_r.ooblen = mtd->oobsize;
			ops_r.ooboffs = 0;
			ops_r.datbuf = data_buffer;
			ops_r.oobbuf = ( data_buffer + mtd->writesize );


			/*first read after erase*/
			mtd->read_oob(mtd,page_addr+old_block_addr,&ops_r); 




#endif
			if(erase.addr>=new_start_addr && erase.addr<new_start_addr+ops.len)
			{
				//    	p->mtd->erase(p->mtd,&erase);
			}

			if(p->mtd->write_oob(p->mtd,page_addr+new_block_addr,&ops_w)){
				p->mtd->block_markbad(p->mtd,new_start_addr); 
				printf("New bad block==%d page==0x%08x addr==0x%08x\n",(new_start_addr / p->mtd->erasesize),(new_start_addr >> chip->page_shift),new_start_addr);
				creat_part_trans_table(p);
				mark_bad(p->mtd,new_start_addr);
				finished = 1;
				break;
			}

		}
	}
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
    unsigned int maxlen,newpos,retlen,left;
    struct mtd_oob_ops ops;
    struct nand_chip *chip;
    priv = (mtdpriv *)_file[fd].data;
    p = priv->file;
    chip = (struct nand_chip*)(p->mtd->priv);
    if(priv->flags & MTD_FLAGS_CHAR_MARK || priv->flags & MTD_FLAGS_CHAR|| priv->flags & MTD_FLAGS_RAW)
        n -= p->mtd->oobsize;
    left=n;
    if (_file[fd].posn + n > priv->open_size)
        n = priv->open_size - _file[fd].posn;
    if(n<=0)
        return 0;
    if(priv->flags & MTD_FLAGS_CHAR_MARK ||priv->flags & MTD_FLAGS_CHAR|| priv->flags & MTD_FLAGS_RAW)
    {
        ops.mode = (priv->flags & MTD_FLAGS_CHAR_MARK) ? MTD_OOB_AUTO : MTD_OOB_RAW;
        while(left)
        {
            start_addr=file_to_mtd_pos(fd,&maxlen);
 //           maxlen=min(left,maxlen);
//            maxlen=(maxlen+p->mtd->writesize-1)&~(p->mtd->writesize-1);
            ops.len=min(maxlen,p->mtd->writesize);

            erase.mtd = p->mtd;
            erase.callback = 0;
            erase.addr = (start_addr+p->mtd->erasesize-1)&~(p->mtd->erasesize-1);
            erase.len = (ops.len+p->mtd->erasesize-1)&~(p->mtd->erasesize-1);
            erase.priv = 0;

            if(erase.addr>=start_addr && erase.addr<start_addr+ops.len)
            {
                p->mtd->erase(p->mtd,&erase);
            }
   //         ops.mode = MTD_OOB_AUTO;
            ops.ooblen = p->mtd->oobsize;
            ops.ooboffs = 0;
            ops.oobbuf = buf+p->mtd->writesize;
            ops.datbuf = buf; 
	    if(p->mtd->write_oob(p->mtd,start_addr,&ops)){
		    p->mtd->block_markbad(p->mtd,start_addr); 
		    printf("New bad block==%d page==0x%08x addr==0x%08x\n",(start_addr / p->mtd->erasesize),(start_addr >> chip->page_shift),start_addr);
		    creat_part_trans_table(p);
		    //               copy_block(fd,p,start_addr,&ops.len);
		    mark_bad(p->mtd,start_addr);

		    continue;
	    }
            if(ops.retlen<=0)break;
            _file[fd].posn += ops.retlen;
            buf += (ops.retlen);
            if(left>ops.retlen) left -=ops.retlen;
            else left=0;
            return (n-left+p->mtd->oobsize);
        }
    }else{
        while(left)
        {
            start_addr=file_to_mtd_pos(fd,&maxlen);
            maxlen=min(left,maxlen);
            maxlen=(maxlen+p->mtd->writesize-1)&~(p->mtd->writesize-1);

            maxlen=p->mtd->writesize;
            erase.mtd = p->mtd;
            erase.callback = 0;
            erase.addr = (start_addr+p->mtd->erasesize-1)&~(p->mtd->erasesize-1);
            erase.len = (maxlen+p->mtd->erasesize-1)&~(p->mtd->erasesize-1);
            erase.priv = 0;

            if(erase.addr>=start_addr && erase.addr<start_addr+maxlen)
            {
                p->mtd->erase(p->mtd,&erase);
            }
            if(p->mtd->write(p->mtd,start_addr,maxlen,&retlen,buf)){
              
		    p->mtd->block_markbad(p->mtd,start_addr); 
		    printf("New bad block==%d page==0x%08x addr==0x%08x\n",(start_addr / p->mtd->erasesize),(start_addr >> chip->page_shift),start_addr);
		    creat_part_trans_table(p);
		    //               copy_block(fd,p,start_addr,&maxlen);
		    mark_bad(p->mtd,start_addr);
		    continue;

            }

            if(retlen<=0)break;
            _file[fd].posn += retlen;
            buf += retlen;
            if(left>retlen) left -=retlen;
            else left=0;

        }
        return (n-left);
    }
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

int add_mtd_device(struct mtd_info *mtd,unsigned long long offset,unsigned long long size,char *name)
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
    else rmp->part_size=mtd->size - offset;
    rmp->part_size_real = rmp->part_size;
    if(name)strcpy(rmp->name,name);

    if(!mtdfiles.lh_first)
    {
	LIST_INSERT_HEAD(&mtdfiles, rmp, i_next);
	ptail = rmp;
    }
    else {
        LIST_INSERT_AFTER(ptail, rmp, i_next);
	ptail = rmp;
    }
    if(mtd->type == MTD_NANDFLASH)
    creat_part_trans_table(rmp);
    return 0;
}
int file_to_mtd_pos(int fd,int *plen)
{
    struct mtdfile *goodpart;
    mtdfile *p;
    mtdpriv *priv;
    int add,file_start,pos;

    priv = (mtdpriv *)_file[fd].data;
    p = priv->file;

    file_start=p->part_offset+priv->open_offset;
    pos=_file[fd].posn;
    if(priv->flags & MTD_FLAGS_GOOD)
    {
    int offset=0;
        goodpart=goodpart0;
        add=goodpart0->part_offset;
        while(goodpart)
        {
            if(file_start<goodpart->part_offset)
                offset += add;
            if(file_start+pos+offset>=goodpart->part_offset && file_start+pos+offset<goodpart->part_offset+goodpart->part_size)
                break;


            if(goodpart->i_next.le_next)
                add=goodpart->i_next.le_next->part_offset - goodpart->part_offset - goodpart->part_size;
            goodpart=goodpart->i_next.le_next;
        }
        if(plen)*plen=(goodpart->part_offset+goodpart->part_size)-(file_start+pos+offset);
    return file_start+pos+offset;
    }
    else if(priv->flags & MTD_FLAGS_RAW)
    {
        if(plen){
            *plen = (p->part_offset + p->part_size)-(file_start + pos);
        }
        return file_start + pos;
    }
    else
    {
    int tmp;
        if(plen){
            *plen=(p->part_offset+p->part_size)-(file_start+pos);
        }
        tmp = pos + priv->open_offset;
        return p->trans_table?((((((int*)(p->trans_table))[tmp/p->mtd->erasesize])*p->mtd->erasesize)|(tmp&(p->mtd->erasesize -1)))):file_start + pos;
    }
    
}

int del_mtd_device (struct mtd_info *mtd)
{
	struct mtdfile *rmp;

	LIST_FOREACH(rmp, &mtdfiles, i_next) {
		if(rmp->mtd==mtd) {
                        if(rmp->refs == 0) {
				if(ptail == rmp) ptail = list_entry(*rmp->i_next.le_prev, struct mtdfile, i_next.le_next);
				LIST_REMOVE(rmp, i_next);
                                mtdidx--;
                                free(rmp->trans_table);
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



static int cmd_flash_erase(int argc,char **argv)
{
    char *path=0, *buf=NULL,str[250]={0};
    int fp=-1,retlen, ret;
    unsigned int start=0,end=0 ;
    unsigned int c,jffs2=0, checkbb = 0, foundbb = 0;
    mtdfile *p;
    mtdpriv *priv;
    struct mtd_info *mtd;
    struct erase_info erase;
    unsigned char clearmark[12]={0x85,0x19,  0x03,0x20,  0x0c,0x00,0x00,0x00,  0xb1,0xb0,0x1e,0xe4};
    optind = 0;
    while((c = getopt(argc, argv, "hHjc")) != EOF) {
        switch(c){    
            case 'h':
            case 'H':
                sprintf(str,"%s","h mtd_erase");
                do_cmd(str);
                return 0;
            case 'j':
                jffs2 = 1;
                break;
	    case 'c':
		checkbb = 1;
		break;
            default :
                return 0;
        }
    }
    if(optind < argc)
        path = argv[optind++];


    if(!path)return -1;
    fp = open(path,O_RDWR|O_CREAT|O_TRUNC);
    if(!fp){printf("open file error!\n");return -1;}
    if(strncmp(_file[fp].fs->devname,"mtd",3)){
        printf("the DEV(\"%s\") isn't flash-dev\n",path);
        close(fp);
        return -1;
    }
    priv = (mtdpriv*)_file[fp].data;
    p = priv->file;
    mtd = p->mtd;
    
    buf = (unsigned char *)malloc(mtd->writesize + mtd->oobsize  + 64);
    if(!buf){
        printf("%s:malloc error...\n",__func__);
        return -1;
    }

    start = (p->part_offset)&~(mtd->erasesize-1);
    end = ((p->part_offset + p->part_size_real)&~(mtd->erasesize-1));
    erase.mtd = mtd;
    erase.callback=0;
    erase.priv = 0;
    if(priv->flags & MTD_FLAGS_RAW)
        printf("\nERASE the device:\"%s\",DON'T skip bad-blocks\n\n",path);
    else 
        printf("\nERASE the device:\"%s\",SKIP bad-blocks\n\n",path);

    if(priv->flags & MTD_FLAGS_RAW){
        struct nand_chip *chip=mtd->priv;
         while(start<end){
            chip->erase_cmd(mtd,start >> chip->page_shift);
            printf("\b\b\b\b\b\b\b\b\b\b%08x  ",start);
	    if(checkbb)
		    foundbb |= mtd_check_fullblock(mtd, start, 1, 1);
            start += mtd->erasesize;
        }
	 if(checkbb && foundbb)
		 creat_part_trans_table(p);
    }else{
        while(start<end){
            erase.addr = start;
            erase.len = mtd->erasesize;
            mtd->erase(mtd,&erase);
            printf("\b\b\b\b\b\b\b\b\b\b%08x ",start);
            if(jffs2){
                p->mtd->write(p->mtd,start,0xc,&retlen,clearmark);
               if(retlen != retlen)
                  break; 
            }
            start += mtd->erasesize;
        
        }
    }
    close(fp);    
    printf("\nmtd_erase work done!\n");
    free(buf);
    return 0;
}

extern struct cmdline_mtd_partition *partitions;
static int num_partitions;
static struct list_head mtd_list = LIST_HEAD_INIT(mtd_list);

int nand_flash_add_parts(struct mtd_info *mtd_soc,char *cmdline)
{

    int i;
    struct mtd_partition *partitions;
    if(!mtd_soc->siblings.next)
    {
	INIT_LIST_HEAD(&mtd_soc->siblings);
	list_add_tail(&mtd_soc->siblings, &mtd_list);
    }
	
    num_partitions = parse_cmdline_partitions(mtd_soc,&partitions,0,cmdline);
    if(num_partitions > 0){ 
        for(i=0;i < num_partitions;i++){ 
            add_mtd_device(mtd_soc,partitions[i].offset,partitions[i].size,partitions[i].name);
        }
    }
    return num_partitions;
}

void first_del_all_parts()
{
	struct mtdfile *rmp;

	LIST_FOREACH(rmp, &mtdfiles, i_next) {
	LIST_REMOVE(rmp, i_next);
	}
}

int mtd_rescan(char *name,char*value)
{
	struct list_head *p;
	struct mtd_info *mtd;
    if (value && !list_empty(&mtd_list)){
        first_del_all_parts(); 
	list_for_each(p, &mtd_list)
	{
	 mtd = list_entry(p, struct mtd_info , siblings);
         nand_flash_add_parts(mtd ,value);
	}
    }
    return 1;
}

int my_mtdparts(int argc,char**argv)
{
    struct cmdline_mtd_partition *part;
    struct mtd_partition *parts;
    int i;
    for (part = partitions; part; part = part->next)
    {
    printf("device <%s>,#parts = %d\n\n",part->mtd_id, part->num_parts);     
    printf(" #: name\tsize\t\toffset\t\tmask_flags\n");
	    for (i = 0, parts = part->parts; i < part->num_parts; i++){
		    printf(" %d: %s\t0x%08x(%dm)\t0x%08x(%dm)\t%x\n",i,parts[i].name,parts[i].size,parts[i].size>>20, parts[i].offset,parts[i].offset>>20,parts[i].mask_flags);
	    }
    }
    printf("\nmtdparts INFO:\n");
    printf("  now-active: \tmtdparts=%s\n",getenv("mtdparts"));
    printf("  <NOTE>:you can use command(CMD:  unset mtdparts ) to become the default !!! \n\n");
    return 0;
}


int mtd_update_ecc(char *file)
{
	uint8_t *buf, *p;
	mtdpriv *priv;
	mtdfile        *mf;
	struct mtd_info *mtd;
	struct nand_chip *chip;
	struct mtd_oob_ops ops;
	struct erase_info erase;
	int fd;
	unsigned int pos, len, once;
	int wecc;
        unsigned long long checked;
	fd = open(file, O_RDWR);
	priv = (mtdpriv *)_file[fd].data;
	mf = priv->file;
	mtd = mf->mtd;
	chip=mtd->priv;
	p = buf = malloc((mtd->writesize+mtd->oobsize)*mtd->erasesize/mtd->writesize);

        pos=mf->part_offset+priv->open_offset;
        len = priv->open_size_real;
        checked = 0;
	len +=  pos&(mtd->erasesize -1);
	pos -= pos&(mtd->erasesize -1);
	if(len&(mtd->erasesize -1))
		len += mtd->erasesize - (len&(mtd->erasesize -1));

	while(len>0)
	{
		int once;
		/*check a erasesize*/
		if((checked & 0xfffff) == 0) printf("0x%llx\n", checked);
		wecc  = 0;
		p = buf;
		for(once=0;once<mtd->erasesize;once += mtd->writesize)
		{
			ops.mode = MTD_OOB_RAW;
			ops.ooblen = mtd->oobsize;
			ops.ooboffs = 0;
			ops.oobbuf = p + mtd->writesize;
			ops.datbuf = p; 
			ops.len = mtd->writesize;
			mtd->read_oob(mtd,pos,&ops);
			if(ops.retlen<=0)
			{
				printf("ops.retlen=%d\n", ops.retlen);
				break;
			}

			if(once == 0 && (ops.oobbuf[0] != 0xff || ops.oobbuf[1] != 0xff))
			{
			  pos += mtd->erasesize;
			  wecc  = 0;
			  break;
			}

			{
				int i, eccsize = chip->ecc.size;
				int eccbytes = chip->ecc.bytes;
				int eccsteps = chip->ecc.steps;
				uint8_t *ecc_calc = chip->buffers.ecccalc;
				const uint8_t *p1 = p;
				int *eccpos = chip->ecc.layout->eccpos;

				/* Software ecc calculation */
				for (i = 0; eccsteps; eccsteps--, i += eccbytes, p1 += eccsize)
					chip->ecc.calculate(mtd, p1, &ecc_calc[i]);

				for (i = 0; i < chip->ecc.total; i++)
				{
					if(ops.oobbuf[eccpos[i]] != ecc_calc[i])
					{
						ops.oobbuf[eccpos[i]] = ecc_calc[i];
						wecc = 1;
					}
				}
			}
		       p += mtd->writesize + mtd->oobsize;
		       pos += mtd->writesize;
		}

		if(wecc)
		{
			pos -= mtd->erasesize;
			p = buf;
			erase.mtd = mtd;
			erase.callback = 0;
			erase.addr = pos;
			erase.len = mtd->erasesize;
			erase.priv = 0;

			mtd->erase(mtd,&erase);

			for(once=0;once<mtd->erasesize;once += mtd->writesize)
			{
				ops.mode = MTD_OOB_RAW;
				ops.ooblen = mtd->oobsize;
				ops.ooboffs = 0;
				ops.oobbuf = p + mtd->writesize;
				ops.datbuf = p; 
				ops.len = mtd->writesize;
				mtd->write_oob(mtd,pos,&ops);
				pos += mtd->writesize;
				p += mtd->writesize + mtd->oobsize;
			}
		}

		len -= mtd->erasesize;
                checked += mtd->erasesize;

	}
	if((checked & 0xfffff) == 0) printf("0x%llx\n", checked);

	free(buf);
	return 0;
}


static int mtd_read_fullblock(struct mtd_info *mtd, unsigned int pos, uint8_t *buf, int raw)
{
int once;
struct mtd_oob_ops ops;
uint8_t *p = buf;
		for(once=0;once<mtd->erasesize;once += mtd->writesize)
		{
			ops.mode = MTD_OOB_RAW;
			ops.ooblen = mtd->oobsize;
			ops.ooboffs = 0;
			ops.oobbuf = p + mtd->writesize;
			ops.datbuf = p; 
			ops.len = mtd->writesize;
			mtd->read_oob(mtd,pos,&ops);
			if(ops.retlen<=0)
			{
				printf("ops.retlen=%d\n", ops.retlen);
				break;
			}

			if(!raw && once == 0 && (ops.oobbuf[0] != 0xff || ops.oobbuf[1] != 0xff))
				break;

		       p += mtd->writesize + mtd->oobsize;
		       pos += mtd->writesize;
		}
return once;
}


static int mtd_erase_fullblock(struct mtd_info *mtd, unsigned int pos, int raw)
{
	struct erase_info erase;
	struct nand_chip *chip;

	if(raw)
	{
		chip  = mtd->priv;
		chip->erase_cmd(mtd, pos >> chip->page_shift);
		return 0;
	}
	else
	{
		erase.mtd = mtd;
		erase.callback = 0;
		erase.addr = pos;
		erase.len = mtd->erasesize;
		erase.priv = 0;
		return mtd->erase(mtd,&erase);
	}
}


static int mtd_write_fullblock(struct mtd_info *mtd, unsigned int pos, uint8_t *buf)
{
int once;
struct mtd_oob_ops ops;
uint8_t *p = buf;
			for(once=0;once<mtd->erasesize;once += mtd->writesize)
			{
				ops.mode = MTD_OOB_RAW;
				ops.ooblen = mtd->oobsize;
				ops.ooboffs = 0;
				ops.oobbuf = p + mtd->writesize;
				ops.datbuf = p; 
				ops.len = mtd->writesize;
				mtd->write_oob(mtd,pos,&ops);
				pos += mtd->writesize;
				p += mtd->writesize + mtd->oobsize;
			}
return once;
}

int mtd_check_fullblock(struct mtd_info *mtd,int pos,int eraseit, int raw)
{
	int foundbb, badbb;
	int buflen;
	uint8_t *buf, *buf1;
        int i;

	buflen = (mtd->writesize+mtd->oobsize)*mtd->erasesize/mtd->writesize;
	buf = malloc(buflen*2);
        buf1 = buf + buflen;

	foundbb = 0;
	badbb = 0;
	if(!eraseit && mtd_read_fullblock(mtd, pos, buf, raw) != mtd->erasesize)
		badbb = 1;


	if(badbb)
	{
		printf("skip bad block 0x%x at 0x%x\n", pos/mtd->erasesize, pos); 
	}
	else
	{
		mtd_erase_fullblock(mtd, pos, raw);
		memset(buf1,0,buflen);
		if(mtd_read_fullblock(mtd, pos, buf1, 1) != mtd->erasesize)
			goto bb1;
		for(i=0;i<buflen;i=i+4)
			if(*(volatile int *)(buf1+i) != -1)
				goto bb1;
		memset(buf1,0,buflen);
		mtd_write_fullblock(mtd, pos, buf1);
		memset(buf1,-1,buflen);
		if(mtd_read_fullblock(mtd, pos, buf1, 1) != mtd->erasesize)
			goto bb1;
		for(i=0;i<buflen;i=i+4)
			if(*(volatile *)(buf1+i) != 0)
				goto bb1;
bb:
		mtd_erase_fullblock(mtd, pos, raw);
		if(!eraseit)
			mtd_write_fullblock(mtd, pos, buf);

		if(foundbb)
		{
			mtd->block_markbad(mtd, pos); 
			printf("\nNew bad block==%d addr==0x%08x\n",(pos / mtd->erasesize), pos);
		}
	}
	free(buf);
	return foundbb;
bb1:
	foundbb = 1;
	goto bb;
}

int mtd_find_bb(char *file, int eraseit, int raw)
{
	mtdpriv *priv;
	mtdfile        *mf;
	struct mtd_info *mtd;
	struct mtd_oob_ops ops;
	struct erase_info erase;
	int fd;
	unsigned int pos, len, buflen;
        unsigned long long checked;
	int foundbb;
	int i;
	fd = open(file, O_RDWR);
	priv = (mtdpriv *)_file[fd].data;
	mf = priv->file;
	mtd = mf->mtd;

        pos=mf->part_offset+priv->open_offset;
        len = priv->open_size_real;
        checked = 0;
	len +=  pos&(mtd->erasesize -1);
	pos -= pos&(mtd->erasesize -1);
	if(len&(mtd->erasesize -1))
		len += mtd->erasesize - (len&(mtd->erasesize -1));
        foundbb = 0;

	while(len>0)
	{
		/*check a erasesize*/
		if((checked & 0xfffff) == 0) printf("\r0x%llx", checked);
		foundbb |= mtd_check_fullblock(mtd, pos, eraseit, raw);

		pos += mtd->erasesize;
		len -= mtd->erasesize;
                checked += mtd->erasesize;
	}
	if((checked & 0xfffff) == 0) printf("\r0x%llx\n", checked);
	creat_part_trans_table(mf);

	return 0;
}

int cmd_mtd_updateecc(int argc,char **argv)
{
 char *file = argc>1? argv[1]: "/dev/mtd0r";
 mtd_update_ecc(file);
 return 0;
}

int cmd_mtd_findbb(int argc,char **argv)
{
	char c;
	char *file;
	int eraseit = 0, raw = 0;
	while((c = getopt(argc, argv, "er")) != EOF) {
		switch(c){    
			case 'e':
				eraseit = 1;
				break;
			case 'r':
				raw = 1;
				break;
		}
	}
	file = optind < argc? argv[optind]: "/dev/mtd0r";
	mtd_find_bb(file, eraseit, raw);
	return 0;
}

const Optdesc         mtd_opts[] = {
	{"-h", "HELP"},
	{"-H", "HELP"},
	{"-j", "format the device for jffs2,the clearmark-size is 12 Byte"},
        {"-c", "check for bad block"},
	{0}
};


static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"mtdparts","0",0,"NANDFlash OPS:mtdparts ",my_mtdparts,0,99,CMD_REPEAT},
	{"mtd_erase","[option] mtd_dev",mtd_opts,0,cmd_flash_erase,0,99,CMD_REPEAT},
	{"mtd_updateecc","mtd_dev",NULL,0,cmd_mtd_updateecc,0,99,CMD_REPEAT},
	{"mtd_findbb","[-e] [-r] [mtd_dev]","e: erase,r: rawerase","find badblock, keep data or erase",cmd_mtd_findbb,0,99,CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
