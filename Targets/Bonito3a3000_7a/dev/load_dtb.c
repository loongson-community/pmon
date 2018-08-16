#include "target/load_dtb.h"
#include <libfdt.h>

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

static int check_mac_ok(void)
{
	const void *nodep;	/* property node pointer */
	int  nodeoffset;	/* node offset from libfdt */
	int  len, id, i;		/* length of the property */
	u8 mac_addr[6] = {0x00, 0x55, 0x7B, 0xB5, 0x7D, 0xF7};	//default mac address
	char ethernet_name[2][25]={"/soc/ethernet@0x40000000", "/soc/ethernet@0x40010000"};

	for(id = 0;id < 2;id++) {
		nodeoffset = fdt_path_offset (working_fdt, ethernet_name[id]);
		if (nodeoffset < 0) {
			return 1;	//no ethernet device, do nothing
		}
		nodep = fdt_getprop (working_fdt, nodeoffset, (const char* )"mac-address", &len);
		if(len <= 0) {
			return 1;	//no mac prop in ethernet, do nothing
		}
	
		i2c_init();//configure the i2c freq
		mac_read(id * 6, mac_addr, 6);
		for(i = 0;i < 6;i++) {
			if(mac_addr[i] != (*((char *)nodep + i) & 0xff)) {
				printf("mac_eeprom[%d]=0x%x; mac_dtb[%d]=0x%x; reset mac addr in dtb\n", \
						i, mac_addr[i], i, *((char *)nodep + i) & 0xff);
				return 0;
			}
		}
	}
	return 1;
}

static int update_mac(void * ssp, int id)
{
	int nodeoffset, len;
	void *nodep;	/* property node pointer */
	u8 mac_addr[6] = {0x00, 0x55, 0x7B, 0xB5, 0x7D, 0xF7};	//default mac address
	char ethernet_name[2][25]={"/soc/ethernet@0x40000000", "/soc/ethernet@0x40010000"};

	nodeoffset = fdt_path_offset (ssp, ethernet_name[id]);
	if (nodeoffset < 0) {
		printf ("libfdt fdt_path_offset() returned %s\n", fdt_strerror(nodeoffset));
		return -1;
	}
	nodep = fdt_getprop ((const char* )ssp, nodeoffset, (const char* )"mac-address", &len);
	if(len <= 0) {
		printf("can not find property: mac-address\n");
		return -1;
	}

	i2c_init();//configure the i2c freq
	mac_read(id * 6, mac_addr, 6);
	len = 6;
	if(fdt_setprop(ssp, nodeoffset, "mac-address", mac_addr, len)) {
		printf("Update mac-address%d error\n", id);
		return -1;
	}
	return 0;
}

void verify_dtb(void)
{
	char *dtbram;
	dtbram = (char *)(tgt_flashmap()->fl_map_base);
	dtbram += DTB_OFFS;
	working_fdt = (struct fdt_header *)(dtbram + 4);
printf("working_fdt=%p\n", working_fdt);
	if(dtb_cksum(dtbram, DTB_SIZE - 4, 0)) {
		printf("dtb chsum err! you should load_dtb before boot kernel!!!\n");
		return;
	}
	if(*(unsigned int *)(dtbram + 4) != 0xedfe0dd0) {
		printf("dtb has a bad bad magic, please reload dtb file!!!\n");
		return;
	}
	printf("dtb verify ok!!!\n");
	if(!check_mac_ok()) {
		u8 buff[DTB_SIZE] = {0};
		fdt_open_into(working_fdt, buff + 4, DTB_SIZE - 8);
		update_mac(buff + 4, 0);
		update_mac(buff + 4, 1);
		dtb_cksum(buff, DTB_SIZE - 4, 1);
		tgt_flashprogram((char *)working_fdt - 4, DTB_SIZE, buff, 0);
	}
}

/*
 * Parse the user's input, partially heuristic.  Valid formats:
 * <0x00112233 4 05>	- an array of cells.  Numbers follow standard
 *			C conventions.
 * [00 11 22 .. nn] - byte stream
 * "string"	- If the the value doesn't start with "<" or "[", it is
 *			treated as a string.  Note that the quotes are
 *			stripped by the parser before we get the string.
 * newval: An array of strings containing the new property as specified
 *	on the command line
 * count: The number of strings in the array
 * data: A bytestream to be placed in the property
 * len: The length of the resulting bytestream
 */
