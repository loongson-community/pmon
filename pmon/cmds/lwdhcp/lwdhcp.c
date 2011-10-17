/////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                 //	
//                                  Light weight DHCP client                                       //
//                                                                                                 //
/////////////////////////////////////////////////////////////////////////////////////////////////////


#include <sys/param.h>
#include <sys/file.h>
#include <sys/syslog.h>
#include <sys/endian.h>

#ifdef		PMON
	#ifdef		_KERNEL
	#undef		_KERNEL
		#include <sys/socket.h>
		#include <sys/ioctl.h>
		#include <netinet/in.h>
		#define _KERNEL
	#else
		#include <sys/socket.h>
		#include <sys/ioctl.h>
		#include <netinet/in.h>
	#endif

	#define	 KERNEL
	#include <pmon/netio/bootp.h>
	#include <sys/types.h>
	#include <sys/net/if.h>
#else
	#include <linux/if_packet.h>
	#include <linux/if_ether.h>
	#include <net/if.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
#endif

#include <pmon.h>
#include <setjmp.h>
#include <signal.h>

#include "lwdhcp.h"
#include "packet.h"
#include "options.h"
#include "pmon/netio/bootparams.h"

struct client_config_t  client_config;
int 	dhcp_request;
int		fd = 0;
sig_t	pre_handler = 0;

static jmp_buf  jmpb;

static void terminate()
{
	DbgPrint("Program terminated by user.\n");
	dhcp_request = 0;
	
	if(fd > 0)
		close(fd);
	
	longjmp(jmpb, 1);
}

static void	init(char *ifname)
{
	memset((void *)&client_config, 0, sizeof(struct client_config_t));

	strcpy(client_config.interface, ifname);

	pre_handler = signal(SIGINT, (sig_t)terminate);
}

int		listen_socket()
{
	int		sock, n;
	int		flag;
	int		dwValue;
	struct ifreq 	ifr;
	struct sockaddr_in		clnt;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0)
	{
		PERROR("socket create error");
		return sock;
	}

	bzero((char*)&clnt, sizeof(clnt));
	clnt.sin_family = AF_INET;
	clnt.sin_port = htons(CLIENT_PORT);
	clnt.sin_addr.s_addr = INADDR_ANY;

	if (bind (sock, (struct sockaddr *)&clnt, sizeof(struct sockaddr_in)) < 0) {
		PERROR ("bind failed");
		close (sock);
		return -1;
	}

	flag = 1;
	setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char*)&flag, sizeof(flag)); 

	n = 1;
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&n, sizeof(n));

	return sock;
}

int del_dummy_addr(char *interface)
{
	int fd;
	struct ifreq ifr;
	
	memset(&ifr, 0, sizeof(struct ifreq));
	if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0)
	{
		ifr.ifr_addr.sa_family = AF_INET;

		strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);
		(void)ioctl(fd, SIOCDIFADDR, &ifr);
	}
	else{
		PERROR("socket failed!\n");
		return -1;
	}
	close(fd);
	return 0;
}
int read_interface(char *interface, int *ifindex, uint32_t *addr, uint8_t *arp)
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *our_ip;

	memset(&ifr, 0, sizeof(struct ifreq));
	if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) 
	{
		ifr.ifr_addr.sa_family = AF_INET;

		strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);

		if (addr) 
		{
			if (ioctl(fd, SIOCAIFADDR, &ifr) == 0) 
			{
				our_ip = (struct sockaddr_in *) &ifr.ifr_addr;
				*addr = our_ip->sin_addr.s_addr;
				DbgPrint("%s (our ip) = %s\n", ifr.ifr_name, inet_ntoa(our_ip->sin_addr));
			} else 
			{
				PERROR("SIOCAIFADDR failed, is the interface up and configured?\n");
				close(fd);
				return -1;
			}
		}

		//the following code needs to change the PMON source code pmon/pmon/netio/bootp.c line 101
		//int
		//getethaddr (unsigned char *eaddr, char *ifc)
		//{
		//    struct ifnet *ifp;
		//        struct ifaddr *ifa;
		// ......
#ifndef		PMON
		if(ioctl(fd, SIOCGIFINDEX, &ifr) == 0) 
		{
			DbgPrint("adapter index %d\n", ifr.ifr_ifindex);
			*ifindex = ifr.ifr_ifindex;
		} else 
		{
			PERROR("SIOCGIFINDEX failed!\n");
			close(fd);
			return -1;
		}
#endif

