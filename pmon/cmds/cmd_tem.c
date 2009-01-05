/*Created By CLF for CPU tempreture control*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/endian.h>

#include <pmon.h>
#include <diskfs.h>
#include <file.h>
#include <pmon/loaders/loadfn.h>

#ifdef __mips__
#include <machine/cpu.h>
#endif

#define nr_printf printf
#define nr_gets gets
#define nr_strtol strtoul

unsigned int flag = 1;
#include "target/cs5536.h"
#include <stdlib.h>

#define  TEM_SMB_CTRL1   (volatile unsigned char *)(0xbfd0b410 | SMB_CTRL1)
#define  TEM_SMB_STS     (volatile unsigned char *)(0xbfd0b410 | SMB_STS)
#define  TEM_SMB_SDA     (volatile unsigned char *)(0xbfd0b410 | SMB_SDA)
//#define  NORMAL          0x00
//#define  ABNORMAL        0x04
static void sumbus_sleep(int ntime)
{
	int i,j=0;
	for(i=0; i<10000; i++)		
	{
		j=i;
		j+=i;
	}
}
static void smbus_wait(void)
{
	unsigned char tmp;
	unsigned int num;
	num = 1000;
	sumbus_sleep(1);
	tmp = ((*TEM_SMB_STS) & SMB_STS_SDAST);
	while(!tmp)
	{ 		
		sumbus_sleep(1);
		num--;
		if(num == 0)
		{
			flag = 0;
			break;
		}
		tmp = ((*TEM_SMB_STS) & SMB_STS_SDAST);
	}
}
unsigned char tem_read_byte(unsigned char slave_addr,unsigned char sub_addr)
{
	unsigned char tmp;
	unsigned char value =0x00;
	slave_addr &=0xfe;
	
	/*********** 	 start condition	***************/
	*TEM_SMB_CTRL1 = 0x00;    				//reset SMB_CTRL1
	tmp =*TEM_SMB_CTRL1;
	*TEM_SMB_CTRL1 = tmp | SMB_CTRL1_START; //CTRL1_START=1
	*TEM_SMB_STS=0x00;       				//reset  SMB_STS
	tmp = *TEM_SMB_STS;		
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);						//SMB_STS=0
	if(tmp)
		goto out;
    smbus_wait();
	//printf("flag= 0x %x\n",flag);
	if (!flag)				/*	flag =1		*/
	{
		//printf("start error,The smbus loses an arbitrarion\n");
		goto out;
	}

	/******************** send slave address	*******************/
	*TEM_SMB_SDA = slave_addr;						/***	0x90 	****/
	//printf("slave_addr = 0x%x\n",slave_addr);
	//printf("tmp= 0x%x\n",tmp);
	//printf("*TEM_SMB_SDA = 0x%x\n",*TEM_SMB_SDA);	
	tmp = *TEM_SMB_STS;
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);		//=0
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp!=  0) 
		goto out;
	smbus_wait();
	//printf("flag=0x%x\n",flag);
	if (!flag)
	{
		//printf("send slave address error\n");
		goto out;
	}	
	/************ 	acknowledge smbus		*****************/
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;
	smbus_wait();
	if (!flag)
		goto out;
	
	/*************	send sub_addr	******************/
	*TEM_SMB_SDA = sub_addr;
	//printf("sub_addr = 0x%x\n",*TEM_SMB_SDA);	
	tmp = *TEM_SMB_STS;;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp !=  0) 
	      goto out;
	smbus_wait();
	//printf("flag = 0x %x\n",flag);  /* flag =1*/
	if (!flag)
	{
		//printf("send sub_addr error\n");
		goto out;
	}
	
	/***************	 start condition again *****************/
	tmp = *TEM_SMB_CTRL1;
	tmp |= SMB_CTRL1_START;
	*TEM_SMB_CTRL1 =tmp;
	tmp = *TEM_SMB_STS;
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp !=  0) 
        goto out;
    smbus_wait();
	//printf("flag = 0x %x\n",flag);  /* flag =1*/
	if (!flag)
		goto out;
	
	/*****************	send salve address again *************/ 
	*TEM_SMB_SDA = slave_addr + 0x01;
	tmp = *TEM_SMB_STS;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp!=  0) 
        goto out;
	smbus_wait();
	if (!flag)
		goto out;
	        
	/****************	stop condition	******************/
    tmp = *TEM_SMB_CTRL1;
	tmp |= SMB_CTRL1_STOP;
	*TEM_SMB_CTRL1 =tmp;
	smbus_wait();
	if (!flag)
		goto out;
		
	/******************	read data 	***********************/ 
	value = *TEM_SMB_SDA;
	//printf("value = 0x%x\n",value);
    return value;
    
out :
     //printf("\n slave_addr : 0x%x, sub_addr : 0x%x, error \n",slave_addr, sub_addr);
   	 return 0;
}