static int fdt_parse_prop(char * const *newval, int count, char *data, int *len)
{
	char *cp;		/* temporary char pointer */
	char *newp;		/* temporary newval char pointer */
	unsigned long tmp;	/* holds converted values */
	int stridx = 0;

	*len = 0;
	newp = newval[0];

	/* An array of cells */
	if (*newp == '<') {
		newp++;
		while ((*newp != '>') && (stridx < count)) {
			/*
			 * Keep searching until we find that last ">"
			 * That way users don't have to escape the spaces
			 */
			if (*newp == '\0') {
				newp = newval[++stridx];
				continue;
			}

			cp = newp;
			tmp = strtoul(cp, &newp, 0);
			*(fdt32_t *)data = cpu_to_fdt32(tmp);
			data  += 4;
			*len += 4;

			/* If the ptr didn't advance, something went wrong */
			if ((newp - cp) <= 0) {
				printf("Sorry, I could not convert \"%s\"\n",
					cp);
				return 1;
			}

			while (*newp == ' ')
				newp++;
		}

		if (*newp != '>') {
			printf("Unexpected character '%c'\n", *newp);
			return 1;
		}
	} else if (*newp == '[') {
		/*
		 * Byte stream.  Convert the values.
		 */
		newp++;
		while ((stridx < count) && (*newp != ']')) {
			while (*newp == ' ')
				newp++;
			if (*newp == '\0') {
				newp = newval[++stridx];
				continue;
			}
			if (!isxdigit(*newp))
				break;
			tmp = strtoul(newp, &newp, 16);
			*data++ = tmp & 0xFF;
			*len    = *len + 1;
		}
		if (*newp != ']') {
			printf("Unexpected character '%c'\n", *newp);
			return 1;
		}
	} else {
		/*
		 * Assume it is one or more strings.  Copy it into our
		 * data area for convenience (including the
		 * terminating '\0's).
		 */
		while (stridx < count) {
			size_t length = strlen(newp) + 1;
			strcpy(data, newp);
			data += length;
			*len += length;
			newp = newval[++stridx];
		}
	}
	return 0;
}

static int update_bootarg(int ac, char ** av, void * ssp)
{
	int i, len = 0, ret;
	char new_arg[200];
	char *p;
	int nodeoffset;

	p = new_arg;
	for (i = 1; i < ac; i++) {
		strcpy(p, av[i]);
		p += strlen(av[i]);
		*p = ' ';
		p++;
	}
	p--;
	*p = '\0';
	len = strlen(new_arg) + 1;
retry:
	nodeoffset = fdt_path_offset (ssp, "/chosen");
	ret = fdt_setprop(ssp, nodeoffset, "bootargs", new_arg, len);
	if(ret == -FDT_ERR_NOSPACE && fdt_totalsize(working_fdt) < DTB_SIZE - 8) {
		fdt_open_into(working_fdt, ssp, DTB_SIZE - 8);
		goto retry;
	}else
		printf("Update bootarg error\n");
	return 0;
}

