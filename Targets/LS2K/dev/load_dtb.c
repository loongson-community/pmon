#include "target/load_dtb.h"
#include <machine/frame.h>

extern char *heaptop;
extern char ls2k_version();
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

static char * str_in_ram(const char *p, const char *q, int size_p, int num)
{
	const char *s, *t;
	const char *p_t, *q_t;
	int cnt;
	p_t = p;
	q_t = q;
	for (; size_p > 0; p_t++, size_p--) {
		if (*p_t == *q_t) {
			t = p_t;
			s = q_t;
			cnt = size_p;
			for (; cnt > 0, *s; s++, t++, cnt--) {
				if (*t != *s)
					break;
			}
			if (!*s) {
				if(!num)
					return ((char *)p_t);
				else {
					num--;
					p_t += strlen(q_t);
					continue;
				}
			}
		}
	}
	return (0);
}

static int update_dma_flag(void * ssp)
{
	char *set_p;
	if(!(set_p = str_in_ram((char *)ssp, "coherent", DTB_SIZE - 8, 0))) {
		printf("can not find \"dma-coherent\" or \"not-coherent\" in dtb file\n");
		return -1;
	}
	if(ls2k_version())
		memcpy((char *)set_p - 4, "dma-", 4);
	else
		memcpy((char *)set_p - 4, "not-", 4);
	return 0;
}

static int update_bootarg(int ac, char ** av, void * ssp)
{
	int i, len, len_new;
	char new_arg[200];
	char *set_p, *p;
	struct trapframe *tf = cpuinfotab[whatcpu];

	if(!(set_p = str_in_ram((char *)ssp, "console", DTB_SIZE - 8, 0))) {
		printf("can not find \"console\" in dtb file\n");
		return -1;
	}
	len = strlen(set_p);
	p = new_arg;
	for (i = 1; i < ac; i++) {
		strcpy(p, av[i]);
		p += strlen(av[i]);
		*p = ' ';
		p++;
	}
	p--;
	*p = '\0';
	len_new = p - new_arg;

	if(len < len_new) {
		printf("len=0x%x; len_new=0x%x\n", len, len_new);
		printf("error: boot arg is too long! use the default boot arg!\nyou can change dts file and make a new dtb file.\n\n");
	}else {
		memset((char *)set_p, 0, len);
		memcpy((char *)set_p, (char *)new_arg, len_new);
	}
	return 0;
}

struct trapframe * setup_dtb(int ac, char ** av, void * ssp)
{
	char *dtbram;
	struct trapframe *tf = cpuinfotab[whatcpu];
	ssp = heaptop;
	tf->a2 = heaptop;

	dtbram = (char *)(tgt_flashmap())->fl_map_base;
	dtbram += DTB_OFFS;
	if(*(unsigned int *)(dtbram + 4) != 0xedfe0dd0 || dtb_cksum(dtbram, DTB_SIZE - 4, 0)) {
		printf("dtb chsum err!!!\n");
		tf->a2 = NULL;
		return tf;
	}
	memcpy((char *)ssp, (char *)(dtbram + 4), DTB_SIZE - 8);

	update_dma_flag(ssp);
	if (ac > 1)
		update_bootarg(ac, av, ssp);

	return tf;
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

static int set_dtb(int argc,char **argv)
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

	if(argc==2){
		sprintf(cmdbuf,"load -o %p -r %s", heaptop + 4, argv[1]);
		printf("load -o %p -r %s\n",heaptop + 4,argv[1]);
		do_cmd(cmdbuf);
	}else {
		printf("argc error\n");
		return -1;
	}
	dtb_cksum(heaptop, DTB_SIZE - 4, 1);

	dtbram = (char *)(tgt_flashmap())->fl_map_base;
	dtbram += DTB_OFFS;

	tgt_flashprogram(dtbram, DTB_SIZE, heaptop, 0);
	return 0;
}

void erase_dtb(void)
{
	char *dtbram;

	dtbram = (char *)(tgt_flashmap())->fl_map_base;
	dtbram += DTB_OFFS;

	if (fl_erase_device(dtbram, DTB_SIZE, TRUE)) {
		printf("Erase failed!\n");
	}
}

static const Cmd Cmds[] =
{
	{"Misc"},
	{"load_dtb","file",0,"load_dtb file(max size 16K)", load_dtb,0,99,CMD_REPEAT},
	{"set_dtb","file",0,"set mac addr in dtb", set_dtb,0,99,CMD_REPEAT},
	{"erase_dtb","file",0,"erase dtb file in flash", erase_dtb,0,99,CMD_REPEAT},
	{0, 0}
};
static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