unsigned int tem_read_word(unsigned char slave_addr,unsigned char sub_addr)
{
	unsigned char tmp;
	unsigned int value =0x00;
	unsigned int value_high = 0x00;
	unsigned int value_low = 0x00;
	slave_addr &=0xfe;
		
	/*********** 	start condition		*****************/
	*TEM_SMB_CTRL1 = 0x00;    				//reset SMB_CTRL1
	tmp =*TEM_SMB_CTRL1;
	*TEM_SMB_CTRL1 = tmp | SMB_CTRL1_START; //CTRL1_START=1
	*TEM_SMB_STS=0x00;       				//reset  SMB_STS
	tmp = *TEM_SMB_STS;		
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);	//SMB_STS=0
	if(	tmp !=  0)
		goto out;
    smbus_wait();
	//printf("flag= 0x %x\n",flag);
	if (!flag)				/*	flag =1		*/
	{
		//printf("start error,The smbus loses an arbitrarion\n");
		goto out;
	}
	
	/****************	 send slave address*****************/
	*TEM_SMB_SDA = slave_addr;						/***	0x90 	****/
	//printf("slave_addr = 0x%x\n",*TEM_SMB_SDA);
	tmp = *TEM_SMB_STS;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);			//=0
	if(	tmp!=  0) 
		goto out;
	smbus_wait();
	//printf("flag=0x%x\n",flag);
	if (!flag)
	{
		//printf("send slave address error\n");
		goto out;
	}	
	/***************** acknowledge smbus	****************/
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;
	smbus_wait();
	if (!flag)
		goto out;
	/****************** send sub_addr	********************/
	*TEM_SMB_SDA = sub_addr;
	//printf("sub_addr = 0x%x\n",*TEM_SMB_SDA);
	tmp = *TEM_SMB_STS;;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	if(	tmp !=  0) 
	      goto out;
	smbus_wait();
	if (!flag)
	{
		//printf("send sub_addr error\n");
		goto out;
	}
	
	/************** start condition again	***************/ 
	*TEM_SMB_CTRL1 = 0x00;    				//reset SMB_CTRL1
	tmp =*TEM_SMB_CTRL1;
	*TEM_SMB_CTRL1 = tmp | SMB_CTRL1_START; //CTRL1_START=1
	*TEM_SMB_STS=0x00;       				//reset  SMB_STS
	tmp = *TEM_SMB_STS;		
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);	//SMB_STS=0
	if(	tmp !=  0)
		goto out;
    smbus_wait();
	//printf("flag= 0x %x\n",flag);
	if (!flag)				/*	flag =1		*/
	{
		//printf("start error,The smbus loses an arbitrarion\n");
		goto out;
	}
	
	/***************	send salve address again	******************/ 
	*TEM_SMB_SDA = slave_addr + 0x01;
	tmp = *TEM_SMB_STS;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	if(	tmp!=  0) 
        goto out;
	smbus_wait();
	if (!flag)
		goto out;
	/***************** acknowledge smbus	****************/
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;
	smbus_wait();
	if (!flag)
		goto out;
	/*****************	read high data  **********************/
	value_high = *TEM_SMB_SDA;
	//printf("value_high = 0x%x\n",value_high);
	tmp = *TEM_SMB_STS;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	if(	tmp!=  0) 
	{
		//printf("read high data error\n");
        goto out;
	}	
	smbus_wait();
	if (!flag)
		goto out;
	/*************** stop condition	*********************/
	*TEM_SMB_CTRL1=0x00;
	tmp = *TEM_SMB_CTRL1;
	tmp |= SMB_CTRL1_STOP;
	*TEM_SMB_CTRL1 =tmp;
	//printf("*TEM_SMB_CTRL1 = 0x%x\n",*TEM_SMB_CTRL1);
	smbus_wait();
	//printf("flag2 = %d \n",flag);
	if (!flag)
	{
		//printf("can not read data again!\n");
		goto out;
	}
	/*****************	read data  **********************/
	value_low= *TEM_SMB_SDA;
	//printf("value_low= 0x %x\n",value_low);
	/*************		real data	************************/
	value = (value_high << 8)+value_low;
	//printf("value= 0x %x\n",value);
    return value;
    
out :
     //printf("\n slave_addr : 0x%x, sub_addr : 0x%x, error \n",slave_addr, sub_addr);
   	 return 0;
}

