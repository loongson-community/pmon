#include <stdio.h>
#include <string.h>
#include <linux/types.h>

#ifdef LOONGSON_3A2H
#define ADDR_ORIG 0x9000000004000000
#define ADDR_TARGET 0x9000000005000000

//#define u64 unsigned long long 
extern u64 __raw__writeq(u64 addr, u64 val);
extern u64 __raw__readq(u64 q);

static int memtest()
{
	int j, error, count = 2000000;
	u64 val, val_orig, val_target;
	printf("Memory Test Begin ..\n");	
	for(j=0;count>0;count--,j+=8)
        {
                val = __raw__readq(ADDR_ORIG+j);
                __raw__writeq(ADDR_TARGET+j,val);
        }
        for(j=0;count>0;count--,j+=8)
        {
                val_orig = __raw__readq(ADDR_ORIG+j);
                val_target = __raw__readq(ADDR_TARGET+j);
                if(val_orig == val_target){
                        continue;
                }
                else{
                        error = 1;
                        break;
                }
        }
        if(error == 1)
                printf("Memory Test Failure\n");
        else
                printf("Memory Test Success\n");
	printf("Memory Test End ..\n");
	return 0;
}
#else
static int memtest()
{
        char cmd[20];
        printf("memory test\n");
        strcpy(cmd,"mt");
        do_cmd(cmd);
        return 0;
}
#endif
