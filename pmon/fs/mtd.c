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
#if 0
#define MTD_FLAGS_CHAR      0x1 /*main+oob,all data */
#define MTD_FLAGS_BLOCK     0x2 /*main;but no OOB */
#define MTD_FLAGS_RAW       0x4 /*raw mtd,no check bad part*/
#define MTD_FLAGS_CHAR_MARK 0x8 /*main+oob,but bad_mark auto*/
#define MTD_FLAGS_GOOD      0x10 /*use good part info*/
#endif
#define min(a,b) (((a)<(b))?(a):(b))

LIST_HEAD(mtdfiles, mtdfile) mtdfiles = LIST_HEAD_INITIALIZER(mtdfiles);
static int mtdidx=0;
static struct mtdfile *goodpart0=0;

static int mtdfile_open (int , const char *, int, int);
static int mtdfile_close (int);
static int mtdfile_read (int, void *, size_t);
static int mtdfile_write (int, const void *, size_t);
static off_t mtdfile_lseek (int, off_t, int);

int highmemcpy(long long dst,long long src,long long count);
void mycacheflush(long long addrin,unsigned int size,unsigned int rw);

#define addr_to_block(x) ((x)>>17)
#define TO_MIN(x,y,z)   min(min((0x20000 - ((z) & (0x1ffff))),(x)),(y))
static creat_part_trans_table(mtdfile *p)
{
    struct mtd_info *mtd=p->mtd;
    struct nand_chip *this = (struct nand_chip*)mtd->priv;
    int start = addr_to_block(p->part_offset & ~(mtd->erasesize -1));
    int end = addr_to_block((p->part_offset + p->part_size) & ~(mtd->erasesize-1));
    int tmp=0,len,good;
    int *table =NULL;
    len = (end-start+1)*sizeof(int); 
    table = (int*)malloc(len);
    for(tmp=0,good=start;tmp<(end-start);tmp++){
        for(;good<end;good++){
            if(!(this->bbt[good>>2] & (0x3<<((good%4 )<<1))))break;
        }
        if(good==end)break;
        table[tmp]=good++;
    }
    p->part_size = (tmp+1)<<17; 
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
            if(flags & MTD_FLAGS_RAW || flags & MTD_FLAGS_CHAR ||flags & MTD_FLAGS_CHAR_MARK){
                open_size=strtoul(psize,0,0);
                open_size -= ((open_size / 0x840)*0x40);
            }else
            open_size=strtoul(psize,0,0);
        }
        open_offset=strtoul(poffset,0,0);
    }

    p->refs++;
    priv=malloc(sizeof(struct mtdpriv));
    priv->flags = flags;
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
    int maxlen,newpos,retlen,left;
    struct mtd_oob_ops ops;
    priv = (mtdpriv *)_file[fd].data;
    p = priv->file;
    if(priv->flags & MTD_FLAGS_CHAR || priv->flags & MTD_FLAGS_RAW ||priv->flags & MTD_FLAGS_CHAR_MARK){
        n-=64;
    }
    left = n;
    if (_file[fd].posn + n > priv->open_size)
        n = priv->open_size - _file[fd].posn;
    if(priv->flags & MTD_FLAGS_CHAR_MARK ||priv->flags & MTD_FLAGS_CHAR|| priv->flags & MTD_FLAGS_RAW ){
        if(n<=0)
            return 0;
        ops.mode = MTD_OOB_RAW;
        ops.ooblen = 64;
        ops.ooboffs = 0;
        ops.oobbuf = buf + p->mtd->writesize;
        ops.datbuf = buf; 
        while(left)
        {
            newpos=file_to_mtd_pos(fd,&maxlen);
            ops.len = min(left,maxlen);
            ops.len = min(ops.len,0x800);
            p->mtd->read_oob(p->mtd,newpos,&ops);
            if(ops.retlen<=0)break;
            _file[fd].posn += ops.retlen;
            left -= ops.retlen;
        }
        return (n-left+64);
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
    priv = (mtdpriv *)_file[fd].data;
    p = priv->file;
    if(priv->flags & MTD_FLAGS_CHAR_MARK || priv->flags & MTD_FLAGS_CHAR|| priv->flags & MTD_FLAGS_RAW)
        n-=64;
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
            ops.len=min(maxlen,0x800);

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
            ops.ooblen = 64;
            ops.ooboffs = 0;
            ops.oobbuf = buf+0x800;
            ops.datbuf = buf; 
            p->mtd->write_oob(p->mtd,start_addr,&ops);
            if(ops.retlen<=0)break;
            _file[fd].posn += ops.retlen;
            buf += (ops.retlen);
            if(left>ops.retlen) left -=ops.retlen;
            else left=0;
            return (n-left+64);
        }
    }else{
        while(left)
        {
            start_addr=file_to_mtd_pos(fd,&maxlen);
            maxlen=min(left,maxlen);
            maxlen=(maxlen+p->mtd->writesize-1)&~(p->mtd->writesize-1);

            erase.mtd = p->mtd;
            erase.callback = 0;
            erase.addr = (start_addr+p->mtd->erasesize-1)&~(p->mtd->erasesize-1);
            erase.len = (maxlen+p->mtd->erasesize-1)&~(p->mtd->erasesize-1);
            erase.priv = 0;

            if(erase.addr>=start_addr && erase.addr<start_addr+maxlen)
            {
                p->mtd->erase(p->mtd,&erase);
            }

            p->mtd->write(p->mtd,start_addr,maxlen,&retlen,buf);

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
    else rmp->part_size=mtd->size - offset;
    if(name)strcpy(rmp->name,name);

    if(!mtdfiles.lh_first)
    {
        rmp->i_next.le_next=NULL;
        rmp->i_next.le_prev=&mtdfiles.lh_first;
        mtdfiles.lh_first=rmp;
    }
    else {
        LIST_INSERT_AFTER(mtdfiles.lh_first, rmp, i_next);
    }
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
        tmp = pos;
        return (((((int*)(p->trans_table))[tmp>>17])<<17)|(tmp&((0x1<<17) -1)));
    }
    
}