unsigned char tem_write_byte(unsigned char slave_addr,unsigned char sub_addr,unsigned char send_value)
{
	unsigned char tmp;
	slave_addr &=0xfe;
		
	/*********** 	 start condition	***************/
	*TEM_SMB_CTRL1 = 0x00;    				//reset SMB_CTRL1
	tmp =*TEM_SMB_CTRL1;
	*TEM_SMB_CTRL1 = tmp | SMB_CTRL1_START; //CTRL1_START=1
	*TEM_SMB_STS=0x00;       				//reset  SMB_STS
	tmp = *TEM_SMB_STS;		
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);						//SMB_STS=0
	if(	tmp !=  0)
		goto out;
    smbus_wait();
	//printf("flag= 0x %x\n",flag);
	if (!flag)				/*	flag =1		*/
	{
		//printf("start error,The smbus loses an arbitrarion\n");
		goto out;
	}

	/******************** send slave address	*******************/
	*TEM_SMB_SDA = slave_addr;						/***	0x90 	****/
	//printf("slave_addr = 0x%x\n",slave_addr);
	//printf("tmp= 0x%x\n",tmp);
	//printf("*TEM_SMB_SDA = 0x%x\n",*TEM_SMB_SDA);	
	tmp = *TEM_SMB_STS;
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);		//=0
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp!=  0) 
		goto out;
	smbus_wait();
	//printf("flag=0x%x\n",flag);
	if (!flag)
	{
		//printf("send slave address error\n");
		goto out;
	}	
	/************ 	acknowledge smbus		*****************/
	
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;
	smbus_wait();
	if (!flag)
		goto out;

	/*************	send sub_addr	******************/
	*TEM_SMB_SDA = sub_addr;
	//printf("sub_addr = 0x%x\n",*TEM_SMB_SDA);	
	tmp = *TEM_SMB_STS;;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp !=  0) 
	      goto out;
	smbus_wait();
	//printf("flag = 0x %x\n",flag);  /* flag =1*/
	if (!flag)
	{
		//printf("send sub_addr error\n");
		goto out;
	}
	
	/************ 	acknowledge smbus		*****************/
	
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;	
	smbus_wait();
	if (!flag)
		goto out;
	 /***************	 write data		***************/
	//printf("send_value = 0x%x\n",send_value);
	*TEM_SMB_SDA = send_value;
	//printf("*TEM_SMB_SDA = 0x%x\n",*TEM_SMB_SDA);
	tmp = *TEM_SMB_STS;;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	if(	tmp !=  0) 
	      goto out;
	//printf("write data part error\n");
	smbus_wait();
	if (!flag)
	{
		//printf("write data error\n");
		goto out;
	}
	/************ 	acknowledge smbus		*****************/
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;
	smbus_wait();
	if (!flag)
		goto out;       
	/****************	stop condition	******************/
  	tmp = *TEM_SMB_CTRL1;
	tmp |= SMB_CTRL1_STOP;
	*TEM_SMB_CTRL1 =tmp;
	return 1;	
out :
     //printf("\n slave_addr : 0x%x, sub_addr : 0x%x, write function error \n",slave_addr, sub_addr);
   	 return 0;
}
unsigned char tem_write_word(unsigned char slave_addr,unsigned char sub_addr,unsigned int send_value)
{
	unsigned char tmp;
	unsigned char value_high = 0x00;
	unsigned char value_low = 0x00;
	value_high = ((send_value & (0xff << 8)) >> 8);
	//printf("value_high = 0x %x\n",value_high);
	value_low = (unsigned char)send_value ;
	//printf("value_low = 0x%x\n",value_low);

	slave_addr &=0xfe;
	/*********** 	 start condition	***************/
	*TEM_SMB_CTRL1 = 0x00;    				//reset SMB_CTRL1
	tmp =*TEM_SMB_CTRL1;
	*TEM_SMB_CTRL1 = tmp | SMB_CTRL1_START; //CTRL1_START=1
	*TEM_SMB_STS=0x00;       				//reset  SMB_STS
	tmp = *TEM_SMB_STS;		
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);						//SMB_STS=0
	if(	tmp !=  0)
		goto out;
    smbus_wait();
	//printf("flag= 0x %x\n",flag);
	if (!flag)				/*	flag =1		*/
	{
		//printf("start error,The smbus loses an arbitrarion\n");
		goto out;
	}

	/******************** send slave address	*******************/
	*TEM_SMB_SDA = slave_addr;						/***	0x90 	****/
	//printf("slave_addr = 0x%x\n",slave_addr);
	//printf("tmp= 0x%x\n",tmp);
	//printf("*TEM_SMB_SDA = 0x%x\n",*TEM_SMB_SDA);	
	tmp = *TEM_SMB_STS;
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);		//=0
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp!=  0) 
		goto out;
	smbus_wait();
	//printf("flag=0x%x\n",flag);
	if (!flag)
	{
		//printf("send slave address error\n");
		goto out;
	}	
	/************ 	acknowledge smbus		*****************/
	
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;
	smbus_wait();
	if (!flag)
		goto out;

	/*************	send sub_addr	******************/
	*TEM_SMB_SDA = sub_addr;
	//printf("sub_addr = 0x%x\n",*TEM_SMB_SDA);	
	tmp = *TEM_SMB_STS;;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp !=  0) 
	      goto out;
	smbus_wait();
	//printf("flag = 0x %x\n",flag);  /* flag =1*/
	if (!flag)
	{
		//printf("send sub_addr error\n");
		goto out;
	}
	
	/************ 	acknowledge smbus		*****************/
	
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;	
	smbus_wait();
	if (!flag)
		goto out;
	/***************	 write high data		***************/
	//printf("send_value = 0x%x\n",send_value);
	*TEM_SMB_SDA = value_high;
	//printf("*TEM_SMB_SDA = 0x%x\n",*TEM_SMB_SDA);
	tmp = *TEM_SMB_STS;;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	if(	tmp !=  0) 
	      goto out;
	//printf("write data part error\n");
	smbus_wait();
	if (!flag)
	{
		//printf("write value_high error\n");
		goto out;
	}
	
	/************ 	acknowledge smbus		*****************/
	
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;	
	smbus_wait();
	if (!flag)
		goto out;
	
	/***************	 write low data		***************/
	//printf("send_value = 0x%x\n",send_value);
	*TEM_SMB_SDA = value_low;
	//printf("*TEM_SMB_SDA = 0x%x\n",*TEM_SMB_SDA);
	tmp = *TEM_SMB_STS;;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	if(	tmp !=  0) 
	      goto out;
	//printf("write data part error\n");
	smbus_wait();
	if (!flag)
	{
		//printf("write value_low error\n");
		goto out;
	}
	/************ 	acknowledge smbus		*****************/
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;
	smbus_wait();
	if (!flag)
		goto out;       
	/****************	stop condition	******************/
  	tmp = *TEM_SMB_CTRL1;
	tmp |= SMB_CTRL1_STOP;
	*TEM_SMB_CTRL1 =tmp;
	return 1;	
