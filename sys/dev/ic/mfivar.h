/* $OpenBSD: mfivar.h,v 1.40 2010/12/30 08:53:50 dlg Exp $ */
/*
 * Copyright (c) 2006 Marco Peereboom <marco@peereboom.us>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define DEVNAME(_s)     ((_s)->sc_dev.dv_xname)

/* #define MFI_DEBUG */
#ifdef MFI_DEBUG
extern uint32_t			mfi_debug;
#define DPRINTF(x...)		do { if (mfi_debug) printf(x); } while(0)
#define DNPRINTF(x...)		do { printf(x) } while(0)
#define	MFI_D_CMD		0x0001
#define	MFI_D_INTR		0x0002
#define	MFI_D_MISC		0x0004
#define	MFI_D_DMA		0x0008
#define	MFI_D_IOCTL		0x0010
#define	MFI_D_RW		0x0020
#define	MFI_D_MEM		0x0040
#define	MFI_D_CCB		0x0080
#else
#define DPRINTF(x...)
#define DNPRINTF(n,x...)
#endif

struct mfi_mem {
	bus_dmamap_t		am_map;
	bus_dma_segment_t	am_seg;
	size_t			am_size;
	caddr_t			am_kva;
};

#define MFIMEM_MAP(_am)		((_am)->am_map)
#define MFIMEM_DVA(_am)		((_am)->am_map->dm_segs[0].ds_addr)
#define MFIMEM_KVA(_am)		((void *)(_am)->am_kva)

struct mfi_prod_cons {
	uint32_t		mpc_producer;
	uint32_t		mpc_consumer;
	uint32_t		mpc_reply_q[1]; /* compensate for 1 extra reply per spec */
};

struct mfi_ccb {
	struct mfi_softc	*ccb_sc;

	union mfi_frame		*ccb_frame;
	paddr_t			ccb_pframe;
	bus_addr_t		ccb_pframe_offset;
	uint32_t		ccb_frame_size;
	uint32_t		ccb_extra_frames;

	struct mfi_sense	*ccb_sense;
	paddr_t			ccb_psense;

	bus_dmamap_t		ccb_dmamap;

	union mfi_sgl		*ccb_sgl;

	/* data for sgl */
	void			*ccb_data;
	uint32_t		ccb_len;

	uint32_t		ccb_direction;
#define MFI_DATA_NONE	0
#define MFI_DATA_IN	1
#define MFI_DATA_OUT	2

	void			*ccb_cookie;
	void			(*ccb_done)(struct mfi_ccb *);

	volatile enum {
		MFI_CCB_FREE,
		MFI_CCB_READY,
		MFI_CCB_DONE
	}			ccb_state;
	uint32_t		ccb_flags;
#define MFI_CCB_F_ERR			(1<<0)
	SLIST_ENTRY(mfi_ccb)	ccb_link;
};

SLIST_HEAD(mfi_ccb_list, mfi_ccb);

enum mfi_iop {
	MFI_IOP_XSCALE,
	MFI_IOP_PPC,
	MFI_IOP_GEN2,
	MFI_IOP_TBOLT
};

struct mfi_iop_ops {
	u_int32_t	(*mio_fw_state)(struct mfi_softc *);
	void		(*mio_intr_ena)(struct mfi_softc *);
	int		(*mio_intr)(struct mfi_softc *);
	void		(*mio_post)(struct mfi_softc *, struct mfi_ccb *);
};



struct mfi_qstat{
			uint32_t q_length;
			uint32_t q_max;
		};

struct mfi_softc {
	struct device		sc_dev;
	void			*sc_ih;
	struct scsi_link	sc_link;
	struct scsi_iopool	sc_iopool;

	const struct mfi_iop_ops *sc_iop;

	u_int32_t		sc_flags;

	bus_space_tag_t		sc_iot;
	bus_space_handle_t	sc_ioh;
	bus_dma_tag_t		sc_dmat;

	/* save some useful information for logical drives that is missing
	 * in sc_ld_list
	 */


#if 1  //modify by niupj
	int				mfi_flags;
#define MFI_FLAGS_SG64		(1<<0)
#define MFI_FLAGS_QFRZN		(1<<1)
#define MFI_FLAGS_OPEN		(1<<2)
#define MFI_FLAGS_STOP		(1<<3)
#define MFI_FLAGS_1064R		(1<<4)
#define MFI_FLAGS_1078		(1<<5)
#define MFI_FLAGS_GEN2		(1<<6)
#define MFI_FLAGS_SKINNY	(1<<7)
#define MFI_FLAGS_TBOLT		(1<<8)
#define MFI_FLAGS_MRSAS		(1<<9)
#define MFI_FLAGS_INVADER	(1<<10)
#define MFI_FLAGS_FURY		(1<<11)
	// Start: LSIP200113393
	bus_dma_tag_t			verbuf_h_dmat;
	bus_dmamap_t			verbuf_h_dmamap;
	bus_addr_t			verbuf_h_busaddr;
	uint32_t			*verbuf;
//	void				*kbuff_arr[MAX_IOCTL_SGE];
	bus_dma_tag_t			mfi_kbuff_arr_dmat[2];
	bus_dmamap_t			mfi_kbuff_arr_dmamap[2];
	bus_addr_t			mfi_kbuff_arr_busaddr[2];