#ifndef		PMON
		if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) 
		{
			memcpy(arp, ifr.ifr_hwaddr.sa_data, 6);
#else
		if(getethaddr(arp, client_config.interface) >= 0)
		{
#endif
			DbgPrint("adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x\n",
					arp[0], arp[1], arp[2], arp[3], arp[4], arp[5]);
		} 
		else{
			PERROR("SIOCGIFHWADDR failed!\n");
			close(fd);
			return -1;
		}
	} 
	else {
		PERROR("socket failed!\n");
		return -1;
	}
	close(fd);
	return 0;
}

extern char dhcp_c_ip_string[15];
extern char dhcp_s_ip_string[15];
extern char dhcp_sname[65];
extern char dhcp_file[129];
extern char dhcp_host_name[309];
extern char dhcp_root_path[309];
int mylwdhcp(struct bootparams *bootp, char *ifname)
{
	int						xid;
	fd_set					fs;
	int						ret, totimes;
	char					buf[1500];
	struct sockaddr_in		from;
	int						size = sizeof(from);
	struct dhcp_packet*		p;
	uint8_t*				dhcp_message_type;
	struct	timeval			tv;
	static struct in_addr nmask;
	long tmo, loop;
#define JTLOOP 20 /* loop test*/
#define MAXTMO 20 /* seconds*/
#define MINTMO 20 /* seconds*/

	if(getuid())
	{
		DbgPrint("Only root can run this program!\n");
		return 0;
	}

	DbgPrint("Light weight DHCP client starts...\n");
	init(ifname);

	if(setjmp(jmpb))
	{
		signal(SIGINT, pre_handler);
		sigsetmask(0);
		return 0;
	}

	
	if(read_interface(client_config.interface, &client_config.ifindex,
				&client_config.addr, client_config.arp) < 0)
	{
		DbgPrint("read_interface error");
		return 0;
	}

	if((fd = listen_socket()) < 0)
	{
		return 0;
	}

	//srand(time(NULL));
	srand(client_config.arp[5]);
	xid = rand();
	DbgPrint("xid = %d\n", xid);
	totimes = 3;
	tmo = MINTMO;
	loop = 0;
	
tryagain:
	if(send_discover(xid) < 0)
		if(--totimes > 0)
			goto tryagain;
		else
		{
			DbgPrint("Fail to send DHCPDISCOVER...\n");
			return 0;
		}

	FD_ZERO(&fs);
	FD_SET(fd, &fs);
	tv.tv_sec = tmo;
	tv.tv_usec = 0;
	//receiving DHCPOFFER
	while(1)
	{
		dhcp_request = 1;
		ret = select(fd + 1, &fs, NULL, NULL, &tv);
		
		if(ret == -1)
		{
			PERROR("select error");
			dhcp_request = 0;
			close(fd);
  			return 0;
		}
		else if(ret == 0)
		{
			tmo <<= 1;
			if(tmo >= MAXTMO)
			{
				if(loop++ < JTLOOP)
					tmo = MINTMO;
				else
				{
					dhcp_request = 0;
					DbgPrint("Fail to get IP from DHCP server, it seems that there is no DHCP server.\n");
					close(fd);
					return 0;
				}
			}
			goto tryagain;
		}

		size = sizeof(from);
		if(recvfrom(fd, buf, sizeof(struct dhcp_packet), (struct sockaddr *)0, &from, &size) < 0)
			continue;
		
		dhcp_request = 0;

		p = (struct dhcp_packet *)buf;

		if(p->xid != xid)
			continue;

		dhcp_message_type = get_dhcp_option(p, DHCP_MESSAGE_TYPE);
		if(!dhcp_message_type)
			continue;
		else if(*dhcp_message_type != DHCPOFFER)
			continue;

		DbgPrint("DHCPOFFER received...\n");
		DbgPrint("IP %s obtained from the DHCP server.\n", inet_ntoa(p->yiaddr));
		break;
	}

	//sending DHCPREQUEST
	send_request(xid, *((uint32_t*)(&(p->siaddr))), *((uint32_t*)(&p->yiaddr)));

	tv.tv_sec = 3;
	tv.tv_usec = 0;
	//receiving DHCPACK
	while(1)
	{
		dhcp_request = 1;
		ret = select(fd + 1, &fs, NULL, NULL, &tv);
		
		if(ret == -1)
		{
			PERROR("select error");
			dhcp_request = 0;
			close(fd);
			return 0;
		}
		else if(ret == 0)
		{
			tmo <<= 1;
			if(tmo >= MAXTMO)
			{	
				if(loop++ < JTLOOP)
					tmo = MINTMO;
				else
				{
					dhcp_request = 0;
					DbgPrint("Fail to get IP from DHCP server, no ACK from DHCP server.\n");
					close(fd);
					return 0;
				}	
			}
			goto tryagain;
		}

		//get_raw_packet(buf, fd);
		
		size = sizeof(struct sockaddr);
		recvfrom(fd, buf, sizeof(struct dhcp_packet), 0, (struct sockaddr*)&from, &size);

		dhcp_request = 0;
	
		p = (struct dhcp_packet *)buf;	
		
		if(p->xid != xid)
			continue;

		dhcp_message_type = get_dhcp_option(p, DHCP_MESSAGE_TYPE);
		if(!dhcp_message_type)
			continue;
		else if(*dhcp_message_type != DHCPACK)
			continue;

		DbgPrint("DHCPACK received...\n");
		DbgPrint("IP %s obtained from the DHCP server.\n", inet_ntoa(p->yiaddr));
		sprintf(dhcp_c_ip_string, "%s", inet_ntoa(p->yiaddr));
		sprintf(dhcp_s_ip_string, "%s", inet_ntoa(p->siaddr));
		sprintf(dhcp_sname, "%s", p->sname);
		sprintf(dhcp_file, "%s", p->file);
		if(p->yiaddr.s_addr != 0 && (bootp->have & BOOT_ADDR) == 0)
		{
			u_long addr;
			bootp->have |= BOOT_ADDR;
			bootp->addr = p->yiaddr;
			printf("our ip address is %s\n", inet_ntoa(bootp->addr));
			
			addr = ntohl(bootp->addr.s_addr);
			addr = (bootp->addr.s_addr);
			if(IN_CLASSA(addr))
				nmask.s_addr = htonl(IN_CLASSA_NET);
			else if(IN_CLASSB(addr))
				nmask.s_addr = htonl(IN_CLASSB_NET);
			else 
				nmask.s_addr = htonl(IN_CLASSC_NET);
			printf("'native netmask' is %s\n", inet_ntoa(nmask));
		}
		if(p->siaddr.s_addr !=0 && (bootp->have & BOOT_BOOTIP) == 0)
		{
			bootp->have |= BOOT_BOOTIP;
            		bootp->bootip = p->siaddr;
		}
		if (p->file[0] != 0 && (bootp->have & BOOT_BOOTFILE) == 0) 
		{
            		bootp->have |= BOOT_BOOTFILE;
            		strncpy (bootp->bootfile, p->file, sizeof (p->file));
        	}
		if ((bootp->have & BOOT_MASK) != 0 && (nmask.s_addr & bootp->mask.s_addr) != nmask.s_addr) 
		{
            		printf("subnet mask (%s) bad\n", inet_ntoa(bootp->mask));
            		bootp->have &= ~BOOT_MASK;
        	}
		/* Get subnet (or natural net) mask */
       	 	if ((bootp->have & BOOT_MASK) == 0)
            	bootp->mask = nmask;
		dhcp_message_type = get_dhcp_option(p, DHCP_HOST_NAME);
		if(dhcp_message_type)
        	{
            		int size = dhcp_message_type[-1];
            		strncpy(dhcp_host_name, dhcp_message_type, size);
            		dhcp_host_name[size] = '\0';
            		if (size < sizeof (bootp->hostname)&& (bootp->have & BOOT_HOSTNAME) == 0) 
			{
                		bootp->have |= BOOT_HOSTNAME;
                		strcpy(bootp->hostname, dhcp_host_name);
            		}
        	}
		dhcp_message_type = get_dhcp_option(p, DHCP_ROOT_PATH);
		if(dhcp_message_type)
        	{
            		strncpy(dhcp_root_path, dhcp_message_type, dhcp_message_type[-1]);
            		dhcp_root_path[dhcp_message_type[-1]] = '\0';
        	}

        	DbgPrint("dhcp_c_ip_string = %s\n", dhcp_c_ip_string);
        	DbgPrint("dhcp_s_ip_string = %s\n", dhcp_s_ip_string);
//        	DbgPrint("dhcp_sname = %s\n", dhcp_sname);
//        	DbgPrint("dhcp_file = %s\n", dhcp_file);
//        	DbgPrint("host name = %s\n", dhcp_host_name);
//        	DbgPrint("root-path = %s\n", dhcp_root_path);				
		break;
	}

	close(fd);
	del_dummy_addr(client_config.interface);

	return 0;
}

int lwdhcp(int argc, char *argv[])
{
	struct bootparams bootp;
#ifdef DHCP_3A780E
	char *ifname = "rte0";
#elif defined DHCP_3ASERVER
	char *ifname = "em0";
#endif
	memset(&bootp, 0, sizeof(struct bootparams));
	
	return mylwdhcp(&bootp, ifname);
}

/*
 *  Command table registration
 *  ==========================
 */

static const Cmd Cmds[] =
{
	{"Network"},
	{"lwdhcp",    "", 0,
		"Light weight DHCP client",
		lwdhcp, 1, 99, CMD_REPEAT},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));

	static void
init_cmd()
{
	cmdlist_expand(Cmds, 1);
}