#define DEBUG_DTB 0
#if DEBUG_DTB
static void print_data(const void *data, int len);
static int fdt_print_ram(char * head, const char *pathp, char *prop, int depth)
{
	static char tabs[MAX_LEVEL+1] =
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	const void *nodep;	/* property node pointer */
	int  nodeoffset;	/* node offset from libfdt */
	int  nextoffset;	/* next node offset from libfdt */
	uint32_t tag;		/* tag */
	int  len;		/* length of the property */
	int  level = 0;		/* keep track of nesting level */
	const struct fdt_property *fdt_prop;

	nodeoffset = fdt_path_offset (head, pathp);
	if (nodeoffset < 0) {
		/*
		 * Not found or something else bad happened.
		 */
		printf ("libfdt fdt_path_offset() returned %s\n",
			fdt_strerror(nodeoffset));
		return 1;
	}
	/*
	 * The user passed in a property as well as node path.
	 * Print only the given property and then return.
	 */
	if (prop) {
		nodep = fdt_getprop (head, nodeoffset, prop, &len);
		if (len == 0) {
			/* no property value */
			printf("%s %s\n", pathp, prop);
			return 0;
		} else if (len > 0) {
			printf("%s = ", prop);
			print_data(nodep, len);
			printf("\n");
			return 0;
		} else {
			printf ("libfdt fdt_getprop(): %s\n",
				fdt_strerror(len));
			return 1;
		}
	}

	/*
	 * The user passed in a node path and no property,
	 * print the node and all subnodes.
	 */
	while(level >= 0) {
		tag = fdt_next_tag(head, nodeoffset, &nextoffset);
		switch(tag) {
		case FDT_BEGIN_NODE:
			pathp = fdt_get_name(head, nodeoffset, NULL);
			if (level <= depth) {
				if (pathp == NULL)
					pathp = "/* NULL pointer error */";
				if (*pathp == '\0')
					pathp = "/";	/* root is nameless */
				printf("%s%s {\n",
					&tabs[MAX_LEVEL - level], pathp);
			}
			level++;
			if (level >= MAX_LEVEL) {
				printf("Nested too deep, aborting.\n");
				return 1;
			}
			break;
		case FDT_END_NODE:
			level--;
			if (level <= depth)
				printf("%s};\n", &tabs[MAX_LEVEL - level]);
			if (level == 0) {
				level = -1;		/* exit the loop */
			}
			break;
		case FDT_PROP:
			fdt_prop = fdt_offset_ptr(head, nodeoffset,
					sizeof(*fdt_prop));
			pathp    = fdt_string(head,
					fdt32_to_cpu(fdt_prop->nameoff));
			len      = fdt32_to_cpu(fdt_prop->len);
			nodep    = fdt_prop->data;
			if (len < 0) {
				printf ("libfdt fdt_getprop(): %s\n",
					fdt_strerror(len));
				return 1;
			} else if (len == 0) {
				/* the property has no value */
				if (level <= depth)
					printf("%s%s;\n",
						&tabs[MAX_LEVEL - level],
						pathp);
			} else {
				if (level <= depth) {
					printf("%s%s = ", &tabs[MAX_LEVEL - level], pathp);
					print_data(nodep, len);
					printf(";\n");
				}
			}
			break;
		case FDT_NOP:
			printf("%s/* NOP */\n", &tabs[MAX_LEVEL - level]);
			break;
		case FDT_END:
			return 1;
		default:
			if (level <= depth)
				printf("Unknown tag 0x%08X\n", tag);
			return 1;
		}
		nodeoffset = nextoffset;
	}
	return 0;
}

#endif
unsigned long long setup_dtb(int ac, char ** av)
{
	char * ssp = heaptop;

	if(*(unsigned int *)(working_fdt) != 0xedfe0dd0 || dtb_cksum((char *)working_fdt - 4, DTB_SIZE - 4, 0)) {
		printf("dtb chsum err!!!\n");
		return 0ULL;
	}
	memcpy(ssp, (char *)working_fdt, DTB_SIZE - 8);
	if (ac > 1)
		update_bootarg(ac, av, ssp);	//note: maybe reload from working_fdt, so do this first

#if DEBUG_DTB
	fdt_print_ram(ssp, "/", NULL, MAX_LEVEL);
#endif

	return (unsigned long long)heaptop;
}

int load_dtb(int argc,char **argv)
{
	char cmdbuf[100];

	if(argc == 2) {
		sprintf(cmdbuf,"load -o %p -r %s", heaptop + 4, argv[1]);
		printf("load -o %p -r %s\n",heaptop + 4,argv[1]);
		if(do_cmd(cmdbuf)) {
			printf("load dtb error\n");
			return -1;
		}
	}else {
		printf("argc error\n");
		printf("load_dtb <path>\n");
		printf("eg: load_dtb tftp://xx.xx.xx.xx/ls2k.dtb\n");
		return -1;
	}
	if(*(unsigned int *)(heaptop + 4) != 0xedfe0dd0) {
		printf("dtb has a bad bad magic, please reload dtb file!!!\n");
		return -1;
	}
//	update_dma_flag(heaptop + 4);
	update_mac(heaptop + 4, 0);
	update_mac(heaptop + 4, 1);
	dtb_cksum(heaptop, DTB_SIZE - 4, 1);

	tgt_flashprogram((char *)working_fdt - 4, DTB_SIZE, heaptop, 0);
	return 0;
}

int erase_dtb(int ac, char ** av)
{
	if (fl_erase_device((char *)working_fdt - 4, DTB_SIZE, TRUE)) {
		printf("Erase failed!\n");
		return -1;
	}else
		printf("Erase ok!\n");
	return 0;
}