out :
     //printf("\n slave_addr : 0x%x, sub_addr : 0x%x, write function error \n",slave_addr, sub_addr);
   	 return 0;
}

unsigned char read_byte(int argc,unsigned char *argv[])
{
	unsigned char tmp;
	unsigned char value =0x00;
	//unsigned char slave_addr = argv[1] ;
	//unsigned char sub_addr = argv [2];
	unsigned char slave_addr = strtoul(argv[1], 0, 16);
	unsigned char sub_addr = strtoul(argv[2], 0, 16);
	//printf("slave_addr 0x%x, index_addr 0x%x\n", slave_addr, sub_addr);
	slave_addr &=0xfe;
	
	/*********** 	 start condition	***************/
	*TEM_SMB_CTRL1 = 0x00;    				//reset SMB_CTRL1
	tmp =*TEM_SMB_CTRL1;
	*TEM_SMB_CTRL1 = tmp | SMB_CTRL1_START; //CTRL1_START=1
	*TEM_SMB_STS=0x00;       				//reset  SMB_STS
	tmp = *TEM_SMB_STS;		
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);						//SMB_STS=0
	if(tmp)
		goto out;
    smbus_wait();
	//printf("flag= 0x %x\n",flag);
	if (!flag)				/*	flag =1		*/
	{
		//printf("start error,The smbus loses an arbitrarion\n");
		goto out;
	}

	/******************** send slave address	*******************/
	*TEM_SMB_SDA = slave_addr;						/***	0x90 	****/
	//printf("slave_addr = 0x%x\n",slave_addr);
	//printf("tmp= 0x%x\n",tmp);
	//printf("*TEM_SMB_SDA = 0x%x\n",*TEM_SMB_SDA);	
	tmp = *TEM_SMB_STS;
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);		//=0
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp!=  0) 
		goto out;
	smbus_wait();
	//printf("flag=0x%x\n",flag);
	if (!flag)
	{
		//printf("send slave address error\n");
		goto out;
	}	
	/************ 	acknowledge smbus		*****************/
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;
	smbus_wait();
	if (!flag)
		goto out;
	
	/*************	send sub_addr	******************/
	*TEM_SMB_SDA = sub_addr;
	//printf("sub_addr = 0x%x\n",*TEM_SMB_SDA);	
	tmp = *TEM_SMB_STS;;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp !=  0) 
	      goto out;
	smbus_wait();
	//printf("flag = 0x %x\n",flag);  /* flag =1*/
	if (!flag)
	{
		//printf("send sub_addr error\n");
		goto out;
	}
	
	/***************	 start condition again *****************/
	tmp = *TEM_SMB_CTRL1;
	tmp |= SMB_CTRL1_START;
	*TEM_SMB_CTRL1 =tmp;
	tmp = *TEM_SMB_STS;
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp !=  0) 
        goto out;
    smbus_wait();
	//printf("flag = 0x %x\n",flag);  /* flag =1*/
	if (!flag)
		goto out;
	
	/*****************	send salve address again *************/ 
	*TEM_SMB_SDA = slave_addr + 0x01;
	tmp = *TEM_SMB_STS;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp!=  0) 
        goto out;
	smbus_wait();
	if (!flag)
		goto out;
	        
	/****************	stop condition	******************/
    tmp = *TEM_SMB_CTRL1;
	tmp |= SMB_CTRL1_STOP;
	*TEM_SMB_CTRL1 =tmp;
	smbus_wait();
	if (!flag)
		goto out;
		
	/******************	read data 	***********************/ 
	value = *TEM_SMB_SDA;
	//printf("value = 0x%x\n",value);
    return value;
    
out :
     //printf("\n slave_addr : 0x%x, sub_addr : 0x%x, error \n",slave_addr, sub_addr);
   	 return 0;
}

