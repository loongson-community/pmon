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
#include <string.h>
#include "target/ls7a.h"
#include "pflash.h"
#include "target/bonito.h"
#include <machine/frame.h>
#include "target/eeprom.h"
#include <stdlib.h>
#include <ctype.h>


#define MAX_LEVEL	32		/* how deeply nested we will go */
#define SCRATCHPAD      1024            /* bytes of scratchpad memory */
#define CONFIG_CMD_FDT_MAX_DUMP 64

extern unsigned long long memorysize_total;
extern char *heaptop;
struct fdt_header *working_fdt;

extern void *memcpy(void *s1, const void *s2, size_t n);
int dtb_cksum(void *p, size_t s, int set);
void verify_dtb(void);
unsigned long long setup_dtb(int ac, char ** av);
int load_dtb(int argc,char **argv);
int erase_dtb(int ac, char ** av);