/****************************************************************************/

/*
 * Heuristic to guess if this is a string or concatenated strings.
 */

static int is_printable_string(const void *data, int len)
{
	const char *s = data;

	/* zero length is not */
	if (len == 0)
		return 0;

	/* must terminate with zero or '\n' */
	if (s[len - 1] != '\0' && s[len - 1] != '\n')
		return 0;

	/* printable or a null byte (concatenated strings) */
	while (((*s == '\0') || isprint(*s) || isspace(*s)) && (len > 0)) {
		/*
		 * If we see a null, there are three possibilities:
		 * 1) If len == 1, it is the end of the string, printable
		 * 2) Next character also a null, not printable.
		 * 3) Next character not a null, continue to check.
		 */
		if (s[0] == '\0') {
			if (len == 1)
				return 1;
			if (s[1] == '\0')
				return 0;
		}
		s++;
		len--;
	}

	/* Not the null termination, or not done yet: not printable */
	if (*s != '\0' || (len != 0))
		return 0;

	return 1;
}

/*
 * Print the property in the best format, a heuristic guess.  Print as
 * a string, concatenated strings, a byte, word, double word, or (if all
 * else fails) it is printed as a stream of bytes.
 */
static void print_data(const void *data, int len)
{
	int j;

	/* no data, don't print */
	if (len == 0)
		return;

	/*
	 * It is a string, but it may have multiple strings (embedded '\0's).
	 */
	if (is_printable_string(data, len)) {
		printf("\"");
		j = 0;
		while (j < len) {
			if (j > 0)
				printf("\", \"");
			printf(data);
			j    += strlen(data) + 1;
			data += strlen(data) + 1;
		}
		printf("\"");
		return;
	}

	if ((len %4) == 0) {
		if (len > CONFIG_CMD_FDT_MAX_DUMP)
			printf("* 0x%p [0x%08x]", data, len);
		else {
			//const __be32 *p;
			const u32 *p;

			printf("<");
			for (j = 0, p = data; j < len/4; j++)
				printf("0x%08x%s", fdt32_to_cpu(p[j]),
					j < (len/4 - 1) ? " " : "");
			printf(">");
		}
	} else { /* anything else... hexdump */
		if (len > CONFIG_CMD_FDT_MAX_DUMP)
			printf("* 0x%p [0x%08x]", data, len);
		else {
			const u8 *s;

			printf("[");
			for (j = 0, s = data; j < len; j++)
				printf("%02x%s", s[j], j < len - 1 ? " " : "");
			printf("]");
		}
	}
}

/*
 * Recursively print (a portion of) the working_fdt.  The depth parameter
 * determines how deeply nested the fdt is printed.
 */
