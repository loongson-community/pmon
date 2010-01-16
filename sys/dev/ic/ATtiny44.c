/*
 * Copyright (C) 2009 Lemote Inc.
 * Author: Junliang Liu, liujl@lemote.com
 * Author: Yu Xiang, xiangy@lemote.com
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include "stdio.h"
#include "include/ATtiny44.h"

typedef unsigned int pcitag_t; 

extern volatile char *mmio;
extern void delay(int);
extern int flash_send(unsigned char, unsigned char, unsigned char);
extern int flash_recv(unsigned char, unsigned char, unsigned char *);
extern pcitag_t _pci_make_tag(int, int, int);
extern pcitag_t _pci_conf_readn(pcitag_t, int, int);
extern void _pci_conf_writen(pcitag_t, int, pcitag_t, int);

int attiny44_update_rom(void *src, int size)
{
	int i;
	unsigned char ret = 0;
	int check_timeout = 0;
	pcitag_t tag, temp;
	char ATTINY_PID[] = "GDIUM";
	unsigned char value, check_sum = 0;
	unsigned long block_num = 0;
	unsigned long all_blocks = 0;
	volatile unsigned char *ptr = src;
	volatile unsigned char buffer[ATTINY_PROGRAM_SIZE];
	
	/* set the mmio base and config the gpio clock */
	tag  = _pci_make_tag(0, 14, 0);
	mmio = (volatile char *)_pci_conf_readn(tag, 0x14, 4);
	mmio = (unsigned int)mmio | 0xb0000000;
	temp = _pci_conf_readn(tag, 0x04, 4);
	_pci_conf_writen(tag, 0x04, temp | 0x07, 0x04);

	/* check the size of program */
	if((size > ATTINY_MAX_SIZE) || (size < 0x100)){
		printf("ATtiny : size invalied .invalid size is between %d Bytes and %d Bytes\n", 0x100,ATTINY_MAX_SIZE);
		return 1;
	}

	for(i = 0; i < size; i++)
		buffer[i] = ptr[i];
	for(i = size; i < ATTINY_PROGRAM_SIZE; i++)
		buffer[i] = 0xff;
	for(i = 0; i < ATTINY_PROGRAM_SIZE; i++)
		check_sum += buffer[i];
		
	ptr = &buffer[0];
	printf("ATtiny : program size 0x%x, src addr 0x%x\n", ATTINY_PROGRAM_SIZE, (unsigned long)ptr);

	flash_recv(ATTINY_SLAVE_ADDRESS, 2, &ret);
	if(ret == 0x1f){
		ret = flash_send(ATTINY_SLAVE_ADDRESS, 1, 0xf4);
		if(ret){
			printf("ATtiny : start instruction fail.\n");
			return 1;
		}
	}

	delay(10000); //delay 10ms

	/* start program stage */
	for(i = 0; i < 5; i++) {
		ret = flash_send(ATTINY_SLAVE_ADDRESS, i, ATTINY_PID[i]);
		if(ret){
			printf("ATtiny : start program %d error.\n", i);
			return 2;
		}
	}
	
	ret = flash_send(ATTINY_SLAVE_ADDRESS, 5, ATTINY_PID_S);
	if(ret){
		printf("ATtiny : start program 5 error.\n");
		return 2;
	}
	ret = flash_send(ATTINY_SLAVE_ADDRESS, 6, (unsigned char)( (ATTINY_PROGRAM_SIZE & 0xff00) >> 8) );
	if(ret){
		printf("ATtiny : start program 6 error.\n");
		return 2;
	}
	ret = flash_send(ATTINY_SLAVE_ADDRESS, 7, (unsigned char)(ATTINY_PROGRAM_SIZE & 0x00ff));
	if(ret){
		printf("ATtiny : start program 7 error.\n");
		return 2;
	}

	/* check the ack */
	while(value != ATTINY_ACK_START){
		ret = flash_recv(ATTINY_SLAVE_ADDRESS, ATTINY_ACK_REG, &value);
		if(ret){
			printf("ATtiny : start ack error.\n");
			return 2;
		}
		delay(10000);	// delay 10ms for not busy
		if(value == ATTINY_ACK_ERROR){
			printf("ATtiny : size error, should be with 32bytes unit.\n");
			return 2;
		}
	}

	printf("stage 1(start) over.\n");

	/* program flash stage : calculate block number*/
	all_blocks = ATTINY_PROGRAM_SIZE / ATTINY_BLOCK_SIZE;
	for(block_num = 0; block_num < all_blocks; block_num++) {
		/* program each block */
		for(i = 0; i < ATTINY_BLOCK_SIZE; i++){
			ret = flash_send(ATTINY_SLAVE_ADDRESS, i, *ptr++);
			if(ret){
				printf("ATtiny : program block %d error.\n", block_num);
				return 3;
			}
		}
		/* add the pad for ATtiny bug */
		ret = flash_send(ATTINY_SLAVE_ADDRESS, ATTINY_BLOCK_SIZE, 0x55);
		if(ret){
			printf("ATtiny : program pad error.\n");
			return 3;
		}
		delay(10000); //delay 10ms

		/* check the ack */
		while(value != ATTINY_ACK_DATA){
			ret = flash_recv(ATTINY_SLAVE_ADDRESS, ATTINY_ACK_REG, &value);
			if(ret){
				printf("ATtiny : program ack error.\n");
				return 3;
			}
			delay(5000);	// delay 5ms for not busy
		}
		printf(".");
	}

	printf("\nstage 2(program) over.\n");

	/* end program stage */
	for(i = 0; i < 5; i++) {
		ret = flash_send(ATTINY_SLAVE_ADDRESS, i, ATTINY_PID[i]);
		if(ret){
			printf("ATtiny : end program %d error.\n", i);
			return 4;
		}
	}

	ret = flash_send(ATTINY_SLAVE_ADDRESS, 5, ATTINY_PID_E);
	if(ret){
		printf("ATtiny : end program 5 error.\n");
		return 4;
	}

	/* check the end ack */
	check_timeout = 100;
	while(check_timeout--){
		ret = flash_recv(ATTINY_SLAVE_ADDRESS, ATTINY_ACK_REG, &value);
		if(ret){
			printf("ATtiny : end ack error.\n");
			return 4;
		}
		delay(1000);	// delay 1ms for not busy
		if(value == check_sum) {
			printf("ATtiny : check sum ok.\n");
			break;
		}
	}
	if(check_timeout <= 0){
		printf("ATtiny :check sum = 0x%x, recv check sum 0x%x\n", check_sum, value);
		printf("ATtiny : check sum error, ATTINY can't work normally, re-program it after power off.\n");
	}
	printf("stage 3(checksum) over.\n");

	/* checksum send stage */
	ret = flash_send(ATTINY_SLAVE_ADDRESS, 0, 'O');
	if(ret){
		printf("ATtiny : checksum program 0 error.\n");
		return 4;
	}
	ret = flash_send(ATTINY_SLAVE_ADDRESS, 1, 'K');
	if(ret){
		printf("ATtiny : checksum program 1 error.\n");
		return 4;
	}
	printf("stage 4(end) over.\n");

	printf("ATtiny : program successful.\n");

	return 0;
}
