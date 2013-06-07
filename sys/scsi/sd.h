#ifndef		_SDH_
#define		_SDH_

/*
 * Some poeration function for bdevsw[].
 */
int sdopen(dev_t, int, int, struct proc *);
int sdread(dev_t, struct uio *, int);
int sdwrite(dev_t, struct uio *, int);
int sdclose(dev_t, int, int, struct proc *);
int sdioctl(dev_t, u_long, caddr_t, int, struct proc *);
int sddump(dev_t, daddr64_t, caddr_t, size_t);
void sdstrategy(struct buf *);
daddr64_t sdsize(dev_t);

#endif