static int fdt_print(const char *pathp, char *prop, int depth)
{
	static char tabs[MAX_LEVEL+1] =
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	const void *nodep;	/* property node pointer */
	int  nodeoffset;	/* node offset from libfdt */
	int  nextoffset;	/* next node offset from libfdt */
	uint32_t tag;		/* tag */
	int  len;		/* length of the property */
	int  level = 0;		/* keep track of nesting level */
	const struct fdt_property *fdt_prop;

	nodeoffset = fdt_path_offset (working_fdt, pathp);
	if (nodeoffset < 0) {
		/*
		 * Not found or something else bad happened.
		 */
		printf ("libfdt fdt_path_offset() returned %s\n",
			fdt_strerror(nodeoffset));
		return 1;
	}
	/*
	 * The user passed in a property as well as node path.
	 * Print only the given property and then return.
	 */
	if (prop) {
		nodep = fdt_getprop (working_fdt, nodeoffset, prop, &len);
		if (len == 0) {
			/* no property value */
			printf("%s %s\n", pathp, prop);
			return 0;
		} else if (len > 0) {
			printf("%s = ", prop);
			print_data(nodep, len);
			printf("\n");
			return 0;
		} else {
			printf ("libfdt fdt_getprop(): %s\n",
				fdt_strerror(len));
			return 1;
		}
	}

	/*
	 * The user passed in a node path and no property,
	 * print the node and all subnodes.
	 */
	while(level >= 0) {
		tag = fdt_next_tag(working_fdt, nodeoffset, &nextoffset);
		switch(tag) {
		case FDT_BEGIN_NODE:
			pathp = fdt_get_name(working_fdt, nodeoffset, NULL);
			if (level <= depth) {
				if (pathp == NULL)
					pathp = "/* NULL pointer error */";
				if (*pathp == '\0')
					pathp = "/";	/* root is nameless */
				printf("%s%s {\n",
					&tabs[MAX_LEVEL - level], pathp);
			}
			level++;
			if (level >= MAX_LEVEL) {
				printf("Nested too deep, aborting.\n");
				return 1;
			}
			break;
		case FDT_END_NODE:
			level--;
			if (level <= depth)
				printf("%s};\n", &tabs[MAX_LEVEL - level]);
			if (level == 0) {
				level = -1;		/* exit the loop */
			}
			break;
		case FDT_PROP:
			fdt_prop = fdt_offset_ptr(working_fdt, nodeoffset,
					sizeof(*fdt_prop));
			pathp    = fdt_string(working_fdt,
					fdt32_to_cpu(fdt_prop->nameoff));
			len      = fdt32_to_cpu(fdt_prop->len);
			nodep    = fdt_prop->data;
			if (len < 0) {
				printf ("libfdt fdt_getprop(): %s\n",
					fdt_strerror(len));
				return 1;
			} else if (len == 0) {
				/* the property has no value */
				if (level <= depth)
					printf("%s%s;\n",
						&tabs[MAX_LEVEL - level],
						pathp);
			} else {
				if (level <= depth) {
					printf("%s%s = ", &tabs[MAX_LEVEL - level], pathp);
					print_data(nodep, len);
					printf(";\n");
				}
			}
			break;
		case FDT_NOP:
			printf("%s/* NOP */\n", &tabs[MAX_LEVEL - level]);
			break;
		case FDT_END:
			return 1;
		default:
			if (level <= depth)
				printf("Unknown tag 0x%08X\n", tag);
			return 1;
		}
		nodeoffset = nextoffset;
	}
	return 0;
}

int print_dtb(int argc,char **argv)
{
	int depth = MAX_LEVEL;	/* how deep to print */
	char *pathp;		/* path */
	char *prop = NULL;		/* property */
	int  ret;		/* return value */

	/*
	 * Get the starting path.  The root node is an oddball,
	 * the offset is zero and has no name.
	 */
	if (argc == 1)
		goto warning;
	else
		pathp = argv[1];
	if (argc > 2)
		prop = argv[2];

	if(!strncmp(argv[1], "head", 4)) {
		u32 version = fdt_version(working_fdt);
		printf("magic:\t\t\t0x%x\n", fdt_magic(working_fdt));
		printf("totalsize:\t\t0x%x (%d)\n", fdt_totalsize(working_fdt), \
			fdt_totalsize(working_fdt));
		printf("off_dt_struct:\t\t0x%x\n", fdt_off_dt_struct(working_fdt));
		printf("off_dt_strings:\t\t0x%x\n", fdt_off_dt_strings(working_fdt));
		printf("off_mem_rsvmap:\t\t0x%x\n", fdt_off_mem_rsvmap(working_fdt));
		printf("version:\t\t%d\n", version);
		printf("last_comp_version:\t%d\n", fdt_last_comp_version(working_fdt));
		if (version >= 2)
			printf("boot_cpuid_phys:\t0x%x\n", fdt_boot_cpuid_phys(working_fdt));
		if (version >= 3)
			printf("size_dt_strings:\t0x%x\n", fdt_size_dt_strings(working_fdt));
		if (version >= 17)
			printf("size_dt_struct:\t\t0x%x\n", fdt_size_dt_struct(working_fdt));
		printf("number mem_rsv:\t\t0x%x\n", fdt_num_mem_rsv(working_fdt));
		return 0;
	}

	ret = fdt_print(pathp, prop, depth);
	return ret;
warning:
	printf("print_dtb <path> [prop] \n");
	printf("eg: print_dtb head			- print dtb header\n");
	printf("eg: print_dtb /				- print all the device three\n");
	printf("eg: print_dtb /soc/dc@0x400c0000	- print property of dc\n");
	printf("eg: print_dtb /soc/dc@0x400c0000 reg	- print reg's value of dc\n");
	return 0;
}