	struct mfi_hwcomms		*mfi_comms;
	TAILQ_HEAD(,mfi_command)	mfi_free;
	TAILQ_HEAD(,mfi_command)	mfi_ready;
	TAILQ_HEAD(BUSYQ,mfi_command)	mfi_busy;
//	struct bio_queue_head		mfi_bioq;
#define MFIQ_FREE 0
#define MFIQ_BIO 1
#define MFIQ_READY 2
#define MFIQ_BUSY 3
#define MFIQ_COUNT 4
	struct mfi_qstat		mfi_qstat[MFIQ_COUNT];

	struct resource			*mfi_regs_resource;
	bus_space_handle_t		mfi_bhandle;
	bus_space_tag_t			mfi_btag;
	int				mfi_regs_rid;

	bus_dma_tag_t			mfi_parent_dmat;
	bus_dma_tag_t			mfi_buffer_dmat;

	bus_dma_tag_t			mfi_comms_dmat;
	bus_dmamap_t			mfi_comms_dmamap;
	bus_addr_t			mfi_comms_busaddr;

	bus_dma_tag_t			mfi_frames_dmat;
	bus_dmamap_t			mfi_frames_dmamap;
	bus_addr_t			mfi_frames_busaddr;
	union mfi_frame			*mfi_frames;

	bus_dma_tag_t			mfi_tb_init_dmat;
	bus_dmamap_t			mfi_tb_init_dmamap;
	bus_addr_t			mfi_tb_init_busaddr;
	bus_addr_t			mfi_tb_ioc_init_busaddr;
	union mfi_frame			*mfi_tb_init;

//	TAILQ_HEAD(,mfi_evt_queue_elm)	mfi_evt_queue;
//	struct task			mfi_evt_task;
//	struct task			mfi_map_sync_task;
//	TAILQ_HEAD(,mfi_aen)		mfi_aen_pids;
	struct mfi_command		*mfi_aen_cm;
	struct mfi_command		*mfi_skinny_cm;
	struct mfi_command		*mfi_map_sync_cm;
	int				cm_aen_abort;
	int				cm_map_abort;
	uint32_t			mfi_aen_triggered;
	uint32_t			mfi_poll_waiting;
	uint32_t			mfi_boot_seq_num;
	//struct selinfo			mfi_select;
	int				mfi_delete_busy_volumes;
	int				mfi_keep_deleted_volumes;
	int				mfi_detaching;

	bus_dma_tag_t			mfi_sense_dmat;
	bus_dmamap_t			mfi_sense_dmamap;
	bus_addr_t			mfi_sense_busaddr;
	struct mfi_sense		*mfi_sense;

	struct resource			*mfi_irq;
	void				*mfi_intr;
	int				mfi_irq_rid;

	//struct intr_config_hook		mfi_ich;
//	eventhandler_tag		eh;
	/* OCR flags */
	uint8_t adpreset;
	uint8_t issuepend_done;
	uint8_t disableOnlineCtrlReset;
	uint32_t mfiStatus;
	uint32_t last_seq_num;
	uint32_t volatile hw_crit_error;

	/*
	 * Allocation for the command array.  Used as an indexable array to
	 * recover completed commands.
	 */
	struct mfi_command		*mfi_commands;
	/*
	 * How many commands the firmware can handle.  Also how big the reply
	 * queue is, minus 1.
	 */
	int				mfi_max_fw_cmds;
	/*
	 * How many S/G elements we'll ever actually use
	 */
	int				mfi_max_sge;
	/*
	 * How many bytes a compound frame is, including all of the extra frames
	 * that are used for S/G elements.
	 */
	int				mfi_cmd_size;
	/*
	 * How large an S/G element is.  Used to calculate the number of single
	 * frames in a command.
	 */
	int				mfi_sge_size;
	/*
	 * Max number of sectors that the firmware allows
	 */
	uint32_t			mfi_max_io;

#if 0
	TAILQ_HEAD(,mfi_disk)		mfi_ld_tqh;
	TAILQ_HEAD(,mfi_system_pd)	mfi_syspd_tqh;
	TAILQ_HEAD(,mfi_disk_pending)	mfi_ld_pend_tqh;
	TAILQ_HEAD(,mfi_system_pending)	mfi_syspd_pend_tqh;
	eventhandler_tag		mfi_eh;
	struct cdev			*mfi_cdev;

//	TAILQ_HEAD(, ccb_hdr)		mfi_cam_ccbq;
	struct mfi_command *		(* mfi_cam_start)(void *);
	void				(*mfi_cam_rescan_cb)(struct mfi_softc *,    uint32_t);
#endif
//	struct callout			mfi_watchdog_callout;
//	struct mtx			mfi_io_lock;
//	struct sx			mfi_config_lock;

