#undef  _KERNEL

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/syslog.h>
#include <sys/endian.h>

#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <file.h>
#include <net/if.h>
#include <pmon.h>
#include "nfs.h"
#include "netio.h"

//#if defined(CONFIG_CMD_NET) && defined(CONFIG_CMD_NFS)

#define HASHES_PER_LINE 65	/* Number of "loading" hashes per line	*/
#define NFS_RETRY_COUNT 30
#define NFS_TIMEOUT 2000UL

typedef ulong IPaddr_t;
typedef unsigned char uchar;

void ip_to_string(IPaddr_t x, char *s);
IPaddr_t string_to_ip(char *s);
void print_IPaddr(IPaddr_t x);
static char* basename (char *);
static char* dirname (char *);
extern void log __P((int kind, const char *fmt, ...));
extern in_addr_t inet_addr __P((const char *));
static void NfsSend (void);
static void NfsHandler (uchar *pkt, void *buf, /*unsigned dest, unsigned src*/ unsigned len); 
//unsigned long simple_strtoul(const char *, char **, unsigned int);
static int
store_block (uchar *, void *, unsigned, unsigned);

static int fs_mounted = 0;
static unsigned long rpc_id = 0;
static int nfs_offset = -1;
static int nfs_len;

static char dirfh[NFS_FHSIZE];	/* file handle of directory */
static char filefh[NFS_FHSIZE]; /* file handle of kernel image */

static int	NfsDownloadState;
static unsigned long NfsServerIP;
static int	NfsSrvMountPort;
static int	NfsSrvNfsPort;
static int	NfsOurPort;
static int	NfsTimeoutCount;
static int	NfsState;

#define STATE_RPCLOOKUP_PROG_MOUNT_REQ	1
#define STATE_RPCLOOKUP_PROG_NFS_REQ	2
#define STATE_MOUNT_REQ			3
#define STATE_UMOUNT_REQ		4
#define STATE_LOOKUP_REQ		5
#define STATE_READ_REQ			6
#define STATE_READLINK_REQ		7

static char default_filename[64];
static char *nfs_filename;
static char *nfs_path;
static char nfs_path_buff[2048];

int NetState;

static    struct sockaddr_in sin;
static    int     sock;
static    short   oflags;

static    int     sock;
static    int     foffs;

static int nfs_open (int , struct Url *, int, int);
static int nfs_read (int, void *, int);
static int nfs_write (int, const void *, int);
static off_t nfs_lseek (int , long, int);
static int nfs_close(int);


//static struct nfsfile *nfsp;
int damn() {
    printf("%s\n", "Oh~Come On! For the love of God!");
}

