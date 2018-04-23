/*************************************************************************
normal:	|-----bfc00000---|	dtb:	|-----bfc00000---|
	|	pmon	 |     		|	pmon	 |
	|		 |      	|		 |
	|----bfcff000----|      	|----DTB_OFFS----|
	|		 |      	|	dtb	 |
	| 	env	 |      	|   (DTB_SIZE)	 |
	|(NVRAM_SECSIZE) |      	|		 |
	|----------------|      	|----bfcff000----|
	|     unused	 |      	|	env	 |
	|----bfcfffff----|      	|----bfcfffff----|
***************************************************************************/


#include <sys/linux/types.h>
#include <pmon.h>
#include <stdio.h>
#include "target/ls2k.h"
#include "pflash.h"
#include "target/bonito.h"

#define	DTB_SIZE 0x4000		/* 16K dtb size*/
#define	DTB_OFFS		(NVRAM_OFFS - DTB_SIZE)
#define	MAC_OFFS	160
#define	MAC_LENTH	12
#define MAX_PHY_NUM	1

int dtb_cksum(void *p, size_t s, int set);
void verify_dtb(void);
struct trapframe * setup_dtb(int ac, char ** av, void *ssp);
int load_dtb(int argc,char **argv);
void erase_dtb(void);
