#include <stdio.h>
#include <termio.h>
#include <string.h>
#include <setjmp.h>
#include <sys/endian.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif

#include <machine/cpu.h>

#include <pmon.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>
#include <flash.h>
#include <time.h>

#include <file.h>

/* header files */
#include <linux/zlib.h>
#include <stdio.h>
#include <stdarg.h>
#define TEST_CPU 1
#define TEST_MEM 2
#define TEST_RTL0 4
////#define TEST_EM0 8
////#define TEST_EM1 16
#define TEST_FREQ 8  //test cpu and bus frequency

#define TEST_PCI 32
#define TEST_VIDEO 64
#define TEST_HD 128
#define TEST_KBD 256
#define TEST_SERIAL 512
//#define TEST_PPPORT 1024
//#define TEST_FLOPPY 2048

#define TEST_ALL  4096

#define INFO_Y  24
#define INFO_W  80
extern void delay1(int);
extern void (*__cprint)(int y, int x,int width,char color, const char *text);
static int pause()
{
char cmd[20];
struct termio tbuf;
printf("\n");
__cprint(INFO_Y,0,INFO_W,0x70,"press enter to continue");
ioctl (STDIN, SETNCNE, &tbuf);
gets(cmd);
ioctl (STDIN, TCSETAW, &tbuf);
__cprint(INFO_Y,0,INFO_W,0x7,0);
return 0;
}

static int printticks(int n,int d)
{
int i;
char c[4]={'\\','|','/','-'};
for(i=0;i<n;i++)
{
	printf("%c",c[i%4]);
	delay1(d);	
	printf("\b");
}
return 0;
}

#include "cpu.c"
#include "mem.c"
#include "serial.c"
#include "video.c"
#include "hd.c"
#include "kbd.c"
#include "pci.c"
#include "pp.c"
#include "fd.c"

#include "../setup.h"

struct setupMenu nextmenu={
0,10,3,
(struct setupMenuitem[])
{
{20,60,1,1,TYPE_CMD,"GPIO Setup",0,10},
{21,60,2,2,TYPE_CMD,"(1)uart1"},
{22,60,0,0,TYPE_CMD,"(2)uart2"},
{}
}
};
#if 1 //xuhua add 
  #include <netdb.h> 
  extern int net_ping;
