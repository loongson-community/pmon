#include <pmon.h>
#include "hda.h"

//#define HDA_DEBUG
struct hda_pintbl {
	unsigned short nid;
	unsigned int val;
};
struct snd_hda_pin {
	unsigned int codec;             /* Codec vendor/device ID */
	const struct hda_pintbl *pins;  /* list of matching pins */
};

#define SND_HDA_PIN(_codec, _pins...) \
	{ .codec = _codec,\
	  .pins = (const struct hda_pintbl[]) { _pins, {0, 0}} \
	}

static const struct snd_hda_pin alc_pin_fixup_tbl[] = {
	SND_HDA_PIN(0x10ec0269,
		{0x1b, 0x02214c40},
		{0x15, 0x01014030}),
	SND_HDA_PIN(0x10ec0662,
		{}),
	SND_HDA_PIN(0x10ec0882,
		{0x1b, 0x02214c40},
		{0x15, 0x01014030}),
	{}
};

int snd_hda_pick_pin_fixup(unsigned int codec_id,
                            const struct snd_hda_pin *pin_quirk)
{
	const struct snd_hda_pin *pq;
	int i = 0;
	for (pq = pin_quirk; pq->codec; pq++, i++) {
		if (codec_id != pq->codec)
		{
			continue;
		}
		return i;
	}
	return -1;
}

void hda_codec_set(void)
{
	int *corb_p, *corb_p0;
	corb_p = corb_p0 = (unsigned int*)pmalloc(sizeof(int) * 0x1000);
	if(corb_p == 0) {
		printf("HDA: can't alloc memory for corb rirb\n");
		return;
	}
	corb_p = (unsigned int)corb_p + (0x80 - ((unsigned int)corb_p & 0x7f)); //used to align adress
	int *rirb_p = (unsigned int)corb_p + 0x400;
	unsigned int corb_buf[80];
	struct snd_hda_pin *pq;
	struct hda_pintbl *ptr;
	int times = 10000;
	int i, j;
	unsigned int val;

#ifdef HDA_DEBUG
	printf("corb_p = 0x%lx\n",corb_p);
	printf("rirb_p = 0x%lx\n",rirb_p);
#endif
	/*7A HDA device 7 function 0*/
	unsigned int base = ls_readl((0xba000000 | (0 << 16) | (7 << 11) | (0 << 9)) + 0x10) & ~0xf;
	base += 0x80000000;
#ifdef HDA_DEBUG
	printf("base = 0x%x\n",base);
#endif
	/*init hda controller*/
	hda_writel(base, GCTL, hda_readb(base, GCTL) & ~ICH6_GCTL_RESET);
	while ((hda_readw(base, GCTL) & 0x1) && times--);
	if (times < 0) {
		printf("HDA reset time out!\n");
		goto hda_out;
	}
	hda_writel(base, GCTL, hda_readb(base, GCTL) | ICH6_GCTL_RESET);
	hda_writew(base, STATESTS,0x1);
	hda_writew(base, GSTS,0x2);
	hda_writel(base, INTCTL,0xc0000010);
	hda_writel(base, CORBLBASE,(unsigned int)corb_p & 0xfffffff);
	hda_writeb(base, CORBCTL,ICH6_CORBCTL_RUN);
	hda_writeb(base, CORBSIZE,0x1);
	hda_writel(base, RIRBLBASE,(unsigned int)rirb_p & 0xfffffff);
	hda_writeb(base, RIRBCTL,ICH6_RBCTL_DMA_EN);
	hda_writeb(base, RIRBSTS,ICH6_RBSTS_OVERRUN);
	hda_writeb(base, RIRBSIZE,2);

	/*this code to get code ID*/
	ls_readl(corb_p + 1) = 0xf0000;
	delay(0x1000),
	hda_writew(base, CORBWP,1);
	times = 1000;
	while(!(hda_readb(base, RIRBSTS) & 0x1) && times--);
	if (times < 0) {
		printf("HDA rirb time out!\n");
		goto hda_out;
	}
	val = ls_readl(rirb_p + 2);
	hda_writeb(base, RIRBSTS, val | 0x1);

	/*get alc_pin_fixup_tbl value to set codec*/
	i = snd_hda_pick_pin_fixup(val,alc_pin_fixup_tbl);
	printf("codec ID: 0x%x\n",val);
	if (i < 0) {
		printf("HDA:fixup_tlb not have this codec!\n");
		goto hda_out;
	}
	pq = alc_pin_fixup_tbl;
	pq += i;
	ptr = pq->pins;

	/*get id operation was used 1*/
	j = 2;
	/*prepare corb buf*/
	while(pq->pins->nid) {
		for(i = 0; i < 4; i++) {
			corb_buf[j++] = (pq->pins->nid << 20) | (0x71c + i) << 8 | ((pq->pins->val >> (i * 8)) & 0xff);
		}
		corb_buf[j++] = (pq->pins->nid << 20) | 0xf1c00;
		pq->pins++;
	}
	for(i = 2; i < j; i++) {
		ls_readl(corb_p + i) = corb_buf[i];
		delay(0x1000),
		hda_writew(base, CORBWP, i);
#ifdef HDA_DEBUG
		printf("corb 0x%x\n", corb_buf[i]);
		printf("corb wp %x\n",hda_readw(base, CORBWP));
#endif
		times = 1000;
		while(!(hda_readb(base, RIRBSTS) & 0x1) && times--);
		if (times < 0) {
			printf("HDA rirb time out!\n");
			goto hda_out;
		}
		if ((corb_buf[i] & 0xf0000) == 0xf0000) {
			if (ptr->val != ls_readl(rirb_p + (i*2))) {
				printf("HDA codec set error!\n");
				goto hda_out;
			}
#ifdef HDA_DEBUG
			printf("val -> 0x%x\n", ptr->val);
			printf("corb 0x%x -> rirb 0x%x\n", corb_buf[i], val = ls_readl(rirb_p + (i*2)));
#endif
			ptr++;
		}
		hda_writeb(base, RIRBSTS, 0x1);
	}
	printf("HDA codec cofigure done!\n");
hda_out:
	free(corb_p0);
}
static const Cmd Cmds[] = {
	{"ls hda"},
	{"hda_codec", "", 0, "test the hda function", hda_codec_set, 1, 99, 0},
	{0, 0}
};

static void init_cmd __P((void)) __attribute__ ((constructor));
static void init_cmd()
{
	cmdlist_expand(Cmds, 1);
}