static int set_dtb(int argc,char **argv)
{
	int  nodeoffset;	/* node offset from libfdt */
	static char data[SCRATCHPAD];	/* storage for the property */
	int  len;		/* new length of the property */
	int  ret;		/* return value */
	u8 buff[DTB_SIZE] = {0};

	if(dtb_cksum((char *)working_fdt - 4, DTB_SIZE - 4, 0)) {
		printf("dtb chsum err! you should reload dtb file!!!\n");
		return -1;
	}
	memcpy(buff + 4, working_fdt, DTB_SIZE - 8);
retry:

	/*
	 * Parameters: Node path, property, optional value.
	 */
	if (argc < 3)
		goto warning;

	if (argc == 3) {
		len = 0;
	} else {
		ret = fdt_parse_prop(&argv[3], argc - 3, data, &len);
		if (ret != 0)
			return ret;
	}

	nodeoffset = fdt_path_offset (buff + 4, argv[1]);
	if (nodeoffset < 0) {
		/*
		 * Not found or something else bad happened.
		 */
		printf ("libfdt fdt_path_offset() returned %s\n",
			fdt_strerror(nodeoffset));
		return 1;
	}

	ret = fdt_setprop(buff + 4, nodeoffset, argv[2], data, len);
	if(ret == -FDT_ERR_NOSPACE && fdt_totalsize(working_fdt) < DTB_SIZE - 8) {
		fdt_open_into(working_fdt, buff + 4, DTB_SIZE - 8);
		goto retry;
	}else if (ret < 0) {
		printf ("libfdt fdt_setprop(): %s\n", fdt_strerror(ret));
		return 1;
	}

	dtb_cksum(buff, DTB_SIZE - 4, 1);
	tgt_flashprogram((char *)working_fdt - 4, DTB_SIZE, buff, 0);
	return 0;
warning:
	printf("set_dtb <path> <prop> [value]\n");
	printf("eg: set_dtb /chosen bootargs \"console=ttyS0,115200 root=/dev/sda1\"\n");
	printf("eg: set_dtb /memory reg <0 0x00200000 0 0x09e00000 1 0x10000000 1 0xd0000000>\n");
	printf("eg: set_dtb /soc/ethernet@0x40000000 mac-address [80 c1 80 c1 80 c1]\n");
	return 0;
}

static int mk_dtb_node(int argc,char **argv)
{
	int  nodeoffset;	/* node offset from libfdt */
	int  err;
	u8 buff[DTB_SIZE] = {0};

	if(dtb_cksum((char *)working_fdt - 4, DTB_SIZE - 4, 0)) {
		printf("dtb chsum err! you should reload dtb file!!!\n");
		return -1;
	}
	memcpy(buff + 4, working_fdt, DTB_SIZE - 8);
retry:

	/*
	 * Parameters: Node path, new node to be appended to the path.
	 */
	if (argc < 3)
		goto warning;

	nodeoffset = fdt_path_offset (buff + 4, argv[1]);
	if (nodeoffset < 0) {
		/*
		 * Not found or something else bad happened.
		 */
		printf ("libfdt fdt_path_offset() returned %s\n",
			fdt_strerror(nodeoffset));
		return 1;
	}
	err = fdt_add_subnode(buff + 4, nodeoffset, argv[2]);
	if(err == -FDT_ERR_NOSPACE && fdt_totalsize(working_fdt) < DTB_SIZE - 8) {
		fdt_open_into(working_fdt, buff + 4, DTB_SIZE - 8);
		goto retry;
	}else if (err < 0) {
		printf ("libfdt fdt_add_subnode(): %s\n",
			fdt_strerror(err));
		return 1;
	}

	dtb_cksum(buff, DTB_SIZE - 4, 1);
	tgt_flashprogram((char *)working_fdt - 4, DTB_SIZE, buff, 0);
	return 0;
warning:
	printf("mk_dtb_node <path> <node>	- Make a new node after <path>\n");
	printf("eg: mk_dtb_node /soc hda@0x400d0000\n");
	return 0;
}