unsigned int read_word(int argc,unsigned char *argv[])
{
	unsigned char tmp;
	unsigned int value =0x00;
	//unsigned char slave_addr = argv[1];
	//unsigned char sub_addr = argv[2];
	unsigned char slave_addr = strtoul(argv[1], 0, 16);
	unsigned char sub_addr = strtoul(argv[2], 0, 16);
	unsigned int value_high = 0x00;
	unsigned int value_low = 0x00;
	slave_addr &=0xfe;
	//printf("slave_addr 0x%x, index_addr 0x%x\n", slave_addr, sub_addr);
	
	/*********** 	start condition		*****************/
	*TEM_SMB_CTRL1 = 0x00;    				//reset SMB_CTRL1
	tmp =*TEM_SMB_CTRL1;
	*TEM_SMB_CTRL1 = tmp | SMB_CTRL1_START; //CTRL1_START=1
	*TEM_SMB_STS=0x00;       				//reset  SMB_STS
	tmp = *TEM_SMB_STS;		
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);	//SMB_STS=0
	if(	tmp !=  0)
		goto out;
    smbus_wait();
	//printf("flag= 0x %x\n",flag);
	if (!flag)				/*	flag =1		*/
	{
		//printf("start error,The smbus loses an arbitrarion\n");
		goto out;
	}
	
	/****************	 send slave address*****************/
	*TEM_SMB_SDA = slave_addr;						/***	0x90 	****/
	//printf("slave_addr = 0x%x\n",*TEM_SMB_SDA);
	tmp = *TEM_SMB_STS;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);			//=0
	if(	tmp!=  0) 
		goto out;
	smbus_wait();
	//printf("flag=0x%x\n",flag);
	if (!flag)
	{
		//printf("send slave address error\n");
		goto out;
	}	
	/***************** acknowledge smbus	****************/
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;
	smbus_wait();
	if (!flag)
		goto out;
	/****************** send sub_addr	********************/
	*TEM_SMB_SDA = sub_addr;
	//printf("sub_addr = 0x%x\n",*TEM_SMB_SDA);
	tmp = *TEM_SMB_STS;;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	if(	tmp !=  0) 
	      goto out;
	smbus_wait();
	if (!flag)
	{
		//printf("send sub_addr error\n");
		goto out;
	}
	
	/************** start condition again	***************/ 
	*TEM_SMB_CTRL1 = 0x00;    				//reset SMB_CTRL1
	tmp =*TEM_SMB_CTRL1;
	*TEM_SMB_CTRL1 = tmp | SMB_CTRL1_START; //CTRL1_START=1
	*TEM_SMB_STS=0x00;       				//reset  SMB_STS
	tmp = *TEM_SMB_STS;		
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);	//SMB_STS=0
	if(	tmp !=  0)
		goto out;
    smbus_wait();
	//printf("flag= 0x %x\n",flag);
	if (!flag)				/*	flag =1		*/
	{
		//printf("start error,The smbus loses an arbitrarion\n");
		goto out;
	}
	
	/***************	send salve address again	******************/ 
	*TEM_SMB_SDA = slave_addr + 0x01;
	tmp = *TEM_SMB_STS;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	if(	tmp!=  0) 
        goto out;
	smbus_wait();
	if (!flag)
		goto out;
	/***************** acknowledge smbus	****************/
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;
	smbus_wait();
	if (!flag)
		goto out;
	/*****************	read high data  **********************/
	value_high = *TEM_SMB_SDA;
	//printf("value_high = 0x%x\n",value_high);
	tmp = *TEM_SMB_STS;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	if(	tmp!=  0) 
	{
		//printf("read high data error\n");
        goto out;
	}	
	smbus_wait();
	if (!flag)
		goto out;
	/*************** stop condition	*********************/
	*TEM_SMB_CTRL1=0x00;
	tmp = *TEM_SMB_CTRL1;
	tmp |= SMB_CTRL1_STOP;
	*TEM_SMB_CTRL1 =tmp;
	//printf("*TEM_SMB_CTRL1 = 0x%x\n",*TEM_SMB_CTRL1);
	smbus_wait();
	//printf("flag2 = %d \n",flag);
	if (!flag)
	{
		//printf("can not read data again!\n");
		goto out;
	}
	/*****************	read data  **********************/
	value_low= *TEM_SMB_SDA;
	//printf("value_low= 0x %x\n",value_low);
	/*************		real data	************************/
	value = (value_high << 8)+value_low;
	//printf("value= 0x %x\n",value);
    return value;
    
out :
     //printf("\n slave_addr : 0x%x, sub_addr : 0x%x, error \n",slave_addr, sub_addr);
   	 return 0;
}