	/* Controller type specific interfaces */
	void	(*mfi_enable_intr)(struct mfi_softc *sc);
	void	(*mfi_disable_intr)(struct mfi_softc *sc);
	int32_t	(*mfi_read_fw_status)(struct mfi_softc *sc);
	int	(*mfi_check_clear_intr)(struct mfi_softc *sc);
	void	(*mfi_issue_cmd)(struct mfi_softc *sc, bus_addr_t bus_add,
		    uint32_t frame_cnt);
	int	(*mfi_adp_reset)(struct mfi_softc *sc);
	int	(*mfi_adp_check_reset)(struct mfi_softc *sc);
	void				(*mfi_intr_ptr)(void *sc);

	/* ThunderBolt */
	uint32_t			mfi_tbolt;
	uint32_t			MFA_enabled;
	/* Single Reply structure size */
	uint16_t			reply_size;
	/* Singler message size. */
	uint16_t			raid_io_msg_size;
	TAILQ_HEAD(TB, mfi_cmd_tbolt)	mfi_cmd_tbolt_tqh;
	/* ThunderBolt base contiguous memory mapping. */
	bus_dma_tag_t			mfi_tb_dmat;
	bus_dmamap_t			mfi_tb_dmamap;
	bus_addr_t			mfi_tb_busaddr;
	/* ThunderBolt Contiguous DMA memory Mapping */
	uint8_t	*			request_message_pool;
	uint8_t *			request_message_pool_align;
	uint8_t *			request_desc_pool;
	bus_addr_t			request_msg_busaddr;
	bus_addr_t			reply_frame_busaddr;
	bus_addr_t			sg_frame_busaddr;
	/* ThunderBolt IOC Init Descriptor */
	bus_dma_tag_t			mfi_tb_ioc_init_dmat;
	bus_dmamap_t			mfi_tb_ioc_init_dmamap;
	uint8_t *			mfi_tb_ioc_init_desc;
	struct mfi_cmd_tbolt		**mfi_cmd_pool_tbolt;
	/* Virtual address of reply Frame Pool */
	struct mfi_mpi2_reply_header*	reply_frame_pool;
	struct mfi_mpi2_reply_header*	reply_frame_pool_align;

	/* Last reply frame address */
	uint8_t *			reply_pool_limit;
	uint16_t			last_reply_idx;
	uint8_t				max_SGEs_in_chain_message;
	uint8_t				max_SGEs_in_main_message;
	uint8_t				chain_offset_value_for_main_message;
	uint8_t				chain_offset_value_for_mpt_ptmsg;
#endif








	struct {
		uint32_t	ld_present;
		char		ld_dev[16];	/* device name sd? */
	}			sc_ld[MFI_MAX_LD];

	/* scsi ioctl from sd device */
	int			(*sc_ioctl)(struct device *, u_long, caddr_t);

	/* firmware determined max, totals and other information*/
	uint32_t		sc_max_cmds;
	uint32_t		sc_max_sgl;
	uint32_t		sc_max_ld;
	uint32_t		sc_ld_cnt;

	/* bio */
	struct mfi_conf		*sc_cfg;
	struct mfi_ctrl_info	*sc_info;
	struct mfi_ld_list	sc_ld_list;
	struct mfi_ld_details	*sc_ld_details; /* array to all logical disks */
	int			sc_no_pd; /* used physical disks */
	int			sc_ld_sz; /* sizeof sc_ld_details */

	/* all commands */
	struct mfi_ccb		*sc_ccb;

	/* producer/consumer pointers and reply queue */
	struct mfi_mem		*sc_pcq;

	/* frame memory */
	struct mfi_mem		*sc_frames;
	uint32_t		sc_frames_size;

	/* sense memory */
	struct mfi_mem		*sc_sense;

	struct mfi_ccb_list	sc_ccb_freeq;
//	struct mutex		sc_ccb_mtx;//wan-

	/* mgmt lock */
//	struct rwlock		sc_lock;//wan-

	/* sensors */
	struct ksensor		*sc_sensors;
	struct ksensordev	sc_sensordev;








};

int	mfi_attach(struct mfi_softc *sc, enum mfi_iop);
int	mfi_intr(void *);
