#include <sys/linux/types.h>
#include <pmon.h>
#include <stdio.h>
#include <machine/pio.h>
#include "target/ls2k.h"
#include "target/board.h"

#include "generate_mac_val.c"

#ifndef LS2K_EEPROM_MAC

extern unsigned char hwethadr[12];

static int ls2k_eeprom_read_seq(unsigned char data_addr, unsigned char *buf, int count)
{
	int i;

	if (count != 6)
		return 0;

	bcopy((void *)&hwethadr + data_addr, buf, 6);

	return count;
}

static int ls2k_eeprom_write_page(unsigned char data_addr, unsigned char *buf, int count)
{
	char hexbuf[128];

	memset(&hexbuf, 0, 128);

	if (count != 6)
		return 0;

	sprintf(hexbuf, "%02x:%02x:%02x:%02x:%02x:%02x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

	if (data_addr == 0) {
		tgt_setenv("ethaddr", &hexbuf);
	} else {
		tgt_setenv("ethaddr1", &hexbuf);
	}

	return count;
}
#endif

int mac_read(unsigned char data_addr, unsigned char *buf, int count)
{
	int i;

	i = ls2k_eeprom_read_seq(data_addr, buf, count);

	if (!i) {
		printf("get random MAC address: ");
		generate_mac_val(buf);

		for (i = 0; i < count; i++) 
			printf("%02x%s", buf[i], (i == (count - 1))? "":":");
		printf("\n");

		return i;
	}

	if (!is_valid_ether_addr_linux(buf)){
		printf("Mac is invalid, now get a random mac\n");
		generate_mac_val(buf);
	}

	return i;
}

int cmd_setmac(int ac, unsigned char *av[])
{
	int i, j, v, count, param = 0;
	unsigned char *s = NULL;
	unsigned char data_addr;
	unsigned char buf[32] = {0};

	switch (ac) {
		case 1:
		case 2:
			param = 1;
			break;
		case 3:
			break;
		default:
			goto warning;
	}

	if (param == 1) {
		for (i = 0; i < 2; i++) {	
			if (ls2k_eeprom_read_seq(i * 6, buf, 6) == 6) {
				if (!is_valid_ether_addr_linux(buf)){
					printf("syn%d Mac is invalid, now get a new mac\n", i);
					generate_mac_val(buf);
					if (ls2k_eeprom_write_page((i * 6), buf, 6) == 6) {
						printf("set syn%d  Mac address: ",i);
						for (v = 0;v < 6;v++)
							printf("%2x%s",*(buf + v) & 0xff,(5-v)?":":" ");
						printf("\n");
						printf("The machine should be restarted to make the new mac change to take effect!!\n");
						} else
							printf("eeprom write error!\n");
					printf("you can set it by youself\n");
				} else {
					printf("syn%d Mac address: ", i);
					for (j = 0; j < 6; j++)
						printf("%02x%s", buf[j], (5-j)?":":" ");
					printf("\n");
				}
			} else {
				printf("eeprom write error!\n");
				return 0;
			}
		}
		goto warning;
	}

	if (av[2]) s = av[2];
	else goto warning; 

	count = strlen(s) / 3 + 1;
	if (count - 6) goto warning;

	for (i = 0; i < count; i++) {
		gethex(&v, s, 2); 
		buf[i] = v;
		s += 3;
	}

	data_addr = strtoul(av[1] + 3, NULL, 0);
	data_addr *= 6;

	if (ls2k_eeprom_write_page(data_addr, buf, count) == count) {
		printf("set syn%d  Mac address: %s\n",data_addr / 6, av[2]);
		printf("The machine should be restarted to make the mac change to take effect!!\n");
	} else 
		printf("eeprom write error!\n");
	return 0;
warning:
	printf("Please accord to correct format.\nFor example:\n");
	printf("\tsetmac syn1 \"00:11:22:33:44:55\"\n");
	printf("\tThis means set syn1's Mac address 00:11:22:33:44:55\n");
	return 0;
}

static const Cmd Cmds[] = {
	{"Misc"},
	{"setmac", "", NULL, "set the Mac address of LS2K syn0 and syn1", cmd_setmac, 1, 5, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