unsigned char write_byte(int argc, char *argv[])
{
	unsigned char tmp;
	unsigned char slave_addr = strtoul(argv[1], 0, 16);
	unsigned char sub_addr = strtoul(argv[2], 0, 16);
	unsigned char send_value = strtoul(argv[3], 0, 16);
	//printf("slave_addr 0x%x, index_addr 0x%x,send_value 0x %x \n", slave_addr, sub_addr,send_value);
	slave_addr &=0xfe;
		
	/*********** 	 start condition	***************/
	*TEM_SMB_CTRL1 = 0x00;    				//reset SMB_CTRL1
	tmp =*TEM_SMB_CTRL1;
	*TEM_SMB_CTRL1 = tmp | SMB_CTRL1_START; //CTRL1_START=1
	*TEM_SMB_STS=0x00;       				//reset  SMB_STS
	tmp = *TEM_SMB_STS;		
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);						//SMB_STS=0
	if(	tmp !=  0)
		goto out;
    smbus_wait();
	//printf("flag= 0x %x\n",flag);
	if (!flag)				/*	flag =1		*/
	{
		//printf("start error,The smbus loses an arbitrarion\n");
		goto out;
	}

	/******************** send slave address	*******************/
	*TEM_SMB_SDA = slave_addr;						/***	0x90 	****/
	//printf("slave_addr = 0x%x\n",slave_addr);
	//printf("tmp= 0x%x\n",tmp);
	//printf("*TEM_SMB_SDA = 0x%x\n",*TEM_SMB_SDA);	
	tmp = *TEM_SMB_STS;
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);		//=0
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp!=  0) 
		goto out;
	smbus_wait();
	//printf("flag=0x%x\n",flag);
	if (!flag)
	{
		//printf("send slave address error\n");
		goto out;
	}	
	/************ 	acknowledge smbus		*****************/
	
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;
	smbus_wait();
	if (!flag)
		goto out;

	/*************	send sub_addr	******************/
	*TEM_SMB_SDA = sub_addr;
	//printf("sub_addr = 0x%x\n",*TEM_SMB_SDA);	
	tmp = *TEM_SMB_STS;;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp !=  0) 
	      goto out;
	smbus_wait();
	//printf("flag = 0x %x\n",flag);  /* flag =1*/
	if (!flag)
	{
		//printf("send sub_addr error\n");
		goto out;
	}
	
	/************ 	acknowledge smbus		*****************/
	
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;	
	smbus_wait();
	if (!flag)
		goto out;
	 /***************	 write data		***************/
	//printf("send_value = 0x%x\n",send_value);
	*TEM_SMB_SDA = send_value;
	//printf("*TEM_SMB_SDA = 0x%x\n",*TEM_SMB_SDA);
	tmp = *TEM_SMB_STS;;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	if(	tmp !=  0) 
	      goto out;
	//printf("write data part error\n");
	smbus_wait();
	if (!flag)
	{
		//printf("write data error\n");
		goto out;
	}
	/************ 	acknowledge smbus		*****************/
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;
	smbus_wait();
	if (!flag)
		goto out;       
	/****************	stop condition	******************/
  	tmp = *TEM_SMB_CTRL1;
	tmp |= SMB_CTRL1_STOP;
	*TEM_SMB_CTRL1 =tmp;
	/*smbus_wait();
	if (!flag)
	{
		printf("stop condition error\n");
		goto out;
	}*/
	return 1;	
out :
     //printf("\n slave_addr : 0x%x, sub_addr : 0x%x, write function error \n",slave_addr, sub_addr);
   	 return 0;
}


unsigned char write_word(int argc, unsigned int *argv[])
{
	unsigned char tmp;
	unsigned char value_high = 0x00;
	unsigned char value_low = 0x00;
	unsigned char slave_addr =(unsigned char) strtoul(argv[1], 0, 16);
	unsigned char sub_addr = (unsigned char)strtoul(argv[2], 0, 16);
	unsigned int send_value = strtoul(argv[3], 0, 16);
	//printf("slave_addr 0x%x, index_addr 0x%x  send_value 0x %x\n", slave_addr, sub_addr,send_value);
	value_high = ((send_value & (0xff << 8)) >> 8);
	//printf("value_high = 0x %x\n",value_high);
	value_low = (unsigned char)send_value ;
	//printf("value_low = 0x%x\n",value_low);

	slave_addr &=0xfe;
	/*********** 	 start condition	***************/
	*TEM_SMB_CTRL1 = 0x00;    				//reset SMB_CTRL1
	tmp =*TEM_SMB_CTRL1;
	*TEM_SMB_CTRL1 = tmp | SMB_CTRL1_START; //CTRL1_START=1
	*TEM_SMB_STS=0x00;       				//reset  SMB_STS
	tmp = *TEM_SMB_STS;		
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);						//SMB_STS=0
	if(	tmp !=  0)
		goto out;
    smbus_wait();
	//printf("flag= 0x %x\n",flag);
	if (!flag)				/*	flag =1		*/
	{
		//printf("start error,The smbus loses an arbitrarion\n");
		goto out;
	}

	/******************** send slave address	*******************/
	*TEM_SMB_SDA = slave_addr;						/***	0x90 	****/
	//printf("slave_addr = 0x%x\n",slave_addr);
	//printf("tmp= 0x%x\n",tmp);
	//printf("*TEM_SMB_SDA = 0x%x\n",*TEM_SMB_SDA);	
	tmp = *TEM_SMB_STS;
	tmp &=(SMB_STS_BER | SMB_STS_NEGACK);		//=0
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp!=  0) 
		goto out;
	smbus_wait();
	//printf("flag=0x%x\n",flag);
	if (!flag)
	{
		//printf("send slave address error\n");
		goto out;
	}	
	/************ 	acknowledge smbus		*****************/
	
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;
	smbus_wait();
	if (!flag)
		goto out;

	/*************	send sub_addr	******************/
	*TEM_SMB_SDA = sub_addr;
	//printf("sub_addr = 0x%x\n",*TEM_SMB_SDA);	
	tmp = *TEM_SMB_STS;;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	//printf("tmp = 0x%x\n",tmp);
	if(	tmp !=  0) 
	      goto out;
	smbus_wait();
	//printf("flag = 0x %x\n",flag);  /* flag =1*/
	if (!flag)
	{
		//printf("send sub_addr error\n");
		goto out;
	}
	
	/************ 	acknowledge smbus		*****************/
	
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;	
	smbus_wait();
	if (!flag)
		goto out;
	/***************	 write high data		***************/
	//printf("send_value = 0x%x\n",send_value);
	*TEM_SMB_SDA = value_high;
	//printf("*TEM_SMB_SDA = 0x%x\n",*TEM_SMB_SDA);
	tmp = *TEM_SMB_STS;;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	if(	tmp !=  0) 
	      goto out;
	//printf("write data part error\n");
	smbus_wait();
	if (!flag)
	{
		//printf("write value_high error\n");
		goto out;
	}
	
	/************ 	acknowledge smbus		*****************/
	
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;	
	smbus_wait();
	if (!flag)
		goto out;
	
	/***************	 write low data		***************/
	//printf("send_value = 0x%x\n",send_value);
	*TEM_SMB_SDA = value_low;
	//printf("*TEM_SMB_SDA = 0x%x\n",*TEM_SMB_SDA);
	tmp = *TEM_SMB_STS;;
	tmp &= (SMB_STS_BER | SMB_STS_NEGACK);
	if(	tmp !=  0) 
	      goto out;
	//printf("write data part error\n");
	smbus_wait();
	if (!flag)
	{
		//printf("write value_low error\n");
		goto out;
	}
	/************ 	acknowledge smbus		*****************/
	tmp = *TEM_SMB_CTRL1;
	tmp |= (SMB_CTRL1_ACK);
	*TEM_SMB_CTRL1 =tmp ;
	smbus_wait();
	if (!flag)
		goto out;       
	/****************	stop condition	******************/
  	tmp = *TEM_SMB_CTRL1;
	tmp |= SMB_CTRL1_STOP;
	*TEM_SMB_CTRL1 =tmp;
	return 1;	
