#include "target/load_dtb.h"
extern int is_valid_ether_addr_linux(const u8 *addr);

int dtb_cksum(void *p, size_t s, int set)
{
	unsigned int sum = 0;
	unsigned int *sp = p;
	int sz = s / 4;

	if (set) {
		*sp = 0;	/* Clear checksum */
	} else if (*sp != *(sp + sz)) {
		return -1;
	}
	while (sz--) {
		sum += *(unsigned int *)sp++;
	}

	if (set) {
		sum = -sum;
		*(volatile unsigned int *) p = sum;
		*(volatile unsigned int *) sp = sum;
		return 0;
	}
	return (sum);
}

void verify_dtb(void)
{
	char *dtbram;
	dtbram = (char *)(tgt_flashmap())->fl_map_base;
	dtbram += DTB_OFFS;
	if(dtb_cksum(dtbram, DTB_SIZE - 4, 0)) {
		printf("dtb chsum err! you should load_dtb before boot kernel!!!\n");
		return;
	}
	if(*(unsigned int *)(dtbram + 4) != 0xedfe0dd0) {
		printf("dtb has a bad bad magic, please reload dtb file!!!\n");
		return;
	}
	printf("dtb verify ok!!!\n");
}

int setup_dtb(void *ssp)
{
	char *dtbram;
	dtbram = (char *)(tgt_flashmap())->fl_map_base;
	dtbram += DTB_OFFS;
	if(dtb_cksum(dtbram, DTB_SIZE - 4, 0)) {
		printf("dtb chsum err!!!\n");
		return (-1);
	}
	memcpy((char *)ssp, (char *)(dtbram + 4), DTB_SIZE);
	return 0;
}

static char * str_in_ram(const char *p, const char *q, int size_p, int num)
{
	const char *s, *t;
	int cnt;
	for (; size_p > 0; p++, size_p--) {
		if (*p == *q) {
			t = p;
			s = q;
			cnt = size_p;
			for (; cnt > 0, *s; s++, t++, cnt--) {
				if (*t != *s)
					break;
			}
			if (!*s) {
				if(!num)
					return ((char *)p);
				else {
					num--;
					p += strlen(q);
					continue;
				}
			}
		}
	}
	return (0);
}

static int check_mac_addr(char * base)
{
	int i;
	u8 val;
	for(i = 0; i < 17; i++) {
		val = *(base + i);
		if(i % 3 == 2) {
			if(val != ':')
				return -1;
		}else if(val < '0' || (val > '9' && val < 'A') || (val > 'F' && val < 'a') || val > 'f') {
			return -1;
		}
	}
	return 0;
}

int set_dtb(int argc,char **argv)
{
        char *dtbram, *set_p;
        char cmdbuf[100];
	u8 buff[DTB_SIZE] = {0};
	u8 mac_addr[6] = {0};
        int num, i, hex;

	if(argc == 2 || argc == 3) {
		num = strtoul(argv[1], NULL, 0);
		if(num < 0 || num > MAX_PHY_NUM) {
			printf("phy_num out of range(0 ~ %d)\n", MAX_PHY_NUM);
			return 0;
		}
		if(argc == 3) {
			if(strlen(argv[2]) != 17) {
				printf("bad mac addr length\n");
				return 0;
			}
			if(check_mac_addr((char *)argv[2])) {
				printf("bad mac addr val\n");
				return 0;
			}
		}
	}else
		goto warning;

	dtbram = (char *)(tgt_flashmap())->fl_map_base;
	dtbram += DTB_OFFS;
	if(dtb_cksum(dtbram, DTB_SIZE - 4, 0)) {
		printf("dtb chsum err! you should reload dtb file!!!\n");
		return -1;
	}
	memcpy(buff, dtbram, DTB_SIZE);

	if(!(set_p = str_in_ram((char *)(buff + 4), "ethernet", DTB_SIZE - 8, num))) {
		printf("can not find \"ethernet\" in dtb file\n");
		return -1;
	}
	set_p += MAC_OFFS;

	if(argc == 2) {
		printf("mac addr of eth%d is:%02x:%02x:%02x:%02x:%02x:%02x\n",	\
			num, *set_p, *(set_p + 1), *(set_p + 2), \
			*(set_p + 3), *(set_p + 4), *(set_p + 5));
		return 0;
	}
	for(i = 0; i < 6; i++) {
		gethex(&hex, &argv[2][3 * i], 2);
		if (hex & ~0xff) {
			printf("get wrong hex value of mac addr, abort!\n");
			return 0;
		}
		mac_addr[i] = hex;
	}
	if(!is_valid_ether_addr_linux(mac_addr)) {
		printf("mac addr is useless, abort!\n");
		return 0;
	}

	memcpy(set_p, mac_addr, MAC_LENTH);
	dtb_cksum(buff, DTB_SIZE - 4, 1);
	tgt_flashprogram(dtbram, DTB_SIZE, buff, 0);
	return 0;
warning:
	printf("set_dtb <phy_num> <mac_addr>\n");
	printf("eg: set_dtb 0 \"00:11:22:33:44:55\"\n");
	printf("the mac addr of eth0 is set to be: 00:11:22:33:44:55\n");
	return 0;
}

int load_dtb(int argc,char **argv)
{
	char *dtbram;
	char cmdbuf[100];
	u8 buff[DTB_SIZE] = {0};

	if(argc==2){
		sprintf(cmdbuf,"load -o %p -r %s", buff + 4, argv[1]);
		printf("load -o %p -r %s\n",buff + 4,argv[1]);
		do_cmd(cmdbuf);
	}else {
		printf("argc error\n");
		return -1;
	}
	dtb_cksum(buff, DTB_SIZE - 4, 1);

	dtbram = (char *)(tgt_flashmap())->fl_map_base;
	dtbram += DTB_OFFS;

	tgt_flashprogram(dtbram, DTB_SIZE, buff, 0);
	return 0;
}

static const Cmd Cmds[] =
{
	{"Misc"},
	{"load_dtb","file",0,"load_dtb file(max size 16K)", load_dtb,0,99,CMD_REPEAT},
	{"set_dtb","file",0,"set mac addr in dtb", set_dtb,0,99,CMD_REPEAT},
	{0, 0}
};
static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