#endif
static int cmd_test(int ac,char **av)
{
long tests;
int i;
int j;
char cmd[200];
int  freq;  
char    fs[10], *fp;
tests=strtoul(av[1],0,0);

for(i=0;i<31;i++)
{   
    int t[8] = {0};
	int k = 0;
	int n = 0;
	int m = 0;
	
	int s = 0;
	if(!(tests&(1<<i)))continue;
	switch(1<<i)
	{
		case TEST_CPU:
			cputest();
			break;
		case TEST_MEM:
			memtest();
			break;
		case TEST_SERIAL:
			serialtest();
			break;
#if 0
        case TEST_PPPORT:
			pptest();
			break;
#endif
		case TEST_RTL0:
//			strcpy(cmd,"ifconfig em0 remove;ifconfig em1 remove;ifconfig rtl0 remove;ifconfig rtl0 192.168.2.1;");
//			strcpy(cmd,"ifconfig rtl0 remove;ifconfig rtl0 192.168.2.1;");
			strcpy(cmd,"set ifconfig rtl0:192.168.2.17;");
			do_cmd(cmd);
			printf("Plese plug net wire into rtl0\n");
			#if 0
			pause();
			#endif
			strcpy(cmd,"ping -c 12 192.168.2.231");
			do_cmd(cmd);
		#if 0  //xuhua
			t[7] = net_ping; 	
			printf("bbbbbbb=%d\n",net_ping);
			m = cmd_ping("ping", 4,  (char **){"ping", "-c", "10", "192.168.2.231"});
			printf("aaaaaaaaaaaaa=%d\n",m);
		#endif
			
			break;
#if 0
       case TEST_EM0:
			strcpy(cmd,"ifconfig em0 remove;ifconfig em1 remove;ifconfig rtl0 remove;ifconfig em0 192.168.2.1");
			do_cmd(cmd);
			printf("Plese plug net wire into em0\n");
			pause();
			strcpy(cmd,"ping -c 3 192.168.2.231");
			do_cmd(cmd);
			break;
		case TEST_EM1:
			strcpy(cmd,"ifconfig em0 remove;ifconfig em1 remove;ifconfig rtl0 remove;ifconfig em1 192.168.2.1;");
			do_cmd(cmd);
			printf("Plese plug net wire into em1\n");
			pause();
			strcpy(cmd,"ping -c 3 192.168.2.231");
			do_cmd(cmd);
			break;
#endif
#if 1			        
		case TEST_FREQ:
			freq = tgt_pipefreq ();
			sprintf(fs, "%d", freq);

			fp = fs + strlen(fs) - 6;
			fp[3] = '\0';
			fp[2] = fp[1];
			fp[1] = fp[0];
			fp[0] = '.';
			printf (" %s MHz\n", fs);
       
			freq = tgt_cpufreq ();
            sprintf(fs, "%d", freq);
			
			fp = fs + strlen(fs) - 6;
			fp[3] = '\0';
			fp[2] = fp[1];
			fp[1] = fp[0];
			fp[0] = '.';
			printf ("Bus @ %s MHz\n", fs);
			break;
#endif		
		case TEST_VIDEO:
			videotest();
			break;
		case TEST_HD:
			hdtest();
			break;
		case TEST_KBD:
			kbdtest();
			break;			
		case TEST_PCI:
			pcitest();
			break; 
#if 0
        case TEST_FLOPPY:
			fdtest();
			break;
#endif

#if 1		
        case TEST_ALL:
		    printf("begin to test cpu float calculation !!\n");
		    j = cputest();
			if ( j != 0 ) {printf("test cpu failure !!\n"); t[k] = 1;k++;t[k] = 0;}
			else {t[k] = 0;k++;}
			printf("continue test memory!!\n");
	        j = memtest();
			if ( j !=0 ) {printf("test memory failure !!\n");t[k] = 2;k++;t[k] = 0;}
			else {t[k] = 0;k++;}
			printf("continue test serial !!\n");
			j = serialtest();
			if ( j !=0 ) {printf("test serial failure !!\n");t[k] = 512;k++;t[k] = 0;}
			else {t[k] = 0;k++;}
           //test rtl0 
           // strcpy(cmd,"ifconfig rtl0 remove;ifconfig rtl0 192.168.2.1;");
            strcpy(cmd,"set ifconfig rtl0:192.168.2.17;");
			do_cmd(cmd); 
			printf("Plese plug net wire into rtl0\n");
			//pause();
			strcpy(cmd,"ping -c 12 192.168.2.231");
			do_cmd(cmd);
		#if 0  //xuhua add
		//	if (net_ping == 4){
			t[7]=net_ping;     
		//	printf("cccccccccccccc=%d\n",net_ping);//} //xuhua
		//	else
		//	    t[7] = 0;
		//	m = cmd_ping("ping", 4,  (char **){"ping", "-c", "4", "192.168.2.231"});
		//	printf("aaaaaaaaaaaaa=%d\n",m);
		#endif
			//#############
			//*****test cpu frequency**********//
			printf("Test cpu frequency");
			freq = tgt_pipefreq ();
			sprintf(fs, "%d", freq);

			fp = fs + strlen(fs) - 6;
			fp[3] = '\0';
			fp[2] = fp[1];
			fp[1] = fp[0];
			fp[0] = '.';
			printf (" %s MHz\n", fs);

			freq = tgt_cpufreq ();
			sprintf(fs, "%d", freq);

			fp = fs + strlen(fs) - 6;
			fp[3] = '\0';
			fp[2] = fp[1];
			fp[1] = fp[0];
			fp[0] = '.';
			printf ("Bus @ %s MHz\n", fs);
			delay(2000000);
			//****************************//
            j = videotest();
			if ( j!= 0 ) {printf("test video failure !!\n");t[k] = 64;k++;t[k] = 0;}
			else {t[k] = 0;k++;}
			#if 1
			printf("continue test hardware !!");
			j = hdtest();
			if ( j != 0 ) {printf("test harddisk failure !!\n");t[k] = 128;k++;t[k] = 0;}
			else {t[k] = 0;k++;}
			#endif
#if !(defined(VGA_NOTEBOOK_V1) || defined(VGA_NOTEBOOK_V2))
			printf("test kbd\n");
			j = kbdtest();
			if ( j != 0 ) {printf("test kbd failure !!\n");t[k] = 256;k++;t[k] = 0;}
			else 
#endif
			{t[k] = 0;k++;}
			printf("continue test pci !!\n");
			j = pcitest();
			if ( j != 0 ) {printf("test pci failure !!\n");t[k] = 32;k++;t[k] = 0;}
			t[k] = net_ping;
            
			for (;n<8;n++)
			  {
			   switch(t[n])
                {
			 	 case 1:
				      printf("test cpu float calculation error!!\n");
					  break;
				 case 2:
				      printf("test memory error!!\n");
					  break;
				 case 4:
				      printf("test net error with ping!!\n");
					  break;
				 case 512:
				      printf("test serial error!!\n");
					  break;
				 case 64:
				      printf("test video error!!\n");
					  break;
				 case 128:
				      printf("test harddisk error!!\n");
					  break;
				 case 256:
				      printf("test kbd error!!\n");
					  break;
				 case 32:
				      printf("test pci error!!\n");
				      break;
				}
			  }
	#if 1		  
		   while (k > 0){
		           t[0] |= t[k];
				   k--;
			       }
		   if ( t[0] == 0 ) 
			  printf("\t Test without error found\n");
		      printf("\nTest Over!!\n");
    #endif
# endif
			break;
}
	     #if 0
	        pause();
		 #endif
}

return 0;
}

//-------------------------------------------------------------------------------------------
static const Cmd Cmds[] =
{
	{"MyCmds"},
	{"test","val",0,"hardware test",cmd_test,0,99,CMD_REPEAT},
	{"serial","val",0,"hardware test",cmd_serial,0,99,CMD_REPEAT},
	{0,0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}

void testok()
{
printf("*********  *******   *******  *********     ******  *   *\n");     
printf("   *      *       * *       *     *        *      * *  * \n"); 
printf("   *      *       * *             *        *      * ***  \n");
printf("   *      *********  *******      *        *      * *    \n");
printf("   *      *                 *     *        *      * ***  \n");
printf("   *      *       * *       *     *        *      * *  * \n");
printf("   *       *******   *******      *         ******  *   *\n");
}


  