int del_mtd_device (struct mtd_info *mtd)
{
	struct mtdfile *rmp;

	LIST_FOREACH(rmp, &mtdfiles, i_next) {
		if(rmp->mtd==mtd) {
                        if(rmp->refs == 0) {
				LIST_REMOVE(rmp, i_next);
                                mtdidx--;
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
    char *path=0;
    int fp=-1,start=0,end=0;
    mtdfile *p;
    mtdpriv *priv;
    struct mtd_info *mtd;
    struct erase_info erase;
    if(argc!=2){
        printf("args error...\n");
        printf("uargs:mtd_erase /dev/mtdx\n");
        printf("example:mtd_erase /dev/mtd0\n\n");
        return -1;
    }
    path = argv[1];
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
    start = (p->part_offset)&~(mtd->erasesize-1);
    end = ((p->part_offset + p->part_size)&~(mtd->erasesize-1)) - mtd->erasesize;
    erase.mtd = mtd;
    erase.callback=0;
    erase.priv = 0;
    if(priv->flags & MTD_FLAGS_RAW)
        printf("\nERASE the device:\"%s\",DON'T skip bad-blocks\n\n",path);
    else 
        printf("\nERASE the device:\"%s\",SKIP bad-blocks\n\n",path);
//    printf("end==0x%08x,start=0x%08x\n",end,start);
    printf("mtd_erase working: \n%08x  ",start);
    if(priv->flags & MTD_FLAGS_RAW){
        struct nand_chip *chip=mtd->priv;
         while(start<end){
            chip->erase_cmd(mtd,start >> chip->page_shift);
            printf("\b\b\b\b\b\b\b\b\b\b%08x  ",start);
            start += mtd->erasesize;
        }
    }else{
        while(start<end){
            erase.addr = start;
            erase.len = mtd->erasesize;
            mtd->erase(mtd,&erase);
            printf("\b\b\b\b\b\b\b\b\b\b%08x  ",start);
            start += mtd->erasesize;
        }
    }
    close(fp);    
    printf("\nmtd_erase work done!\n");
    return 0;
}

static struct mtd_partition *partitions;
static int num_partitions;
static struct mtd_info *nand_flash_mtd_info;

int nand_flash_add_parts(struct mtd_info *mtd_soc,char *cmdline)
{

    int i;
    if(!nand_flash_mtd_info) 
        nand_flash_mtd_info = mtd_soc;
    num_partitions = parse_cmdline_partitions(mtd_soc,&partitions,0,cmdline);
    if(num_partitions > 0){ 
        for(i=0;i < num_partitions;i++){ 
            add_mtd_device(mtd_soc,partitions[i].offset,partitions[i].size,partitions[i].name);
        }
    }
    return num_partitions;
}

void first_del_all_parts(struct mtd_info *soc_mtd)
{
    int i=0;
    for(;i<num_partitions;i++){
        del_mtd_device(soc_mtd);
    }
}

int mtd_rescan(char *name,char*value)
{
    if(value && nand_flash_mtd_info){
        first_del_all_parts(nand_flash_mtd_info); 
        nand_flash_add_parts(nand_flash_mtd_info,value);
    }
    return 1;
}

int my_mtdparts(int argc,char**argv)
{
    struct mtd_partition *parts=partitions;
    int num=num_partitions,i;
    printf("device <%s>,#parts = %d\n\n",nand_flash_mtd_info->name,num);     
    printf(" #: name\tsize\t\toffset\t\tmask_flags\n");
    for(i=0;i<num;i++){
        printf(" %d: %s\t0x%08x(%dm)\t0x%08x(%dm)\t%x\n",i,parts[i].name,parts[i].size,parts[i].size>>20, parts[i].offset,parts[i].offset>>20,parts[i].mask_flags);
    }
    printf("\nmtdparts INFO:\n");
    printf("  now-active: \tmtdparts=%s\n",getenv("mtdparts"));
    printf("  <NOTE>:you can use command(CMD:  unset mtdparts ) to become the default !!! \n\n");
    return 0;
}



static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"mtdparts","0",0,"NANDFlash OPS:mtdparts ",my_mtdparts,0,99,CMD_REPEAT},
	{"mtd_erase","nand-flash-device",0,"NANDFlash OPS:mtd_erase /dev/mtdx",cmd_flash_erase,0,99,CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
