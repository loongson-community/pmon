#include <stdio.h>
#include <termio.h>
#include <string.h>
#include <setjmp.h>
#include <sys/endian.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <file.h>
#include <mtdfile.h>
#include<pmon.h>
#include<asm.h>
#include<machine/types.h>
#include<linux/mtd/mtd.h>
#include<linux/mtd/nand.h>
#include<linux/mtd/partitions.h>
#include<sys/malloc.h>
#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif


#define MYDBG 
//printf("%s:%x\n",__func__,__LINE__);
int my_flash_erase(int argc,char **argv)
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
        printf("example:mtd_erase /dev/mtd0\n");
        return -1;
    }
    path = argv[1];
    if(!path)return -1;
    fp = open(path,O_RDWR|O_CREAT|O_TRUNC);
    if(!fp){printf("open file error!\n");return -1;}
    if(strncmp(_file[fp].fs->devname,"mtd",3)){
        printf("the DEV(\"%s\") isn't flash-dev\n",path);
        return -1;
    }
    priv = (mtdpriv*)_file[fp].data;
    p = priv->file;
    mtd = p->mtd;
    start = (p->part_offset)&~(mtd->erasesize-1);
    end = (p->part_offset + p->part_size)&~(mtd->erasesize-1);
    erase.mtd = mtd;
    erase.callback=0;
    erase.priv = 0;
    printf("mtd_erase working: \n%08x  ",start);
    while(start<end){
        erase.addr = start;
        erase.len = mtd->erasesize;
        mtd->erase(mtd,&erase);
        printf("\b\b\b\b\b\b\b\b\b\b%08x  ",start);
        start += mtd->erasesize;
    }
    
    printf("\nmtd_erase work done!\n");
    return 0;
}
static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"mtd_erase","val",0,"NANDFlash OPS:mtd_erase /dev/mtdx",my_flash_erase,0,99,CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