static int rm_dtb_node(int argc,char **argv)
{
	int  nodeoffset;	/* node offset from libfdt */
	int  err;
	u8 buff[DTB_SIZE] = {0};

	if(argc == 1)
		goto warning;

	if(dtb_cksum((char *)working_fdt - 4, DTB_SIZE - 4, 0)) {
		printf("dtb chsum err! you should reload dtb file!!!\n");
		return -1;
	}
	memcpy(buff + 4, working_fdt, DTB_SIZE - 8);

	/*
	 * Get the path.  The root node is an oddball, the offset
	 * is zero and has no name.
	 */
	nodeoffset = fdt_path_offset (buff + 4, argv[1]);
	if (nodeoffset < 0) {
		/*
		 * Not found or something else bad happened.
		 */
		printf ("libfdt fdt_path_offset() returned %s\n", fdt_strerror(nodeoffset));
		return 1;
	}
	/*
	 * Do the delete.  A fourth parameter means delete a property,
	 * otherwise delete the node.
	 */
	if (argc > 2) {
		err = fdt_delprop(buff + 4, nodeoffset, argv[2]);
		if (err < 0) {
			printf("libfdt fdt_delprop():  %s\n", fdt_strerror(err));
			return err;
		}
	} else {
		err = fdt_del_node(buff + 4, nodeoffset);
		if (err < 0) {
			printf("libfdt fdt_del_node():  %s\n", fdt_strerror(err));
			return err;
		}
	}

	dtb_cksum(buff, DTB_SIZE - 4, 1);
	tgt_flashprogram((char *)working_fdt - 4, DTB_SIZE, buff, 0);
	return 0;
warning:
	printf("rm_dtb_node <path> [prop]	- Remove the node or property\n");
	printf("eg: rm_dtb_node /soc/hda@0x400d0000 \n");
	printf("eg: rm_dtb_node /soc dma-coherent\n");
	return 0;
}

int show_fdt_cmds(int argc,char **argv)
{
	printf("load_dtb		- Load dtb file\n");
	printf("\tload_dtb <path>\n");
	printf("\teg: load_dtb tftp://xx.xx.xx.xx/ls2k.dtb\n\n");

	printf("set_dtb			- Set dtb\n");
	printf("\tset_dtb <path> <prop> [value]\n");
	printf("\teg: set_dtb /chosen bootargs \"console=ttyS0,115200 root=/dev/sda1\"\n");
	printf("\teg: set_dtb /memory reg <0 0x00200000 0 0x09e00000 1 0x10000000 1 0xd0000000>\n");
	printf("\teg: set_dtb /soc/ethernet@0x40000000 mac-address [80 c1 80 c1 80 c1]\n\n");

	printf("print_dtb		- Print dtb\n");
	printf("\tprint_dtb <path> [prop]\n");
	printf("\teg: print_dtb head			- print dtb header\n");
	printf("\teg: print_dtb /				- print all the device three\n");
	printf("\teg: print_dtb /soc/dc@0x400c0000	- print property of dc\n");
	printf("\teg: print_dtb /soc/dc@0x400c0000 reg	- print reg's value of dc\n\n");

	printf("mk_dtb_node		- Make a new node after <path>\n");
	printf("\tmk_dtb_node <path> <node>\n");
	printf("\teg: mk_dtb_node /soc hda@0x400d0000\n\n");

	printf("rm_dtb_node		- Remove the node or [property]\n");
	printf("\trm_dtb_node <path> [prop]\n");
	printf("\teg: rm_dtb_node /soc/hda@0x400d0000 \n");
	printf("\teg: rm_dtb_node /soc dma-coherent\n\n");

	printf("erase_dtb		- Erase dtb file in flash\n");
	return 0;
}

static const Cmd Cmds[] =
{
	{"FDT"},
	{"load_dtb","file",0,"load_dtb file(max size 16K)", load_dtb,0,99,CMD_REPEAT},
	{"fdt","",0,"show all fdt cmds", show_fdt_cmds,0,99,CMD_REPEAT},
	{"set_dtb","<path> <prop> [value]",0,"set property in dtb", set_dtb,0,99,CMD_REPEAT},
	{"print_dtb","<path> [prop]",0,"print dtb at <path> [prop]", print_dtb,0,99,CMD_REPEAT},
	{"mk_dtb_node","<path> <node>",0,"Create a new node after <path> in dtb", mk_dtb_node,0,99,CMD_REPEAT},
	{"rm_dtb_node","<path> <node> [prop]",0,"Delete the node or [property] in dtb", rm_dtb_node,0,99,CMD_REPEAT},
	{"erase_dtb","",0,"erase dtb file in flash", erase_dtb,0,99,CMD_REPEAT},
	{0, 0}
};
static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