static int nfs_open(int fd, struct Url *url, int flags, int perms) {
    struct hostent *hp;
    const char *mode;
    char *host;
    char hbuf[MAXHOSTNAMELEN];
    struct rpc_t pktbuf;

    nfs_path = (char *)nfs_path_buff;

    oflags = flags & O_ACCMODE;
    if(oflags != O_RDONLY && oflags != O_WRONLY) {
        errno = EACCES;
        return -1;
    }

    if(strlen(url->hostname) != 0) {
        host = url->hostname;
    } else {
            log(LOG_INFO, "nfs: missing/bad host name: %s\n", url->filename);
            errno = EDESTADDRREQ;
            return -1;
    }
    
//    fp = (struct nfsfile *)malloc(sizeof(struct nfsfile));
 //   if(!fp) {
  //      errno = ENOBUFS;
   //     return -1;
  //  }
    
    NfsServerIP = string_to_ip(host);
    strcpy(nfs_path, url->filename);
    nfs_filename = basename(nfs_path);
    nfs_path = dirname(nfs_path);

    puts("File transfer via NFS from server ");
    print_IPaddr(NfsServerIP);
    printf("nfs_filename : %s\n", nfs_filename);
    printf("nfs_path : %s\n", nfs_path);
//    puts("; out IP address is ");
 //   print_IPaddr(NetOurIP);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) goto error;

    sin.sin_family = AF_INET;
    sin.sin_port = htons(1000);
    if(bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        goto error;
    hp = gethostbyname(host);
    if(hp) {
        sin.sin_family = hp->h_addrtype;
        bcopy(hp->h_addr, (void *)&sin.sin_addr, hp->h_length);
        strncpy(hbuf, hp->h_name, sizeof(hbuf)-1);
        hbuf[sizeof(hbuf)-1] = 0;
                host = hbuf;
    } else {
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = inet_addr(host);
        if(sin.sin_addr.s_addr == -1) {
            log(LOG_INFO, "nfs: bad internet address: %s\n", host);
            errno = EADDRNOTAVAIL;
            goto error;
        }
    }
   // sin.sin_port = SUNRPC_PORT;

    NfsState = STATE_RPCLOOKUP_PROG_MOUNT_REQ;
    NfsSend();
    while(NfsState != STATE_READ_REQ  || NfsState == STATE_UMOUNT_REQ) {
        NfsHandler((uchar *)&pktbuf, NULL, sizeof(struct rpc_t));
    }

    if(NfsState == STATE_READ_REQ)
        return 0;

error:
    if(sock >= 0)
        close(sock);
   // free(fp);
    return -1;
}

static int nfs_close(int fd) {
	return 0;
}

static int start, end, off;
struct rpc_t readbuf;

static int nfs_read_reply (uchar *,void *, unsigned int);

static int nfs_read(int fd, void *buf, int nread) {
//    nfs_len = n;
//    printf("%d bytes requested....\n", nfs_len);
/*
    NfsSend();
    while(NfsState != STATE_UMOUNT_REQ) {
        NfsHandler((uchar *)&pktbuf, buf, n);
    }
    return n;
    */
    int nb, n;
    start = off = 0;
    NfsSend();

    end = nfs_read_reply((uchar *)&readbuf, buf, n);
    nfs_offset += end;
    for(nb = nread; nb != 0 && off >= start && off < end; ) {
	if(off >= start && off < end) {
		n = end - off;
		if(n > nb) n = nb;
		store_block ((uchar *)&readbuf+sizeof(readbuf.u.reply), buf, off, n);
		off += n;
		nb -= n;
	}
	if(off >= end) {
		NfsSend();
		n = nfs_read_reply ((uchar *)&readbuf, buf, n);
		if(n > 0) nfs_offset += n;
		start = end;
		end = start + n;
	}
    }
    printf("nfs_read %d\n", nread-nb);
    return nread - nb;
}

static int nfs_write(int fd, const void *buf, int n) {
	return -1;
}
    
static off_t nfs_lseek(int fd, long offset, int whence) {
    printf("nfs_lseek...offset = %d\n", offset);
    return (nfs_offset = offset);
}

static NetFileOps nfsops = {
    "nfs", 
    nfs_open,
    nfs_read,
    nfs_write,
    nfs_lseek,
    NULL,
    nfs_close,
};

static void init_nfs __P((void)) __attribute__ ((constructor));

static void init_nfs(void) {
    netfs_init(&nfsops);
}

static int
store_block (uchar * src, void *dest, unsigned offset, unsigned len)
{
/*	ulong newsize = offset + len;
#ifdef CONFIG_SYS_DIRECT_FLASH_NFS
	int i, rc = 0;

	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		if (load_addr + offset >= flash_info[i].start[0]) {
			rc = 1;
			break;
		}
	}

	if (rc) { 
		rc = flash_write ((uchar *)src, (ulong)(load_addr+offset), len);
		if (rc) {
			flash_perror (rc);
			return -1;
		}
	} else

	{
		(void)memcpy ((void *)(load_addr + offset), src, len);
	}

	if (NetBootFileXferSize < (offset+len))
		NetBootFileXferSize = newsize; */
 //   printf("len = %d\n", len);
    bcopy(src, dest+offset, len);
//    printf("buf addr = 0x%x\n", dest+offset);
	return 0;
}

static char*
basename (char *path)
{
	char *fname;

	fname = path + strlen(path) - 1;
	while (fname >= path) {
		if (*fname == '/') {
			fname++;
			break;
		}
		fname--;
	}
	return fname;
}

static char*
dirname (char *path)
{
	char *fname;

	fname = basename (path);
	--fname;
	*fname = '\0';
	return path;
}

/**************************************************************************
RPC_ADD_CREDENTIALS - Add RPC authentication/verifier entries
**************************************************************************/
static long *rpc_add_credentials (long *p)
{
	int hl;
	int hostnamelen;
	char hostname[256];

	strcpy (hostname, "");
	hostnamelen=strlen (hostname);

	/* Here's the executive summary on authentication requirements of the
	 * various NFS server implementations:	Linux accepts both AUTH_NONE
	 * and AUTH_UNIX authentication (also accepts an empty hostname field
	 * in the AUTH_UNIX scheme).  *BSD refuses AUTH_NONE, but accepts
	 * AUTH_UNIX (also accepts an empty hostname field in the AUTH_UNIX
	 * scheme).  To be safe, use AUTH_UNIX and pass the hostname if we have
	 * it (if the BOOTP/DHCP reply didn't give one, just use an empty
	 * hostname).  */

	hl = (hostnamelen + 3) & ~3;

	/* Provide an AUTH_UNIX credential.  */
	*p++ = htonl(1);		/* AUTH_UNIX */
	*p++ = htonl(hl+20);		/* auth length */
	*p++ = htonl(0);		/* stamp */
	*p++ = htonl(hostnamelen);	/* hostname string */
	if (hostnamelen & 3) {
		*(p + hostnamelen / 4) = 0; /* add zero padding */
	}
	memcpy (p, hostname, hostnamelen);
	p += hl / 4;
	*p++ = 0;			/* uid */
	*p++ = 0;			/* gid */
	*p++ = 0;			/* auxiliary gid list */

	/* Provide an AUTH_NONE verifier.  */
	*p++ = 0;			/* AUTH_NONE */
	*p++ = 0;			/* auth length */

	return p;
}

/**************************************************************************
RPC_LOOKUP - Lookup RPC Port numbers
**************************************************************************/
static void
rpc_req (int rpc_prog, int rpc_proc, uint32_t *data, int datalen)
{
	struct rpc_t pkt;
	unsigned long id;
	uint32_t *p;
	int pktlen;
	int sport;

    int n = 0;

	id = ++rpc_id;
	pkt.u.call.id = htonl(id);
	pkt.u.call.type = htonl(MSG_CALL);
	pkt.u.call.rpcvers = htonl(2);	/* use RPC version 2 */
	pkt.u.call.prog = htonl(rpc_prog);
	pkt.u.call.vers = htonl(2);	/* portmapper is version 2 */
	pkt.u.call.proc = htonl(rpc_proc);
	p = (uint32_t *)&(pkt.u.call.data);

	if (datalen)
		memcpy ((char *)p, (char *)data, datalen*sizeof(uint32_t));

	pktlen = (char *)p + datalen*sizeof(uint32_t) - (char *)&pkt;

//	memcpy ((char *)NetTxPacket + NetEthHdrSize() + IP_HDR_SIZE, (char *)&pkt, pktlen);

	if (rpc_prog == PROG_PORTMAP)
		sport = SUNRPC_PORT;
	else if (rpc_prog == PROG_MOUNT)
		sport = NfsSrvMountPort;
	else
		sport = NfsSrvNfsPort;

    sin.sin_port = htons(sport);
//	NetSendUDPPacket (NetServerEther, NfsServerIP, sport, NfsOurPort, pktlen);
    n = sendto(sock, &pkt, pktlen, 0, (struct sockaddr *)&sin, sizeof(sin));
//    printf("In rpc_req -- %d bytes have been sent ...\n", n);
}

/**************************************************************************
RPC_LOOKUP - Lookup RPC Port numbers
**************************************************************************/
static void
rpc_lookup_req (int prog, int ver)
{
	uint32_t data[16];

	data[0] = 0; data[1] = 0;	/* auth credential */
	data[2] = 0; data[3] = 0;	/* auth verifier */
	data[4] = htonl(prog);
	data[5] = htonl(ver);
	data[6] = htonl(17);	/* IP_UDP */
	data[7] = 0;

	rpc_req (PROG_PORTMAP, PORTMAP_GETPORT, data, 8);
}

/**************************************************************************
NFS_MOUNT - Mount an NFS Filesystem
**************************************************************************/
static void
nfs_mount_req (char *path)
{
	uint32_t data[1024];
	uint32_t *p;
	int len;
	int pathlen;

	pathlen = strlen (path);

	p = &(data[0]);
	p = (uint32_t *)rpc_add_credentials((long *)p);

	*p++ = htonl(pathlen);
	if (pathlen & 3) *(p + pathlen / 4) = 0;
	memcpy (p, path, pathlen);
	p += (pathlen + 3) / 4;

	len = (uint32_t *)p - (uint32_t *)&(data[0]);

	rpc_req (PROG_MOUNT, MOUNT_ADDENTRY, data, len);
}

/**************************************************************************
NFS_UMOUNTALL - Unmount all our NFS Filesystems on the Server
**************************************************************************/
static void
nfs_umountall_req (void)
{
	uint32_t data[1024];
	uint32_t *p;
	int len;

	if ((NfsSrvMountPort == -1) || (!fs_mounted)) {
		/* Nothing mounted, nothing to umount */
		return;
	}

	p = &(data[0]);
	p = (uint32_t *)rpc_add_credentials ((long *)p);

	len = (uint32_t *)p - (uint32_t *)&(data[0]);

	rpc_req (PROG_MOUNT, MOUNT_UMOUNTALL, data, len);
}

/***************************************************************************
 * NFS_READLINK (AH 2003-07-14)
 * This procedure is called when read of the first block fails -
 * this probably happens when it's a directory or a symlink
 * In case of successful readlink(), the dirname is manipulated,
 * so that inside the nfs() function a recursion can be done.
 **************************************************************************/
static void
nfs_readlink_req (void)
{
	uint32_t data[1024];
	uint32_t *p;
	int len;

	p = &(data[0]);
	p = (uint32_t *)rpc_add_credentials ((long *)p);

	memcpy (p, filefh, NFS_FHSIZE);
	p += (NFS_FHSIZE / 4);

	len = (uint32_t *)p - (uint32_t *)&(data[0]);

	rpc_req (PROG_NFS, NFS_READLINK, data, len);
}

/**************************************************************************
NFS_LOOKUP - Lookup Pathname
**************************************************************************/
static void
nfs_lookup_req (char *fname)
{
	uint32_t data[1024];
	uint32_t *p;
	int len;
	int fnamelen;

	fnamelen = strlen (fname);

	p = &(data[0]);
	p = (uint32_t *)rpc_add_credentials ((long *)p);

	memcpy (p, dirfh, NFS_FHSIZE);
	p += (NFS_FHSIZE / 4);
	*p++ = htonl(fnamelen);
	if (fnamelen & 3) *(p + fnamelen / 4) = 0;
	memcpy (p, fname, fnamelen);
	p += (fnamelen + 3) / 4;

	len = (uint32_t *)p - (uint32_t *)&(data[0]);

	rpc_req (PROG_NFS, NFS_LOOKUP, data, len);
}

/**************************************************************************
NFS_READ - Read File on NFS Server
**************************************************************************/
static void
nfs_read_req (int offset, int readlen)
{
	uint32_t data[1024];
	uint32_t *p;
	int len;

	p = &(data[0]);
	p = (uint32_t *)rpc_add_credentials ((long *)p);

	memcpy (p, filefh, NFS_FHSIZE);
	p += (NFS_FHSIZE / 4);
	*p++ = htonl(offset);
	*p++ = htonl(readlen);
	*p++ = 0;

	len = (uint32_t *)p - (uint32_t *)&(data[0]);

	rpc_req (PROG_NFS, NFS_READ, data, len);
}

/**************************************************************************
RPC request dispatcher
**************************************************************************/

static void
NfsSend (void)
{
#ifdef NFS_DEBUG
	printf ("%s\n", __FUNCTION__);
#endif

	switch (NfsState) {
	case STATE_RPCLOOKUP_PROG_MOUNT_REQ:
		rpc_lookup_req (PROG_MOUNT, 1);
		break;
	case STATE_RPCLOOKUP_PROG_NFS_REQ:
		rpc_lookup_req (PROG_NFS, 2);
		break;
	case STATE_MOUNT_REQ:
		nfs_mount_req (nfs_path);
		break;
	case STATE_UMOUNT_REQ:
		nfs_umountall_req ();
		break;
	case STATE_LOOKUP_REQ:
		nfs_lookup_req (nfs_filename);
		break;
	case STATE_READ_REQ:
		nfs_read_req (nfs_offset, nfs_len);
		break;
	case STATE_READLINK_REQ:
		nfs_readlink_req ();
		break;
	}
}

/**************************************************************************
Handlers for the reply from server
**************************************************************************/

static int
rpc_lookup_reply (int prog, uchar *pkt, unsigned len)
{
	struct rpc_t rpc_pkt;

	memcpy ((unsigned char *)&rpc_pkt, pkt, len);

#ifdef NFS_DEBUG
	printf ("%s\n", __FUNCTION__);
#endif

	if (ntohl(rpc_pkt.u.reply.id) != rpc_id)
		return -1;

	if (rpc_pkt.u.reply.rstatus  ||
	    rpc_pkt.u.reply.verifier ||
	    rpc_pkt.u.reply.astatus) {
		return -1;
	}

	switch (prog) {
	case PROG_MOUNT:
		NfsSrvMountPort = ntohl(rpc_pkt.u.reply.data[0]);
        printf("PROG_MOUNT -- NfsSrvMountPort = %d\n", NfsSrvMountPort);
		break;
	case PROG_NFS:
		NfsSrvNfsPort = ntohl(rpc_pkt.u.reply.data[0]);
        printf("PROG_NFS -- NfsSrvNfsPort = %d\n", NfsSrvNfsPort);
		break;
	}

	return 0;
}

static int
nfs_mount_reply (uchar *pkt, unsigned len)
{
	struct rpc_t rpc_pkt;

#ifdef NFS_DEBUG
	printf ("%s\n", __FUNCTION__);
#endif

	memcpy ((unsigned char *)&rpc_pkt, pkt, len);

	if (ntohl(rpc_pkt.u.reply.id) != rpc_id)
		return -1;

	if (rpc_pkt.u.reply.rstatus  ||
	    rpc_pkt.u.reply.verifier ||
	    rpc_pkt.u.reply.astatus  ||
	    rpc_pkt.u.reply.data[0]) {
		return -1;
	}

	fs_mounted = 1;
	memcpy (dirfh, rpc_pkt.u.reply.data + 1, NFS_FHSIZE);

	return 0;
}

static int
nfs_umountall_reply (uchar *pkt, unsigned len)
{
	struct rpc_t rpc_pkt;

#ifdef NFS_DEBUG
	printf ("%s\n", __FUNCTION__);
#endif

	memcpy ((unsigned char *)&rpc_pkt, pkt, len);

	if (ntohl(rpc_pkt.u.reply.id) != rpc_id)
		return -1;

	if (rpc_pkt.u.reply.rstatus  ||
	    rpc_pkt.u.reply.verifier ||
	    rpc_pkt.u.reply.astatus) {
		return -1;
	}

	fs_mounted = 0;
	memset (dirfh, 0, sizeof(dirfh));

	return 0;
}

static int
nfs_lookup_reply (uchar *pkt, unsigned len)
{
	struct rpc_t rpc_pkt;

#ifdef NFS_DEBUG
	printf ("%s\n", __FUNCTION__);
#endif

	memcpy ((unsigned char *)&rpc_pkt, pkt, len);

	if (ntohl(rpc_pkt.u.reply.id) != rpc_id)
		return -1;

	if (rpc_pkt.u.reply.rstatus  ||
	    rpc_pkt.u.reply.verifier ||
	    rpc_pkt.u.reply.astatus  ||
	    rpc_pkt.u.reply.data[0]) {
		return -1;
	}

	memcpy (filefh, rpc_pkt.u.reply.data + 1, NFS_FHSIZE);

	return 0;
}

static int
nfs_readlink_reply (uchar *pkt, unsigned len)
{
	struct rpc_t rpc_pkt;
	int rlen;

#ifdef NFS_DEBUG
	printf ("%s\n", __FUNCTION__);
#endif

	memcpy ((unsigned char *)&rpc_pkt, pkt, len);

	if (ntohl(rpc_pkt.u.reply.id) != rpc_id)
		return -1;

	if (rpc_pkt.u.reply.rstatus  ||
	    rpc_pkt.u.reply.verifier ||
	    rpc_pkt.u.reply.astatus  ||
	    rpc_pkt.u.reply.data[0]) {
		return -1;
	}

	rlen = ntohl (rpc_pkt.u.reply.data[1]); /* new path length */

	if (*((char *)&(rpc_pkt.u.reply.data[2])) != '/') {
		int pathlen;
		strcat (nfs_path, "/");
		pathlen = strlen(nfs_path);
		memcpy (nfs_path+pathlen, (uchar *)&(rpc_pkt.u.reply.data[2]), rlen);
		nfs_path[pathlen+rlen+1] = 0;
	} else {
		memcpy (nfs_path, (uchar *)&(rpc_pkt.u.reply.data[2]), rlen);
		nfs_path[rlen] = 0;
	}
	return 0;
}

static int
nfs_read_reply (uchar *pkt, void *buf, unsigned len)
{
	int rlen;
	
	struct sockaddr_in from;
	int fromlen, n;
	struct rpc_t pktbuf;

#ifdef NFS_DEBUG_nop
	printf ("%s\n", __FUNCTION__);
#endif

	n = recvfrom(sock, &pktbuf, sizeof(struct rpc_t), 0, (struct sockaddr *)&from, &fromlen);
	memcpy(pkt, &pktbuf, sizeof(struct rpc_t));

	pkt = (struct rpc_t *)pkt;
	
	if (ntohl(pktbuf.u.reply.id) != rpc_id)
		return -1;

	if (pktbuf.u.reply.rstatus  ||
	    pktbuf.u.reply.verifier ||
	    pktbuf.u.reply.astatus  ||
	    pktbuf.u.reply.data[0]) {
		if (pktbuf.u.reply.rstatus) {
			return -9999;
		}
		if (pktbuf.u.reply.astatus) {
			return -9999;
		}
		return -ntohl(pktbuf.u.reply.data[0]);;
	}

	if ((nfs_offset!=0) && !((nfs_offset) % (NFS_READ_SIZE/2*10*HASHES_PER_LINE))) {
		puts ("\n");
	}
	if (!(nfs_offset % ((NFS_READ_SIZE/2)*10))) {
		putchar ('#');
	}

	rlen = ntohl(pktbuf.u.reply.data[18]);
//    printf("%d bytes have been read from nfs server...\n", rlen);
//	if ( store_block ((uchar *)pkt+sizeof(rpc_pkt.u.reply), buf, nfs_offset, rlen) )
//		return -9999;

	return rlen;
}

static void
NfsHandler (uchar *pkt, void * buf, /*unsigned dest, unsigned src,*/ unsigned len)
{
	int rlen;
    struct sockaddr_in from;
    int fromlen;
    int n;

#ifdef NFS_DEBUG
	printf ("%s\n", __FUNCTION__);
#endif

//    n = recvfrom(sock, pkt, len, 0,
//            (struct sockaddr *)&from, &fromlen);
    n = recvfrom(sock, pkt, sizeof(struct rpc_t), 0, (struct sockaddr *)&from, &fromlen);

//    printf("In NfsHandler -- %d bytes have been received...\n", n);

    if(n < 0) {
        perror("nfs: recvfrom");
        return;
    }
//	if (dest != NfsOurPort) return;

	switch (NfsState) {
	case STATE_RPCLOOKUP_PROG_MOUNT_REQ:
		rpc_lookup_reply (PROG_MOUNT, pkt, n);
		NfsState = STATE_RPCLOOKUP_PROG_NFS_REQ;
		NfsSend ();
		break;

	case STATE_RPCLOOKUP_PROG_NFS_REQ:
		rpc_lookup_reply (PROG_NFS, pkt, n);
		NfsState = STATE_MOUNT_REQ;
		NfsSend ();
		break;

	case STATE_MOUNT_REQ:
		if (nfs_mount_reply(pkt, n)) {
			puts ("*** ERROR: Cannot mount\n");
			/* just to be sure... */
			NfsState = STATE_UMOUNT_REQ;
			NfsSend ();
		} else {
			NfsState = STATE_LOOKUP_REQ;
			NfsSend ();
		}
		break;

	case STATE_UMOUNT_REQ:
		if (nfs_umountall_reply(pkt, n)) {
			puts ("*** ERROR: Cannot umount\n");
//			NetState = NETLOOP_FAIL;
		} else {
			puts ("\ndone\n");
			NetState = NfsDownloadState;
		}
		break;

	case STATE_LOOKUP_REQ:
		if (nfs_lookup_reply(pkt, n)) {
			puts ("*** ERROR: File lookup fail\n");
			NfsState = STATE_UMOUNT_REQ;
			NfsSend ();
		} else {
			NfsState = STATE_READ_REQ;
			nfs_offset = 0;
			nfs_len = NFS_READ_SIZE;
		//	NfsSend ();
		}
		break;

	case STATE_READLINK_REQ:
		if (nfs_readlink_reply(pkt, n)) {
			puts ("*** ERROR: Symlink fail\n");
			NfsState = STATE_UMOUNT_REQ;
			NfsSend ();
		} else {
#ifdef NFS_DEBUG
			printf ("Symlink --> %s\n", nfs_path);
#endif
			nfs_filename = basename (nfs_path);
			nfs_path     = dirname (nfs_path);

			NfsState = STATE_MOUNT_REQ;
			NfsSend ();
		}
		break;

	case STATE_READ_REQ:
		rlen = nfs_read_reply (pkt, buf, n);
	//	NetSetTimeout (NFS_TIMEOUT, NfsTimeout);
		if (rlen > 0) {
			nfs_offset += rlen;
//			NfsSend ();
		}
		else if ((rlen == -NFSERR_ISDIR)||(rlen == -NFSERR_INVAL)) {
			/* symbolic link */
			NfsState = STATE_READLINK_REQ;
			NfsSend ();
		} else {
			if ( ! rlen ) //NfsDownloadState = NETLOOP_SUCCESS;
			NfsState = STATE_UMOUNT_REQ;
//			NfsSend ();
		}
		break;
	}
}

void ip_to_string(IPaddr_t x, char *s) {
    x = ntohl(x);
    sprintf(s, "%d.%d.%d.%d",
            (int)((x >> 24) & 0xff),
            (int)((x >> 16) & 0xff),
            (int)((x >> 8) & 0xff), (int)((x >> 0) & 0xff)
           );
}

IPaddr_t string_to_ip(char *s) {
    IPaddr_t addr;
    char *e;
    int i;
    
    if(s == NULL) return 0;

    for(addr = 0, i =0; i < 4; ++i) {
        ulong val = s ? strtoul(s, &e, 10) : 0;
        addr <<= 8;
        addr |= (val & 0xFF);
        if(s)
            s = (*e) ? e+1 : e;
    }
    return htonl(addr);
}

void print_IPaddr(IPaddr_t x) {
    char tmp[16];
    ip_to_string(x, tmp);
    puts(tmp);
}

/*
unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base) {
    unsigned long result = 0, value;

    if(*cp == '0') {
        cp++;
        if((*cp == 'x') && isxdigit(cp[1])) {
            base = 16;
            cp++;
        }
        if(!base) base = 8;
    }
    if(!base) base = 10;
    while(isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
                    ? toupper(*cp) : *cp)-'A'+10) < base) {
        result = result*base + value;
        cp++;
    }
    if(endp)
        *endp = (char *)cp;
    return result;
}
*/

