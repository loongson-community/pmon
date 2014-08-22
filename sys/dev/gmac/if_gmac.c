#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/syslog.h>


#include "synopGMAC_plat.h"


static int
syn_match(parent, match, aux)
	struct device *parent;
#if defined(__BROKEN_INDIRECT_CONFIG) || defined(__OpenBSD__)
	void *match;
#else
	struct cfdata *match;
#endif
	void *aux;
{
	return 1;
}

s32  synopGMAC_init_network_interface(char* xname,u64 synopGMACMappedAddr);

static void
syn_attach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct device *sc = self;
#ifdef LOONGSON_2G5536
synopGMAC_init_network_interface(sc->dv_xname,sc->dv_unit?0x90000d0000000000LL:0x90000c0000000000LL);
#else
	synopGMAC_init_network_interface(sc->dv_xname, sc);
#endif

}


struct cfattach syn_ca = {
	sizeof(struct device), syn_match, syn_attach
};

struct cfdriver syn_cd = {
	NULL, "syn", DV_IFNET
};