out :
     //printf("\n slave_addr : 0x%x, sub_addr : 0x%x, write function error \n",slave_addr, sub_addr);
   	 return 0;
}

/*
* this function is used to reverse fan.
* if the fan running ,using this function ,the fan will stop.
* if the fan not running,using this function ,the fan will run.
*/
void fan_off(int argc, unsigned char *argv[])
{	
	tem_write_byte(0x90,0x01,0x02);
	//printf("the fan stop  now\n");
	
}
void fan_on(int argc, unsigned char *argv[])
{	
	tem_write_byte(0x90,0x01,0x06);
	//printf("the fan run now\n");
}
void reverse_fan(int argc, unsigned char *argv[])
{	
	unsigned char tmp;
	tmp = tem_read_byte(0x90,0x01);
	if (( tmp & (1 << 2)))
	{	
		tem_write_byte(0x90,0x01,0x00);
		//printf("config value is 0x00,operate in normal mode\n");
	}
	else
	{
		tem_write_byte(0x90,0x01,0x04);
		//printf("config value is 0x04,operate in abnormal mode\n");
	}
}
void read_TT (int argc,unsigned char *argv[])
{	
	char *ptr;
	ptr = strchr(argv[1],'-');
	if(ptr == NULL){
		//printf("read :not enough arguments");
		//printf("usage: read -lht");
		}
	else
		{
			ptr++;
			switch (*ptr) 
			{
			case 'l':
				printf("The low value is 0x %x\n",tem_read_word(0x90,0x02));
				break;
			case 'h':
	    		printf("The high value is 0x %x\n",tem_read_word(0x90,0x03));
	    		break;
			case 't':
	    		printf("The temperature value now is 0x %x\n",tem_read_word(0x90,0x00));
				break;
			default:
				printf("please enter the argument afer \- \n");
				break;
			}
		}
}

void write_TT (int argc,unsigned char *argv[])
{	
	char *ptr;
	unsigned int send_value;
	unsigned char tmp;
	ptr = strchr(argv[1],'-');
	if(ptr == NULL){
		printf("write :not enough arguments\n");
		printf("usage: write -lht 0xdata\n");
		}
	else
		{
			ptr++;
			switch (*ptr) 
			{
			case 'l':
				send_value = strtoul(argv[2], 0, 16);
				tem_write_word(0x90,0x02,send_value);
				printf("The low value  is 0x %x\n",send_value);
				break;
			case 'h':
	    		send_value = strtoul(argv[2], 0, 16);
				tem_write_word(0x90,0x03,send_value);
				printf("The high value  is 0x %x\n",send_value);
	    		break;
			case 'm':
	    		send_value = strtoul(argv[2], 0, 16);
				if (send_value)
					{  
						tem_write_byte(0x90,0x01,0x00);
	 					printf("The ADT75 operate in normal mode\n");
					}
				else
					{  
						tem_write_byte(0x90,0x01,0x04);
	 					printf("The ADT75 operate in abnormal mode\n");
					}		
	    		break;
				
			case 'r':
				tmp = tem_read_byte(0x90,0x01);
				if (( tmp & (1 << 2)))
					{	
					tem_write_byte(0x90,0x01,0x00);
					printf("config value is 0x00,operate in normal mode\n");
					}
				else
					{
					tem_write_byte(0x90,0x01,0x04);
					printf("config value is 0x04,operate in abnormal mode\n");
					}
				break;
			default:
				printf(" argument is error \n  \n");
				break;
			}
		}
}

