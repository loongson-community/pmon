/************************************************************************

 Copyright (C)
 File name:     sata.c
 Author:  qianyl      Version:  ***      Date: ***
 Description:   
 Others:        
 Function List:
 
 Revision History:
 
 --------------------------------------------------------------------------
  Date              Author           Activity ID              Activity Headline
  2008-05-12    QianYuli        PMON00000001    Creat it
*************************************************************************/
#include "sata.h"


static int sata_match(struct device *, void *, void *);
static void sata_attach (struct device *, struct device *, void *);

sata_operation_t sata_op = {NULL, NULL, NULL, NULL, NULL};
struct cfattach sata_ca = {
    sizeof(struct device),sata_match,sata_attach,
};
struct cfdriver sata_cd = {
        NULL, "sata", DV_DISK, 
};

static int sata_match(struct device *parent,void *match,void *aux)
{
    //printf("sata_match:parent:%x  \n",(unsigned int)parent);
    return 1;
}

static void sata_attach(struct device * parent,struct device * self,void *aux)
{
    //printf("sata_attach:parent:%x self:%x\n",(unsigned int)parent,(unsigned int)self);
    return;
}

int sata_open(
    dev_t dev,
    int flag, int fmt,
    struct proc *p)
{
    return (sata_op.open?sata_op.open(dev, flag, fmt, p):-1);
}


int sata_close( dev_t dev,
        int flag, int fmt,
        struct proc *p)
{
    return (sata_op.close?sata_op.close(dev, flag, fmt, p):-1);
}

int sata_read(
    dev_t dev,
    struct uio *uio,
    int flags)
{
    return (sata_op.read?sata_op.read(dev, uio, flags):-1);
}

int sata_write(
    dev_t dev,
    struct uio *uio,
    int flags)
{
    return (sata_op.write?sata_op.write(dev, uio, flags):-1);
}