/*
void read_T (int argc,unsigned char *argv[])
{
	int c, err;
	extern int optind;
	extern char *optarg;
	int flags;
	unsigned long offset = 0;
	unsigned long entry = 0;
	err = 0;
	while ((c = getopt (argc, argv, "lht")) != EOF) {
		switch (c) {
			case 'l':
				flags |= 1;printf("flags |= 1\n"); break;
			case 'h':
				flags |= 2;printf("flags |= 2\n"); break;
			case 't':
				flags |= 4;printf("flags |= 4\n"); break;
			case 'r':
				flags |= 8;printf("flags |= 8\n"); break;
			case 'e':
				if (!get_rsa ((u_int32_t *)&entry, optarg)) {
					err++;
				}
				break;
			case 'o':
				if (!get_rsa ((u_int32_t *)&offset, optarg)) {
					err++;
				}
				break;
			default:
				err++;
				break;
		}
	}
	if (err) 
		return 1;
}*/
void set_low(int argc, unsigned char *argv[])
{	
	unsigned int send_value = strtoul(argv[1], 0, 16);
	tem_write_word(0x90,0x02,send_value);
}
void set_high(int argc, unsigned char *argv[])
{	
	unsigned int send_value = strtoul(argv[1], 0, 16);
	tem_write_word(0x90,0x03,send_value);
}
void read_low(int argc, unsigned char *argv[])
{	
	
	printf("The low value is 0x %x\n",tem_read_word(0x90,0x02));
}
void read_high(int argc, unsigned char *argv[])
{	
	printf("The low value is 0x %x\n",tem_read_word(0x90,0x03));
}
void read_temperature (int argc, unsigned char *argv[])
{	
	printf("The temperature value now is 0x %x\n",tem_read_word(0x90,0x00));
}
void set_mode(int argc, unsigned char *argv[])
{	
	unsigned char send_value = strtoul(argv[1], 0, 16);
	
	if (send_value)
	{  
		tem_write_byte(0x90,0x01,0x00);
	 	printf("The ADT75 operate in normal mode\n");
	}
	else
	{  
		tem_write_byte(0x90,0x01,0x04);
	 	printf("The ADT75 operate in abnormal mode\n");
	}
}
/*
const Optdesc r_opts[] =
{
	{"*", "read  registers value"},
    {"-h", "read register 3,high temperature"},
    {"-l", "read register 2,low temperature"},
    {"-t", "read temperature value"},
    {0}
};
const Optdesc w_opts[] =
{
	{"*", "write value to registers "},
    {"-h", "write value to register 3"},
    {"-l", "write value to register 2"},
    {"-m", "write value to register 1"},
    {"-r", "reverse state of the fan "},
    {0}
};*/
static const Cmd Cmds[] =
{
	{"fan control"},
	{"write_word", "", 0, "write 16 bit registers", write_word, 0, 99, CMD_REPEAT},
	{"write_byte", "", 0, "write 8 bits registers", write_byte, 0, 99, CMD_REPEAT},
	{"read_word", "", 0, "read 16 bit registers",read_word, 0, 99, CMD_REPEAT},
	{"read_byte", "", 0, "read 8 bit registers", read_byte, 0, 99, CMD_REPEAT},
	{"reverse", "", 0, "reverse fan running", reverse_fan, 0, 99, CMD_REPEAT},
	{"setlow", "", 0, "set register 2", set_low, 0, 99, CMD_REPEAT},
	{"sethigh", "", 0, "set register 3", set_high, 0, 99, CMD_REPEAT},
	{"readlow", "", 0, "read  register 3", read_low, 0, 99,CMD_REPEAT},
	{"readhigh", "", 0, "read  register 3", read_high, 0, 99,CMD_REPEAT},
	{"readT", "", 0, "read  register 0 ", read_temperature, 0, 99,CMD_REPEAT},
	{"setmode", "", 0, "reverse fan running", set_mode, 0, 99, CMD_REPEAT},
	{"read", "", 0, "read registers ", read_TT, 1, 99,CMD_REPEAT},
	{"write", "", 0, "write registers ", write_TT, 1, 99,CMD_REPEAT},
	//{"rd","[-lht]",r_opts,"read registers",read_T,1,99,0},
	//{"wr","[-lhmr]",w_opts,"write registers",write_reg,1,99,0},
	{"fanon", "", 0, "write registers ", fan_on, 0, 99,CMD_REPEAT},
	{"fanoff", "", 0, "write registers ", fan_off, 0, 99,CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

static void init_cmd(void)
{
	cmdlist_expand(Cmds, 1);
}
