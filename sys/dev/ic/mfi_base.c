/* $OpenBSD: mfi.c,v 1.114 2010/12/30 08:53:50 dlg Exp $ */
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

#include <stddef.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/ioctl.h>
#include <sys/device.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/proc.h>
#include <sys/sensors.h>

#include <machine/bus.h>

#include <scsi/scsi_all.h>
#include <scsi/scsi_disk.h>
#include <scsi/scsiconf.h>

#include <dev/biovar.h>
#include <dev/ic/mfireg.h>
#include <dev/ic/mfivar.h>
#include <dev/ic/mfivar_fury.h>

#ifdef MFI_DEBUG
uint32_t	mfi_debug = 0
/*		    | MFI_D_CMD */
/*		    | MFI_D_INTR */
/*		    | MFI_D_MISC */
/*		    | MFI_D_DMA */
/*		    | MFI_D_IOCTL */
/*		    | MFI_D_RW */
/*		    | MFI_D_MEM */
/*		    | MFI_D_CCB */
		;
#endif

//wan+ if
extern int cold;
extern size_t strlcpy(char *dst, const char*src, size_t siz);
void
mfi_scsi_xs_done_cm(struct mfi_command *cm);
void	mfi_scsi_cmd_fury(struct scsi_xfer *);
int	mfi_scsi_ioctl_fury(struct scsi_link *, u_long, caddr_t, int);
void	mfiminphys_fury(struct buf *bp, struct scsi_link *sl);

struct scsi_adapter mfi_switch_fury = {
	mfi_scsi_cmd_fury, mfiminphys_fury, 0, 0, mfi_scsi_ioctl_fury
};

static int
mfi_alloc_commands(struct mfi_softc *sc);

struct mfi_mem	*mfi_allocmem(struct mfi_softc *, size_t);
void		mfi_freemem_fury(struct mfi_softc *, struct mfi_mem *);

static int mfi_get_controller_info(struct mfi_softc *sc);
int		mfi_transition_firmware_fury(struct mfi_softc *);
int		mfi_initialize_firmware(struct mfi_softc *);
int		mfi_get_info_fury(struct mfi_softc *);
int		mfi_create_sgl(struct mfi_ccb *, int);

extern  uint32_t	mfi_read(struct mfi_softc *, bus_size_t);
extern void		mfi_write(struct mfi_softc *, bus_size_t, uint32_t);
/* commands */
int		mfi_scsi_ld(struct mfi_ccb *, struct scsi_xfer *);
int		mfi_scsi_io(struct mfi_ccb *, struct scsi_xfer *, uint64_t,
		    uint32_t);
void		mfi_scsi_xs_done(struct mfi_ccb *);
int		mfi_do_mgmt(struct mfi_softc *, struct mfi_ccb * , uint32_t,
		    uint32_t, uint32_t, void *, uint8_t *);
void		mfi_mgmt_done(struct mfi_ccb *);

#if NBIO > 0
int		mfi_ioctl(struct device *, u_long, caddr_t);
int		mfi_bio_getitall(struct mfi_softc *);
int		mfi_ioctl_inq(struct mfi_softc *, struct bioc_inq *);
int		mfi_ioctl_vol(struct mfi_softc *, struct bioc_vol *);
int		mfi_ioctl_disk(struct mfi_softc *, struct bioc_disk *);
int		mfi_ioctl_alarm(struct mfi_softc *, struct bioc_alarm *);
int		mfi_ioctl_blink(struct mfi_softc *sc, struct bioc_blink *);
int		mfi_ioctl_setstate(struct mfi_softc *, struct bioc_setstate *);
int		mfi_bio_hs(struct mfi_softc *, int, int, void *);
#ifndef SMALL_KERNEL
int		mfi_create_sensors(struct mfi_softc *);
void		mfi_refresh_sensors(void *);
#endif /* SMALL_KERNEL */
#endif /* NBIO > 0 */
u_int32_t	mfi_tbolt_fw_state(struct mfi_softc *);
void		mfi_tbolt_intr_ena(struct mfi_softc *);
int		mfi_tbolt_intr(struct mfi_softc *);
void		mfi_tbolt_post(struct mfi_softc *, struct mfi_ccb *);
static const struct mfi_iop_ops mfi_iop_tbolt = {
	mfi_tbolt_fw_state,
	mfi_tbolt_intr_ena,
	mfi_tbolt_intr,
	mfi_tbolt_post
};


#define mfi_fw_state(_s)	((_s)->sc_iop->mio_fw_state(_s))
#define mfi_intr_enable(_s)	((_s)->sc_iop->mio_intr_ena(_s))
#define mfi_my_intr(_s)		((_s)->sc_iop->mio_intr(_s))
#define mfi_post(_s, _c)	((_s)->sc_iop->mio_post((_s), (_c)))
void
mfi_release_command(void *io)
{
	struct mfi_command 		*cm = io;
	struct mfi_softc	*sc = cm->cm_sc;
	struct mfi_frame_header	*hdr = &cm->cm_frame->mfr_header;
	uint32_t *hdr_data;

//	DNPRINTF(MFI_D_CCB, "%s: mfi_put_ccb: %p\n", DEVNAME(sc), cm);
  	if(cm->cm_data !=NULL && hdr->mfh_sg_count)
		{
			cm->cm_sg->sg32[0].len = 0;
			cm->cm_sg->sg32[0].addr= 0;
		}
	DNPRINTF("cm->cm_flags is %x \n",cm->cm_flags);
if(cm->cm_flags & MFI_CMD_TBOLT)
	mfi_tbolt_return_cmd(cm->cm_sc,cm->cm_sc->mfi_cmd_pool_tbolt[cm->cm_extra_frames-1],cm);

	hdr_data =(uint32_t *)cm->cm_frame;
	hdr_data[0]=0;
	hdr_data[1]=0;
	hdr_data[4]=0;
	hdr_data[5]=0;
//	cm->cm_state = MFI_CCB_FREE;
	cm->cm_extra_frames = 0;
	cm->cm_flags	    = 0;
	cm->cm_complete	    = NULL;
	cm->cm_private	    = NULL;
	cm->cm_data	    = NULL;
	cm->cm_sg	    = 0;
//	cm->cm_direction = 0;
	cm->cm_total_frame_size	= 0;
	cm->retry_for_fw_reset = 0;
	mfi_enqueue_free(sc,cm);	
//	SLIST_INSERT_HEAD(&sc->sc_ccb_freeq, ccb, ccb_link);
}
#if 1 //modify by niupj



static int
mfi_alloc_commands(struct mfi_softc *sc)
{
	struct mfi_command *cm;
	int i, j;

	/*
	 * XXX Should we allocate all the commands up front, or allocate on
	 * demand later like 'aac' does?
	 */

	sc->mfi_commands = malloc(sizeof(sc->mfi_commands[0]) * sc->mfi_max_fw_cmds, M_DEVBUF, M_WAITOK | M_ZERO);
	bzero(sc->mfi_commands, sizeof(sc->mfi_commands[0]) * sc->mfi_max_fw_cmds);

	for (i = 0; i < sc->mfi_max_fw_cmds; i++) {
		cm = &sc->mfi_commands[i];
#if 0
		cm->cm_frame = (union mfi_frame *)(sc->mfi_frames +
		    sc->mfi_cmd_size * i);
#endif
		cm->cm_frame = ((uint32_t)sc->mfi_frames +
		    sc->mfi_cmd_size * i);
		cm->cm_frame_busaddr = sc->mfi_frames_busaddr +
		    sc->mfi_cmd_size * i;
		cm->ccb_pframe_offset = sc->sc_frames_size * i;
		cm->cm_frame->mfr_header.mfh_context = i;
		//cm->cm_frame->pad0 = 0;
		cm->cm_sense = &sc->mfi_sense[i];
		cm->cm_sense_busaddr= sc->mfi_sense_busaddr + MFI_SENSE_LEN * i;
		cm->cm_sc = sc;
		cm->cm_index = i;

	DNPRINTF("sizeof (union *) %x\n",sizeof(union mfi_frame));
        DNPRINTF("pmon i %x sc->mfi_frames :%x, sc->mfi_frames_busaddr:%x\n",i,sc->mfi_frames, sc->mfi_frames_busaddr);
        DNPRINTF("pmon cm->cm_frame : %x cm->cm_frame_busaddr %x\n",cm->cm_frame,cm->cm_frame_busaddr);
	DNPRINTF("pmon cm->cm_sense : %x cm->cm_sense_busaddr %x\n",cm->cm_sense,cm->cm_sense_busaddr);
	DNPRINTF("sc->mfi_max_sge is %x\n",sc->mfi_max_sge);
		if (1) {
			mfi_release_command(cm);
		} else {
			printf("Failed to allocate %d "
			   "command blocks, only allocated %d\n",
			    sc->mfi_max_fw_cmds, i - 1);
				while(1);
			for (j = 0; j < i; j++) {
				cm = &sc->mfi_commands[i];
				bus_dmamap_destroy(sc->sc_dmat,
				    cm->cm_dmamap);
			}
			free(sc->mfi_commands, M_DEVBUF);
			sc->mfi_commands = NULL;

			return (-1);
		}
	}

	return (0);
}


#endif


void
mfi_freemem_fury(struct mfi_softc *sc, struct mfi_mem *mm)
{
	DNPRINTF("%s: mfi_freemem: %p\n", DEVNAME(sc), mm);

	bus_dmamap_unload(sc->sc_dmat, mm->am_map);
	bus_dmamem_unmap(sc->sc_dmat, mm->am_kva, mm->am_size);
	bus_dmamem_free(sc->sc_dmat, &mm->am_seg, 1);
	bus_dmamap_destroy(sc->sc_dmat, mm->am_map);
	free(mm, M_DEVBUF);
}

int
mfi_transition_firmware_fury(struct mfi_softc *sc)
{
	uint32_t fw_state, cur_state;
	int max_wait, i;
	uint32_t cur_abs_reg_val = 0;
	uint32_t prev_abs_reg_val = 0;
//#define MFI_RESET_WAIT_TIME 18
#define MFI_FWINIT_CLEAR_HANDSHAKE 0x00000008 
#define MFI_FWINIT_HOTPLUG 0x00000010
	cur_abs_reg_val = sc->mfi_read_fw_status(sc);
	DNPRINTF("cur_abs_reg_val1 is %x\n",cur_abs_reg_val);
	fw_state = cur_abs_reg_val & MFI_FWSTATE_MASK;
	while (fw_state != MFI_FWSTATE_READY) {
		cur_state = fw_state;
		switch (fw_state) {
		case MFI_FWSTATE_FAULT:
			device_printf(sc->mfi_dev, "Firmware fault\n");
			return (ENXIO);
		case MFI_FWSTATE_WAIT_HANDSHAKE:
			    MFI_WRITE4(sc, MFI_SKINNY_IDB, MFI_FWINIT_CLEAR_HANDSHAKE|0x10);
			max_wait = MFI_RESET_WAIT_TIME;
			break;
		case MFI_FWSTATE_OPERATIONAL:
			    MFI_WRITE4(sc, MFI_SKINNY_IDB, 7);
				while(MFI_READ4(sc,MFI_SKINNY_IDB)&1)
				printf("MFI_7 cur_abs_reg_val is %x\n",cur_abs_reg_val);
			max_wait = MFI_RESET_WAIT_TIME;
			break;
		case MFI_FWSTATE_UNDEFINED:
		case MFI_FWSTATE_BB_INIT:
			max_wait = MFI_RESET_WAIT_TIME;
			break;
		case MFI_FWSTATE_FW_INIT_2:
			max_wait = MFI_RESET_WAIT_TIME;
			break;
		case MFI_FWSTATE_FW_INIT:
		case MFI_FWSTATE_FLUSH_CACHE:
			max_wait = MFI_RESET_WAIT_TIME;
			break;
		case MFI_FWSTATE_DEVICE_SCAN:
			max_wait = MFI_RESET_WAIT_TIME; /* wait for 180 seconds */
			prev_abs_reg_val = cur_abs_reg_val;
			break;
		case MFI_FWSTATE_BOOT_MESSAGE_PENDING:
			    MFI_WRITE4(sc, MFI_SKINNY_IDB, MFI_FWINIT_HOTPLUG);
			max_wait = MFI_RESET_WAIT_TIME;
			break;
		default:
			device_printf(sc->mfi_dev, "Unknown firmware state %#x\n",
			    fw_state);
			return (ENXIO);
		}
	cur_abs_reg_val = sc->mfi_read_fw_status(sc);
	DNPRINTF("cur_abs_reg_val2 is %x\n",cur_abs_reg_val);
		for (i = 0; i < (max_wait * 10); i++) {
			symprintf();
			cur_abs_reg_val = sc->mfi_read_fw_status(sc);
			fw_state = cur_abs_reg_val & MFI_FWSTATE_MASK;
			if (fw_state == cur_state)
				DELAY(100000);
			else
				break;
		}
		if (fw_state == MFI_FWSTATE_DEVICE_SCAN) {
			/* Check the device scanning progress */
			if (prev_abs_reg_val != cur_abs_reg_val) {
				continue;
			}
		}
		if (fw_state == cur_state) {
			device_printf(sc->mfi_dev, "Firmware stuck in state "
			    "%#x\n", fw_state);
			printf("Firmware stuck in state "
			    "%#x\n", fw_state);
			return (ENXIO);
		}
	}
	DNPRINTF("cur_abs_reg_val is %x\n",cur_abs_reg_val);
	printf("cur_abs_reg_val is %x\n",cur_abs_reg_val);
	return (0);

}

#if 1 //modify by niuj

int
mfi_send_frame(struct mfi_softc *sc, struct mfi_command *cm)
{
	int error;


	if (sc->MFA_enabled)
		error = mfi_tbolt_send_frame(sc, cm);
	else
		error = mfi_std_send_frame(sc, cm);

	if (error != 0 && (cm->cm_flags & MFI_ON_MFIQ_BUSY) != 0)
		mfi_remove_busy(cm);

	return (error);
}
 int
mfi_std_send_frame(struct mfi_softc *sc, struct mfi_command *cm)
{
	struct mfi_frame_header *hdr;
	int tm = 60 * 1000;

	hdr = &cm->cm_frame->mfr_header;

	if ((cm->cm_flags & MFI_CMD_POLLED) == 0) {
		cm->cm_timestamp = time_uptime;
		mfi_enqueue_busy(sc,cm);
	} else {
		hdr->mfh_cmd_status = MFI_STAT_INVALID_STATUS;
		hdr->mfh_flags |= MFI_FRAME_DONT_POST_IN_REPLY_QUEUE;
	}

	/*
	 * The bus address of the command is aligned on a 64 byte boundary,
	 * leaving the least 6 bits as zero.  For whatever reason, the
	 * hardware wants the address shifted right by three, leaving just
	 * 3 zero bits.  These three bits are then used as a prefetching
	 * hint for the hardware to predict how many frames need to be
	 * fetched across the bus.  If a command has more than 8 frames
	 * then the 3 bits are set to 0x7 and the firmware uses other
	 * information in the command to determine the total amount to fetch.
	 * However, FreeBSD doesn't support I/O larger than 128K, so 8 frames
	 * is enough for both 32bit and 64bit systems.
	 */
	if (cm->cm_extra_frames > 7)
		cm->cm_extra_frames = 7;

	sc->mfi_issue_cmd(sc, cm->cm_frame_busaddr, cm->cm_extra_frames);

	if ((cm->cm_flags & MFI_CMD_POLLED) == 0)
		return (0);
	/* This is a polled command, so busy-wait for it to complete. */
	while (hdr->mfh_cmd_status == MFI_STAT_INVALID_STATUS) {
		DELAY(1000);
		tm -= 1;
		if (tm <= 0)
			break;
	}

	if (hdr->mfh_cmd_status == MFI_STAT_INVALID_STATUS) {
		device_printf(sc->mfi_dev, "Frame %p timed out "
		    "command 0x%X\n", hdr, cm->cm_frame->dcmd.opcode);
		return (ETIMEDOUT);
	}

	return (0);
}


static int
mfi_data_cb(void *arg, void *segs, int nsegs, int error)
{
	struct mfi_frame_header *hdr;
	struct mfi_command *cm;
	union mfi_sgl *sgl;
	struct mfi_softc *sc;
	int i, j, first, dir;
	int sge_size, locked;

	int *tem_addr, tem_i;
	cm = (struct mfi_command *)arg;
	sc = cm->cm_sc;
	hdr = &cm->cm_frame->mfr_header;
	sgl = cm->cm_sg;

	/*
	 * We need to check if we have the lock as this is async
	 * callback so even though our caller mfi_mapcmd asserts
	 * it has the lock, there is no garantee that hasn't been
	 * dropped if bus_dmamap_load returned prior to our
	 * completion.
	 */

	if (error) {
		printf("error %d in callback\n", error);
		cm->cm_error = error;
		mfi_complete(sc, cm);
		goto out;
	}
	/* Use IEEE sgl only for IO's on a SKINNY controller
	 * For other commands on a SKINNY controller use either
	 * sg32 or sg64 based on the sizeof(bus_addr_t).
	 * Also calculate the total frame size based on the type
	 * of SGL used.
	 */
	segs = (void *) ((unsigned int ) segs & 0x7fffffff);
	DNPRINTF("nsegs is %x\n",nsegs);
	if (((cm->cm_frame->mfr_header.mfh_cmd == MFI_CMD_PD_SCSI_IO) ||
	    (cm->cm_frame->mfr_header.mfh_cmd == MFI_CMD_LD_READ) ||
	    (cm->cm_frame->mfr_header.mfh_cmd == MFI_CMD_LD_WRITE)) &&
	    (sc->mfi_flags & MFI_FLAGS_SKINNY) ) {
		for (i = 0; i < 1; i++) {
			sgl->sg_skinny[i].addr = segs;
			sgl->sg_skinny[i].len = nsegs;
			sgl->sg_skinny[i].flag = 0;
			DNPRINTF("sgl->sg_skinny[%d].addr  is %x\n",i,sgl->sg_skinny[i].addr);
			DNPRINTF("sgl->sg_skinny[%d].len  is %x\n",i,sgl->sg_skinny[i].len);
		}
		hdr->mfh_flags |= MFI_FRAME_IEEE_SGL | MFI_FRAME_SGL64;
		sge_size = sizeof(struct mfi_sg_skinny);
		hdr->mfh_sg_count = 1;
	} else {
		j = 0;
		if (cm->cm_frame->mfr_header.mfh_cmd == MFI_CMD_STP) {
			first = cm->cm_stp_len;
			if ((sc->mfi_flags & MFI_FLAGS_SG64) == 0) {
				sgl->sg32[j].addr = segs;
				sgl->sg32[j++].len = first;
			} else {
				sgl->sg64[j].addr = segs;
				sgl->sg64[j++].len = first;
			}
		} else
			first = 0;
		if ((sc->mfi_flags & MFI_FLAGS_SG64) == 0) {
			for (i = 0; i < 1; i++) {
				sgl->sg32[j].addr = segs + first;
				sgl->sg32[j++].len = nsegs - first;
				first = 0;
			DNPRINTF("sgl->sg32[%d].addr  is %x\n",j-1,sgl->sg32[j-1].addr);
			DNPRINTF("sgl->sg32[%d].len  is %x\n",j-1,sgl->sg32[j-1].len);
			}
		} else {
			for (i = 0; i < nsegs; i++) {
				sgl->sg64[j].addr = segs + first;
				sgl->sg64[j++].len = nsegs - first;
				first = 0;
			}
			hdr->mfh_flags |= MFI_FRAME_SGL64;
		}
		hdr->mfh_sg_count = j;
		sge_size = sc->mfi_sge_size;
	}

//				sgl->sg32[0].addr = 0xfa5a8000;
//				sgl->sg32[0].len = 0x800;
//				sgl->sg32[0].addr &= 0xfffffff;
			//printf("sgl->sg32[0].addr  is %x\n",sgl->sg32[0].addr);
			//printf("sgl->sg32[0].len  is %x\n",sgl->sg32[0].len);
	//int *tem_addr, tem_i;
#if 0
	tem_addr=segs[0].ds_addr | 0x80000000;
		for(tem_i=0;tem_i<0x10;tem_i++)
			tem_addr[tem_i]=tem_i;
		for(tem_i=0;tem_i<0x10;tem_i++)
		printf("tem_addr[%x]=%x\n",tem_i,tem_addr[tem_i]);
#endif



//		hdr->flags |= MFI_FRAME_DIR_READ;
//		hdr->sg_count = 1;
	dir = 0;
	if (cm->cm_flags & MFI_CMD_DATAIN) {
		dir |= BUS_DMASYNC_PREREAD;
		hdr->mfh_flags |= MFI_FRAME_DIR_READ;
	}
	if (cm->cm_flags & MFI_CMD_DATAOUT) {
		dir |= BUS_DMASYNC_PREWRITE;
		hdr->mfh_flags |= MFI_FRAME_DIR_WRITE;
	}
//	bus_dmamap_sync(sc->sc_dmat, cm->cm_dmamap, 0,cm->cm_dmamap->dm_mapsize,dir);
	cm->cm_flags |= MFI_CMD_MAPPED;

	/*
	 * Instead of calculating the total number of frames in the
	 * compound frame, it's already assumed that there will be at
	 * least 1 frame, so don't compensate for the modulo of the
	 * following division.
	 */
	cm->cm_total_frame_size += (sc->mfi_sge_size * nsegs);
	cm->cm_extra_frames = (cm->cm_total_frame_size - 1) / MFI_FRAME_SIZE;

	if ((error = mfi_send_frame(sc, cm)) != 0) {
		printf("error %d in callback from mfi_send_frame\n", error);
		cm->cm_error = error;
		mfi_complete(sc, cm);
		goto out;
	}
#if 0
	tem_addr=segs[0].ds_addr | 0x80000000;
		for(tem_i=0;tem_i<0x10;tem_i++)
		printf("tem_addr[%x]=%x\n",tem_i,tem_addr[tem_i]);
#endif
out:
	/* leave the lock in the state we found it */

	return error;
}




int
mfi_mapcmd(struct mfi_softc *sc, struct mfi_command *cm)
{
	int error, polled;


	if ((cm->cm_data != NULL) && (cm->cm_frame->mfr_header.mfh_cmd != MFI_CMD_STP )) {
		polled = (cm->cm_flags & MFI_CMD_POLLED) ? BUS_DMA_NOWAIT : 0;
#if 0
		if (cm->cm_flags & MFI_CMD_CCB)
			error = bus_dmamap_load_ccb(sc->mfi_buffer_dmat,
			    cm->cm_dmamap, cm->cm_data, mfi_data_cb, cm,
			    polled);
		else if (cm->cm_flags & MFI_CMD_BIO)
{
			error = bus_dmamap_load_bio(sc->mfi_buffer_dmat,
			    cm->cm_dmamap, cm->cm_private, mfi_data_cb, cm,
			    polled);
}
		else
#endif

			 error=   mfi_data_cb(cm,cm->cm_data,cm->cm_len,0);
		if (error == EINPROGRESS) {
			sc->mfi_flags |= MFI_FLAGS_QFRZN;
			return (1);
		}
	} else {
		error = mfi_send_frame(sc, cm);
	}

	return (error);
}


#endif


int
mfi_get_info_fury(struct mfi_softc *sc)
{
#ifdef MFI_DEBUG
	int i;
#endif
	int i;
	DNPRINTF( "%s: mfi_get_info\n", DEVNAME(sc));
#if 0

	if (mfi_mgmt(sc, MR_DCMD_CTRL_GET_INFO, MFI_DATA_IN,
	    sizeof(sc->sc_info), &sc->sc_info, NULL))
		return (1);
#endif
	if(mfi_get_controller_info(sc))
		return	1;
//#ifdef MFI_DEBUG
#if 0
	for (i = 0; i < sc->sc_info->image_component_count; i++) {
		printf("%s: active FW %s Version %s date %s time %s\n",
		    DEVNAME(sc),
		    sc->sc_info->image_component[i].name,
		    sc->sc_info->image_component[i].version,
		    sc->sc_info->image_component[i].build_date,
		    sc->sc_info->image_component[i].build_time);
	}

	for (i = 0; i < sc->sc_info->pending_image_component_count; i++) {
		printf("%s: pending FW %s Version %s date %s time %s\n",
		    DEVNAME(sc),
		    sc->sc_info->pending_image_component[i].name,
		    sc->sc_info->pending_image_component[i].version,
		    sc->sc_info->pending_image_component[i].build_date,
		    sc->sc_info->pending_image_component[i].build_time);
	}

	printf("%s: max_arms %d max_spans %d max_arrs %d max_lds %d name %s\n",
	    DEVNAME(sc),
	    sc->sc_info->max_arms,
	    sc->sc_info->max_spans,
	    sc->sc_info->max_arrays,
	    sc->sc_info->max_lds,
	    sc->sc_info->product_name);

	printf("%s: serial %s present %#x fw time %d max_cmds %d max_sg %d\n",
	    DEVNAME(sc),
	    sc->sc_info->serial_number,
	    sc->sc_info->hw_present,
	    sc->sc_info->current_fw_time,
	    sc->sc_info->max_cmds,
	    sc->sc_info->max_sg_elements);

	printf("%s: max_rq %d lds_pres %d lds_deg %d lds_off %d pd_pres %d\n",
	    DEVNAME(sc),
	    sc->sc_info->max_request_size,
	    sc->sc_info->lds_present,
	    sc->sc_info->lds_degraded,
	    sc->sc_info->lds_offline,
	    sc->sc_info->pd_present);

	printf("%s: pd_dsk_prs %d pd_dsk_pred_fail %d pd_dsk_fail %d\n",
	    DEVNAME(sc),
	    sc->sc_info->pd_disks_present,
	    sc->sc_info->pd_disks_pred_failure,
	    sc->sc_info->pd_disks_failed);

	printf("%s: nvram %d mem %d flash %d\n",
	    DEVNAME(sc),
	    sc->sc_info->nvram_size,
	    sc->sc_info->memory_size,
	    sc->sc_info->flash_size);

	printf("%s: ram_cor %d ram_uncor %d clus_all %d clus_act %d\n",
	    DEVNAME(sc),
	    sc->sc_info->ram_correctable_errors,
	    sc->sc_info->ram_uncorrectable_errors,
	    sc->sc_info->cluster_allowed,
	    sc->sc_info->cluster_active);

	printf("%s: max_strps_io %d raid_lvl %#x adapt_ops %#x ld_ops %#x\n",
	    DEVNAME(sc),
	    sc->sc_info->max_strips_per_io,
	    sc->sc_info->raid_levels,
	    sc->sc_info->adapter_ops,
	    sc->sc_info->ld_ops);

	printf("%s: strp_sz_min %d strp_sz_max %d pd_ops %#x pd_mix %#x\n",
	    DEVNAME(sc),
	    sc->sc_info->stripe_sz_ops.min,
	    sc->sc_info->stripe_sz_ops.max,
	    sc->sc_info->pd_ops,
	    sc->sc_info->pd_mix_support);

	printf("%s: ecc_bucket %d pckg_prop %s\n",
	    DEVNAME(sc),
	    sc->sc_info->ecc_bucket_count,
	    sc->sc_info->package_version);

#if 1
	printf("%s: sq_nm %d prd_fail_poll %d intr_thrtl %d intr_thrtl_to %d\n",
	    DEVNAME(sc),
	    sc->sc_info->properties.seq_num,
	    sc->sc_info->properties.pred_fail_poll_interval,
	    sc->sc_info->properties.intr_throttle_cnt,
	    sc->sc_info->properties.intr_throttle_timeout);

	printf("%s: rbld_rate %d patr_rd_rate %d bgi_rate %d cc_rate %d\n",
	    DEVNAME(sc),
	    sc->sc_info->properties.rebuild_rate,
	    sc->sc_info->properties.patrol_read_rate,
	    sc->sc_info->properties.bgi_rate,
	    sc->sc_info->properties.cc_rate);

	printf("%s: rc_rate %d ch_flsh %d spin_cnt %d spin_dly %d clus_en %d\n",
	    DEVNAME(sc),
	    sc->sc_info->properties.recon_rate,
	    sc->sc_info->properties.cache_flush_interval,
	    sc->sc_info->properties.spinup_drv_cnt,
	    sc->sc_info->properties.spinup_delay,
	    sc->sc_info->properties.cluster_enable);

	printf("%s: coerc %d alarm %d dis_auto_rbld %d dis_bat_wrn %d ecc %d\n",
	    DEVNAME(sc),
	    sc->sc_info->properties.coercion_mode,
	    sc->sc_info->properties.alarm_enable,
	    sc->sc_info->properties.disable_auto_rebuild,
	    sc->sc_info->properties.disable_battery_warn,
	    sc->sc_info->properties.ecc_bucket_size);

	printf("%s: ecc_leak %d rest_hs %d exp_encl_dev %d\n",
	    DEVNAME(sc),
	    sc->sc_info->properties.ecc_bucket_leak_rate,
	    sc->sc_info->properties.restore_hotspare_on_insertion,
	    sc->sc_info->properties.expose_encl_devices);

	printf("%s: vendor %#x device %#x subvendor %#x subdevice %#x\n",
	    DEVNAME(sc),
	    sc->sc_info->pci.vendor,
	    sc->sc_info->pci.device,
	    sc->sc_info->pci.subvendor,
	    sc->sc_info->pci.subdevice);

	printf("%s: type %#x port_count %d port_addr ",
	    DEVNAME(sc),
	    sc->sc_info->host.type,
	    sc->sc_info->host.port_count);
	for (i = 0; i < 8; i++)
		printf("%.0llx ", sc->sc_info->host.port_addr[i]);
	printf("\n");

	printf("%s: type %.x port_count %d port_addr ",
	    DEVNAME(sc),
	    sc->sc_info->device.type,
	    sc->sc_info->device.port_count);

	for (i = 0; i < 8; i++)
		printf("%.0llx ", sc->sc_info->device.port_addr[i]);
	printf("\n");
#endif
#endif /* MFI_DEBUG */

#ifdef MFI_DEBUG
	for (i = 0; i < sc->sc_info->mci_image_component_count; i++) {
		printf("%s: active FW %s Version %s date %s time %s\n",
		    DEVNAME(sc),
		    sc->sc_info->mci_image_component[i].mic_name,
		    sc->sc_info->mci_image_component[i].mic_version,
		    sc->sc_info->mci_image_component[i].mic_build_date,
		    sc->sc_info->mci_image_component[i].mic_build_time);
	}

	for (i = 0; i < sc->sc_info->mci_pending_image_component_count; i++) {
		printf("%s: pending FW %s Version %s date %s time %s\n",
		    DEVNAME(sc),
		    sc->sc_info->mci_pending_image_component[i].mic_name,
		    sc->sc_info->mci_pending_image_component[i].mic_version,
		    sc->sc_info->mci_pending_image_component[i].mic_build_date,
		    sc->sc_info->mci_pending_image_component[i].mic_build_time);
	}

	printf("%s: max_arms %d max_spans %d max_arrs %d max_lds %d name %s\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_max_arms,
	    sc->sc_info->mci_max_spans,
	    sc->sc_info->mci_max_arrays,
	    sc->sc_info->mci_max_lds,
	    sc->sc_info->mci_product_name);

	printf("%s: serial %s present %#x fw time %d max_cmds %d max_sg %d\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_serial_number,
	    sc->sc_info->mci_hw_present,
	    sc->sc_info->mci_current_fw_time,
	    sc->sc_info->mci_max_cmds,
	    sc->sc_info->mci_max_sg_elements);

	printf("%s: max_rq %d lds_pres %d lds_deg %d lds_off %d pd_pres %d\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_max_request_size,
	    sc->sc_info->mci_lds_present,
	    sc->sc_info->mci_lds_degraded,
	    sc->sc_info->mci_lds_offline,
	    sc->sc_info->mci_pd_present);

	printf("%s: pd_dsk_prs %d pd_dsk_pred_fail %d pd_dsk_fail %d\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_pd_disks_present,
	    sc->sc_info->mci_pd_disks_pred_failure,
	    sc->sc_info->mci_pd_disks_failed);

	printf("%s: nvram %d mem %d flash %d\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_nvram_size,
	    sc->sc_info->mci_memory_size,
	    sc->sc_info->mci_flash_size);

	printf("%s: ram_cor %d ram_uncor %d clus_all %d clus_act %d\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_ram_correctable_errors,
	    sc->sc_info->mci_ram_uncorrectable_errors,
	    sc->sc_info->mci_cluster_allowed,
	    sc->sc_info->mci_cluster_active);

	printf("%s: max_strps_io %d raid_lvl %#x adapt_ops %#x ld_ops %#x\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_max_strips_per_io,
	    sc->sc_info->mci_raid_levels,
	    sc->sc_info->mci_adapter_ops,
	    sc->sc_info->mci_ld_ops);

	printf("%s: strp_sz_min %d strp_sz_max %d pd_ops %#x pd_mix %#x\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_stripe_sz_ops.min,
	    sc->sc_info->mci_stripe_sz_ops.max,
	    sc->sc_info->mci_pd_ops,
	    sc->sc_info->mci_pd_mix_support);

	printf("%s: ecc_bucket %d pckg_prop %s\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_ecc_bucket_count,
	    sc->sc_info->mci_package_version);

	printf("%s: sq_nm %d prd_fail_poll %d intr_thrtl %d intr_thrtl_to %d\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_properties.mcp_seq_num,
	    sc->sc_info->mci_properties.mcp_pred_fail_poll_interval,
	    sc->sc_info->mci_properties.mcp_intr_throttle_cnt,
	    sc->sc_info->mci_properties.mcp_intr_throttle_timeout);

	printf("%s: rbld_rate %d patr_rd_rate %d bgi_rate %d cc_rate %d\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_properties.mcp_rebuild_rate,
	    sc->sc_info->mci_properties.mcp_patrol_read_rate,
	    sc->sc_info->mci_properties.mcp_bgi_rate,
	    sc->sc_info->mci_properties.mcp_cc_rate);

	printf("%s: rc_rate %d ch_flsh %d spin_cnt %d spin_dly %d clus_en %d\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_properties.mcp_recon_rate,
	    sc->sc_info->mci_properties.mcp_cache_flush_interval,
	    sc->sc_info->mci_properties.mcp_spinup_drv_cnt,
	    sc->sc_info->mci_properties.mcp_spinup_delay,
	    sc->sc_info->mci_properties.mcp_cluster_enable);

	printf("%s: coerc %d alarm %d dis_auto_rbld %d dis_bat_wrn %d ecc %d\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_properties.mcp_coercion_mode,
	    sc->sc_info->mci_properties.mcp_alarm_enable,
	    sc->sc_info->mci_properties.mcp_disable_auto_rebuild,
	    sc->sc_info->mci_properties.mcp_disable_battery_warn,
	    sc->sc_info->mci_properties.mcp_ecc_bucket_size);

	printf("%s: ecc_leak %d rest_hs %d exp_encl_dev %d\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_properties.mcp_ecc_bucket_leak_rate,
	    sc->sc_info->mci_properties.mcp_restore_hotspare_on_insertion,
	    sc->sc_info->mci_properties.mcp_expose_encl_devices);

	printf("%s: vendor %#x device %#x subvendor %#x subdevice %#x\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_pci.mip_vendor,
	    sc->sc_info->mci_pci.mip_device,
	    sc->sc_info->mci_pci.mip_subvendor,
	    sc->sc_info->mci_pci.mip_subdevice);

	printf("%s: type %#x port_count %d port_addr ",
	    DEVNAME(sc),
	    sc->sc_info->mci_host.mih_type,
	    sc->sc_info->mci_host.mih_port_count);

	for (i = 0; i < 8; i++)
		printf("%.0llx ", sc->sc_info->mci_host.mih_port_addr[i]);
	printf("\n");

	printf("%s: type %.x port_count %d port_addr ",
	    DEVNAME(sc),
	    sc->sc_info->mci_device.mid_type,
	    sc->sc_info->mci_device.mid_port_count);

	for (i = 0; i < 8; i++)
		printf("%.0llx ", sc->sc_info->mci_device.mid_port_addr[i]);
	printf("\n");
#endif /* MFI_DEBUG */


	return (0);
}

void
mfiminphys_fury(struct buf *bp, struct scsi_link *sl)
{
	DNPRINTF("mfiminphys: %d\n", bp->b_bcount);

	/* XXX currently using MFI_MAXFER = MAXPHYS */
	if (bp->b_bcount > MFI_MAXFER)
		bp->b_bcount = MFI_MAXFER;
	minphys(bp);
}


#define	UNCACHED_TO_PHYS(x) 	((unsigned)(x) & 0x1fffffff)
#define VA_TO_PA(x)     UNCACHED_TO_PHYS(x)
#define	vtophys(p)			_pci_dmamap((unsigned long )p, 1)
 void *dma_alloc_coherent(void *hwdev, size_t size,
                                dma_addr_t *dma_handle, int flag)
{
    void *buf;
    buf = malloc(size, M_DEVBUF, M_WAITOK|M_ZERO);
#ifdef LS3_HT
#else
    CPU_IOFlushDCache(buf, size, SYNC_R);

    buf = (unsigned char *)CACHED_TO_UNCACHED(buf);
#endif
    *dma_handle = vtophys(buf);

    return (void *)buf;
}
void mfi_put_cmd(void *sc,void *cmd) 
	{
		struct mfi_command *cm=cmd;	
		mfi_release_command(cm);
	}
struct megasas_instance *instance;



int
mfi_attach_fury(struct mfi_softc *sc, enum mfi_iop iop)
{
	struct scsibus_attach_args saa;
	int			i;
	uint32_t status;
	int error, commsz, framessz, sensesz;
	int frames, unit, max_fw_sge, max_fw_cmds;
	uint32_t tb_mem_size = 0;



	mfi_initq_free(sc);
	mfi_initq_ready(sc);
	mfi_initq_busy(sc);

	sc->adpreset = 0;
	sc->last_seq_num = 0;
	sc->disableOnlineCtrlReset = 1;
	sc->issuepend_done = 1;
	sc->hw_crit_error = 0;



	/* switch the dev class */
	switch (iop) {
#if 0
	case MFI_IOP_XSCALE:
		sc->sc_iop = &mfi_iop_xscale;
		break;
	case MFI_IOP_PPC:
		sc->sc_iop = &mfi_iop_ppc;//wan: valid for 8708EM2
		break;
	case MFI_IOP_GEN2:
		sc->sc_iop = &mfi_iop_gen2;
		break;
#endif
	case MFI_IOP_TBOLT:
		sc->sc_iop = &mfi_iop_tbolt;
		sc->mfi_enable_intr = mfi_tbolt_enable_intr_ppc;
		sc->mfi_disable_intr = mfi_tbolt_disable_intr_ppc;
		sc->mfi_read_fw_status = mfi_tbolt_read_fw_status_ppc;
		sc->mfi_check_clear_intr = mfi_tbolt_check_clear_intr_ppc;
		sc->mfi_issue_cmd = mfi_tbolt_issue_cmd_ppc;
		sc->mfi_adp_reset = mfi_tbolt_adp_reset;
		sc->mfi_tbolt = 1;
		TAILQ_INIT(&sc->mfi_cmd_tbolt_tqh);

		break;
	default:
		panic("%s: unknown iop %d", DEVNAME(sc), iop);
	}

	DNPRINTF("%s: mfi_attach\n", DEVNAME(sc));
	DNPRINTF("%s: switch device done...\n", DEVNAME(sc));

	/* get some dev status */
	if (mfi_transition_firmware_fury(sc))
	{
		printf("the statue of mfi is error\n");	
		tgt_reboot();
		return (1);
	}
	scsi_iopool_init(&sc->sc_iopool, sc, mfi_dequeue_free, mfi_put_cmd);

	status = mfi_fw_state(sc);

	max_fw_cmds = status & 0xffff;
	sc->mfi_max_fw_cmds = max_fw_cmds;
	max_fw_sge = (status & 0xffff) >> 16;
	sc->mfi_max_sge = min(max_fw_sge, ((MFI_MAXPHYS / PAGE_SIZE) + 1));
	/* consumer/producer and reply queue memory */
#if 1

	/* ThunderBolt Support get the contiguous memory */
	if (sc->mfi_flags & MFI_FLAGS_TBOLT) {
		mfi_tbolt_init_globals(sc);
		device_printf(sc->mfi_dev, "MaxCmd = %d, Drv MaxCmd = %d, "
		    "MaxSgl = %d, state = %#x\n", max_fw_cmds,
		    sc->mfi_max_fw_cmds, sc->mfi_max_sge, status);
		sc->mfi_max_fw_cmds = 16;
		tb_mem_size = mfi_tbolt_get_memory_requirement(sc);
#if 1
//		sc->sc_tb_request_message_pool = mfi_allocmem(sc, tb_mem_size);
			sc->request_message_pool = dma_alloc_coherent(0,
			tb_mem_size,
			&sc->mfi_tb_busaddr, 0);

		if (sc->request_message_pool == NULL) {
			printf("%s: unable to allocate reply queue memory\n",
		    	DEVNAME(sc));
			goto nopcq;
		}
#endif
		bzero(sc->request_message_pool, tb_mem_size);
		/* For ThunderBolt memory init */

//		sc->sc_tb_init = mfi_allocmem(sc, MFI_FRAME_SIZE);
			sc->mfi_tb_init = dma_alloc_coherent(0,
			MFI_FRAME_SIZE,
			&sc->mfi_tb_init_busaddr , 0);

		if (sc->mfi_tb_init  == NULL) {
			printf("%s: unable to allocate reply queue memory\n",
		    	DEVNAME(sc));
			goto nopcq;
		}

		bzero(sc->mfi_tb_init, MFI_FRAME_SIZE);

		if (mfi_tbolt_init_desc_pool(sc, sc->request_message_pool,
		    tb_mem_size)) {
			device_printf(sc->mfi_dev,
			    "Thunderbolt pool preparation error\n");
			return 0;
		}

		/*
		  Allocate DMA memory mapping for MPI2 IOC Init descriptor,
		  we are taking it diffrent from what we have allocated for Request
		  and reply descriptors to avoid confusion later
		*/
		tb_mem_size = sizeof(struct MPI2_IOC_INIT_REQUEST);

//		sc->sc_tb_ioc_init_desc = mfi_allocmem(sc, tb_mem_size);

	sc->mfi_tb_ioc_init_desc = dma_alloc_coherent(0,
			tb_mem_size,
			&sc->mfi_tb_ioc_init_busaddr , 0);



		if (sc->mfi_tb_ioc_init_desc  == NULL) {
			printf("%s: unable to allocate reply queue memory\n",
		    	DEVNAME(sc));
			goto nopcq;
		}

		bzero(sc->mfi_tb_ioc_init_desc, tb_mem_size);
	}


#endif

	if (sizeof(bus_addr_t) == 8) {
		sc->mfi_sge_size = sizeof(struct mfi_sg64);
		sc->mfi_flags |= MFI_FLAGS_SG64;
	} else {
		sc->mfi_sge_size = sizeof(struct mfi_sg32);
	}
#if 1
	if (sc->mfi_flags & MFI_FLAGS_SKINNY)
		sc->mfi_sge_size = sizeof(struct mfi_sg_skinny);
#endif
	frames = (sc->mfi_sge_size * sc->mfi_max_sge - 1) / MFI_FRAME_SIZE + 2;
	frames =16;
	sc->mfi_cmd_size = frames * MFI_FRAME_SIZE;
	framessz = sc->mfi_cmd_size * sc->mfi_max_fw_cmds;
        
	sc->sc_frames_size = sc->mfi_cmd_size;
	sc->mfi_frames = dma_alloc_coherent(0,
			framessz,
			&sc->mfi_frames_busaddr , 0);
	if (sc->mfi_frames == NULL) {
		printf("%s: unable to allocate frame memory\n", DEVNAME(sc));
		goto noframe;
	}
	bzero(sc->mfi_frames, framessz);

	/* XXX hack, fix this */
#if 1
	if ((unsigned int)sc->mfi_frames & 0x3f) {
		printf("%s: improper frame alignment (%#x) FIXME\n",
		    DEVNAME(sc), sc->mfi_frames);
		goto noframe;
	}
#endif
	/* sense memory */
	sensesz = sc->mfi_max_fw_cmds * MFI_SENSE_LEN*2;
	sc->mfi_sense = dma_alloc_coherent(0,
			sensesz,
			&sc->mfi_sense_busaddr , 0);

	if (sc->mfi_sense == NULL) {
		printf("%s: unable to allocate sense memory\n", DEVNAME(sc));
		goto nosense;
	}

	bzero(sc->mfi_sense, sensesz);
	/* now that we have all memory bits go initialize ccbs */
	if (mfi_alloc_commands(sc)) {
		printf("%s: could not init ccb list\n", DEVNAME(sc));
		goto noinit;
	}
#if 1 //modify by niupj

	/* Before moving the FW to operational state, check whether
	 * hostmemory is required by the FW or not
	 */

	/* ThunderBolt MFI_IOC2 INIT */
	if (sc->mfi_flags & MFI_FLAGS_TBOLT) {
		sc->mfi_disable_intr(sc);

		if ((error = mfi_tbolt_init_MFI_queue(sc)) != 0) {
			device_printf(sc->mfi_dev,
			    "TB Init has failed with error %d\n",error);
			return error;
		}

		if ((error = mfi_tbolt_alloc_cmd(sc)) != 0)
			return error;
		sc->mfi_intr_ptr = mfi_intr_tbolt;
		sc->mfi_enable_intr(sc);
	} else {
		printf("it is error\n");
#if 0
		if ((error = mfi_comms_init(sc)) != 0)
			return (error);
#endif
	}

#endif

	 sc->sc_info = malloc(sizeof(struct mfi_ctrl_info), M_DEVBUF, M_NOWAIT|M_ZERO);
	if(sc->sc_info == NULL){
			printf("the sc_info malloc faile\n");
			return 0;
		}
	if ((error = mfi_get_controller_info(sc)) != 0)
		return (error);
	sc->disableOnlineCtrlReset = 0;
	/* kickstart firmware with all addresses and pointers */
#if 1
	if (mfi_get_info_fury(sc)) {
		printf("%s: could not retrieve controller information\n",
		    DEVNAME(sc));
		goto noinit;
	}
#endif
	printf("%s: logical drives %d, version %s, %dMB RAM\n",
	    DEVNAME(sc),
	    sc->sc_info->mci_lds_present,
	    sc->sc_info->mci_package_version,
	    sc->sc_info->mci_memory_size);

	sc->sc_ld_cnt = sc->sc_info->mci_lds_present;
	sc->sc_max_ld = sc->sc_ld_cnt;
	for (i = 0; i < sc->sc_ld_cnt; i++)
		sc->sc_ld[i].ld_present = 1;

	sc->sc_max_cmds = sc->mfi_max_fw_cmds;
	if (sc->sc_ld_cnt)
		sc->sc_link.openings = sc->sc_max_cmds / sc->sc_ld_cnt;
	else
		sc->sc_link.openings = sc->sc_max_cmds;

	printf("sc->sc_link.openings is %x\n",sc->sc_link.openings);
	sc->sc_link.adapter_softc = sc;
	sc->sc_link.adapter = &mfi_switch_fury;
	sc->sc_link.adapter_target = MFI_MAX_LD;
	sc->sc_link.adapter_buswidth = sc->sc_max_ld;
	sc->sc_link.pool = &sc->sc_iopool;

	bzero(&saa, sizeof(saa));
	saa.saa_sc_link = &sc->sc_link;

	config_found(&sc->sc_dev, &saa, scsiprint);

	/* enable interrupts */
	mfi_intr_enable(sc);

#if NBIO > 0
	if (bio_register(&sc->sc_dev, mfi_ioctl) != 0)
		panic("%s: controller registration failed", DEVNAME(sc));
	else
		sc->sc_ioctl = mfi_ioctl;

#ifndef SMALL_KERNEL
	if (mfi_create_sensors(sc) != 0)
		printf("%s: unable to create sensors\n", DEVNAME(sc));
#endif
#endif /* NBIO > 0 */

	return (0);
noinit:
	mfi_freemem_fury(sc, sc->sc_sense);
nosense:
	mfi_freemem_fury(sc, sc->sc_frames);
noframe:
	mfi_freemem_fury(sc, sc->sc_pcq);
nopcq:
	return (1);
}
static struct mfi_command *
mfi_build_syspdio(struct mfi_command *cm, struct scsi_xfer *xs)
{
	struct mfi_pass_frame *pass;
	uint32_t context = 0;
	int flags = 0, blkcount = 0, readop;
	uint8_t cdb_len;
	struct scsi_link *link= xs->sc_link;

	struct mfi_softc	*sc = link->adapter_softc;


	/* Zero out the MFI frame */
	context = cm->cm_frame->mfr_header.mfh_context;

	DNPRINTF("cm->index %x ,context %x \n",cm->cm_index,context);
	DNPRINTF("sizeof (union *) %x\n",sizeof(union mfi_frame));
	DNPRINTF(" sc->mfi_frames :%x, sc->mfi_frames_busaddr:%x\n",sc->mfi_frames, sc->mfi_frames_busaddr);
	DNPRINTF("pmon cm->cm_frame : %x cm->cm_frame_busaddr %x\n",cm->cm_frame,cm->cm_frame_busaddr);
	DNPRINTF("cm->cm_sense is %x,cm->cm_sense_busaddr\n",cm->cm_sense,cm->cm_sense_busaddr);

	bzero(cm->cm_frame, sizeof(union mfi_frame));
	cm->cm_frame->mfr_header.mfh_context = context;
	pass = &cm->cm_frame->mfr_pass;
	bzero(pass->mpf_cdb, 16);
	

	memcpy(pass->mpf_cdb, xs->cmd, xs->cmdlen);
	pass->mpf_header.mfh_cmd = MFI_CMD_PD_SCSI_IO;
	if (xs->flags & (SCSI_DATA_IN )) {
		flags = MFI_CMD_DATAIN | MFI_CMD_BIO;
		readop = 1;
	}else{
		flags = MFI_CMD_DATAOUT | MFI_CMD_BIO;
		readop = 0;
	}

	/* Cheat with the sector length to avoid a non-constant division */
	/* Fill the LBA and Transfer length in CDB */
	pass->mpf_header.mfh_target_id = link->target;
	pass->mpf_header.mfh_lun_id = 0;
	pass->mpf_header.mfh_timeout = 0;
	pass->mpf_header.mfh_flags = 0;
	pass->mpf_header.mfh_scsi_status = 0;
	pass->mpf_header.mfh_sense_len = MFI_SENSE_LEN;
	pass->mpf_header.mfh_data_len = xs->datalen;
	pass->mpf_header.mfh_cdb_len = xs->cmdlen;
	pass->mpf_sense_addr_lo = (uint32_t)cm->cm_sense_busaddr;
	pass->mpf_sense_addr_hi = (uint32_t)((uint64_t)cm->cm_sense_busaddr >> 32);
	cm->cm_complete = mfi_scsi_xs_done_cm;
	cm->cm_private = xs;
	if(xs->data){
//	xs->data = (void *)0x87048000;
	cm->cm_data = xs->data;
	cm->cm_len =  xs->datalen;
		

	memset(xs->data,0,cm->cm_len);
	}
	cm->cm_sg = &pass->mpf_sgl;
	cm->cm_total_frame_size = MFI_PASS_FRAME_SIZE;
	cm->cm_flags = flags;

	return (cm);
}


static struct mfi_command *
mfi_build_ldio(struct mfi_command *cm, struct scsi_xfer *xs,uint64_t blockno,uint32_t blockcnt)
{
	struct mfi_io_frame *io;
	//struct mfi_command *cm = sc;
	int flags;
	uint32_t blkcount;
	uint32_t context = 0;
	struct scsi_link	*link = xs->sc_link;

	struct mfi_softc	*sc = link->adapter_softc;

//printf("mfi_build_ldio start\n");

	/* Zero out the MFI frame */
	context = cm->cm_frame->mfr_header.mfh_context;
	bzero(cm->cm_frame, sizeof(union mfi_frame));
	cm->cm_frame->mfr_header.mfh_context = context;
	io = &cm->cm_frame->mfr_io;


	if (xs->flags & SCSI_DATA_IN) {
		io->mif_header.mfh_cmd = MFI_CMD_LD_READ;
		flags = MFI_CMD_DATAIN | MFI_CMD_BIO;
	}else
		{
		io->mif_header.mfh_cmd = MFI_CMD_LD_WRITE;
		flags = MFI_CMD_DATAOUT | MFI_CMD_BIO;}

	/* Cheat with the sector length to avoid a non-constant division */
	io->mif_header.mfh_target_id = link->target;
	io->mif_header.mfh_timeout = 0;
	io->mif_header.mfh_flags = 0;
	io->mif_header.mfh_scsi_status = 0;
	io->mif_header.mfh_sense_len = MFI_SENSE_LEN;
	io->mif_header.mfh_data_len = blockcnt;
	io->mif_sense_addr_lo = (uint32_t)cm->cm_sense_busaddr;
	io->mif_sense_addr_hi = (uint32_t)((uint64_t)cm->cm_sense_busaddr >> 32);
	io->mif_lba_hi = (blockno & 0xffffffff00000000) >> 32;
	io->mif_lba_lo = blockno & 0xffffffff;

	DNPRINTF("cm->index %x ,context %x \n",cm->cm_index,context);
	DNPRINTF("sizeof (union *) %x\n",sizeof(union mfi_frame));
	DNPRINTF(" sc->mfi_frames :%x, sc->mfi_frames_busaddr:%x\n",sc->mfi_frames, sc->mfi_frames_busaddr);
	DNPRINTF("pmon cm->cm_frame : %x cm->cm_frame_busaddr %x\n",cm->cm_frame,cm->cm_frame_busaddr);
	DNPRINTF("cm->cm_sense is %x,cm->cm_sense_busaddr %x\n",cm->cm_sense,cm->cm_sense_busaddr);
	DNPRINTF("io->header.target_id is %x\n",io->header.target_id);
	DNPRINTF("io->header.sense_len %x\n",io->header.sense_len);

	cm->cm_complete = mfi_scsi_xs_done_cm;
	cm->cm_private = xs;
	cm->cm_data = xs->data;
	cm->cm_len = xs->datalen;
//	cm->cm_len = 64;
#if 0
	printf("xs->data is %x\n",xs->data);
	{
	int	tem_i , tem_j;
		for(tem_i =446;tem_i<510; tem_i+=0x10 )
		{
			 for(tem_j=0; tem_j<0x10;tem_j++)
			printf("%02x ",xs->data[tem_i+tem_j]);
			printf("\n");
		}
	}
#endif
	cm->cm_sg = &io->mif_sgl;
	cm->cm_total_frame_size = MFI_IO_FRAME_SIZE;
	cm->cm_flags = flags;
	return 0;
//	return (cm);
}


void
mfi_scsi_xs_done_cm(struct mfi_command *cm)
{
	struct scsi_xfer	*xs = cm->cm_private;
	struct mfi_softc	*sc = cm->cm_sc;
	struct mfi_frame_header	*hdr = &cm->cm_frame->mfr_header;

	DNPRINTF(MFI_D_INTR, "%s: mfi_scsi_xs_done %#x %#x\n",
	    DEVNAME(sc), cm, cm->cm_frame);
#if 0

	if (xs->data != NULL) {
		DNPRINTF(MFI_D_INTR, "%s: mfi_scsi_xs_done sync\n",
		    DEVNAME(sc));
		bus_dmamap_sync(sc->sc_dmat, cm->cm_dmamap, 0,
		    cm->cm_dmamap->dm_mapsize,
		    (xs->flags & SCSI_DATA_IN) ?
		    BUS_DMASYNC_POSTREAD : BUS_DMASYNC_POSTWRITE);

		bus_dmamap_unload(sc->sc_dmat, cm->cm_dmamap);
	}
#endif
	DNPRINTF("mfi_scsi_xs_done_cm status %x\n",hdr->cmd_status);
	switch (hdr->mfh_cmd_status) {
	case MFI_STAT_OK:
		xs->resid = 0;
		break;

	case MFI_STAT_SCSI_DONE_WITH_ERROR:
		xs->error = XS_SENSE;
		xs->resid = 0;
		memset(&xs->sense, 0, sizeof(xs->sense));
		memcpy(&xs->sense, cm->cm_sense, sizeof(xs->sense));
		break;

	default:
		//xs->error = XS_DRIVER_STUFFUP;
		xs->error = XS_BUSY;
		printf("%s: mfi_scsi_xs_done stuffup %#x\n",
		    DEVNAME(sc), hdr->mfh_cmd_status);

		if (hdr->mfh_scsi_status != 0) {
/*			DNPRINTF(MFI_D_INTR,
			    "%s: mfi_scsi_xs_done sense %#x %x %x\n",
			    DEVNAME(sc), hdr->mfh_scsi_status,
			    &xs->sense, cm->cm_sense);*/
			memset(&xs->sense, 0, sizeof(xs->sense));
			memcpy(&xs->sense, cm->cm_sense,
			    sizeof(struct scsi_sense_data));
			xs->error = XS_SENSE;
		}
		break;
	}

	scsi_done(xs);
}



/*  xs->sc_link->adapter->cmd entry point. */
void
mfi_scsi_cmd_fury(struct scsi_xfer *xs)
{
	struct scsi_link	*link = xs->sc_link;
	struct mfi_softc	*sc = link->adapter_softc;
	struct device		*dev = link->device_softc;
	//struct mfi_ccb		*ccb = xs->io;
	struct mfi_command		*cm= xs->io;
	struct scsi_rw		*rw;
	struct scsi_rw_big	*rwb;
	struct scsi_rw_16	*rw16;
	uint64_t		blockno;
	uint32_t		blockcnt;
	uint8_t			target = link->target;
	uint8_t			mbox[MFI_MBOX_SIZE];

	DNPRINTF("%s: mfi_scsi_cmd opcode: %#x\n",
	    DEVNAME(sc), xs->cmd->opcode);

#if 0
	if (target >= MFI_MAX_LD || !sc->sc_ld[target].ld_present ||
	    link->lun != 0) {
		DNPRINTF(MFI_D_CMD, "%s: invalid target %d\n",
		    DEVNAME(sc), target);
		goto stuffup;
	}
#endif
	xs->error = XS_NOERROR;
	switch (xs->cmd->opcode) {
	/* IO path */
#if 1
	case READ_BIG:
	case WRITE_BIG:
		rwb = (struct scsi_rw_big *)xs->cmd;
#if 0
		printf("xs->cmdlen is %x\n",xs->cmdlen);
	
		printf("xs->cmd :\n");
		{
			int	tem_i , tem_j;
		for(tem_i =0;tem_i<xs->cmdlen; tem_i+=1 )
		{
			printf("%x ",*(((char *)xs->cmd)+tem_i));
		}
		printf("\n");
		}

#endif	
		blockno = (uint64_t)_4btol(rwb->addr);
		blockcnt = _2btol(rwb->length);
		DNPRINTF("rwb : %x  blockno : %llx blockcnt : %x\n",rwb,blockno,blockcnt);
		//if (mfi_scsi_io(cm, xs, blockno, blockcnt))
		if (mfi_build_ldio(cm, xs, blockno, blockcnt))
			goto stuffup;
		break;
#endif
	case READ_COMMAND:
	case WRITE_COMMAND:
		rw = (struct scsi_rw *)xs->cmd;
		blockno =
		    (uint64_t)(_3btol(rw->addr) & (SRW_TOPADDR << 16 | 0xffff));
		blockcnt = rw->length ? rw->length : 0x100;
		//if (mfi_scsi_io(cm, xs, blockno, blockcnt))
		if (mfi_build_ldio(cm, xs, blockno, blockcnt))
			goto stuffup;
		break;

	case READ_16:
	case WRITE_16:
		rw16 = (struct scsi_rw_16 *)xs->cmd;
		blockno = _8btol(rw16->addr);
		blockcnt = _4btol(rw16->length);
		//if (mfi_scsi_io(cm, xs, blockno, blockcnt))
		if (mfi_build_ldio(cm, xs, blockno, blockcnt))
			goto stuffup;
		break;

	case SYNCHRONIZE_CACHE:
		mbox[0] = MR_FLUSH_CTRL_CACHE | MR_FLUSH_DISK_CACHE;
		if( mfi_dcmd_command(sc, &cm, MR_DCMD_CTRL_CACHE_FLUSH,
	    NULL, 3))
//		if (mfi_do_mgmt(sc, cm, MR_DCMD_CTRL_CACHE_FLUSH,
//		    MFI_DATA_NONE, 0, NULL, mbox))
			goto stuffup;

		mfi_mapcmd(sc,cm);
		goto complete;
		/* NOTREACHED */

	/* hand it of to the firmware and let it deal with it */
	case TEST_UNIT_READY:
		/* save off sd? after autoconf */
		if (!cold)	/* XXX bogus */
			strlcpy(sc->sc_ld[target].ld_dev, dev->dv_xname,
			    sizeof(sc->sc_ld[target].ld_dev));
		/* FALLTHROUGH */

	default:
		//if (mfi_scsi_ld(cm, xs))
		if (mfi_build_syspdio(cm, xs)==NULL)
			goto stuffup;
		break;
	}

	DNPRINTF("%s: start io %d\n", DEVNAME(sc), target);
	DNPRINTF("%s: start io %d\n", DEVNAME(sc), target);

	if (xs->flags & SCSI_POLL) {
		if(mfi_mapcmd(sc,cm)){
//		if (mfi_poll(ccb)) {
			/* XXX check for sense in ccb->ccb_sense? */
			printf("%s: mfi_scsi_cmd poll failed\n",
			    DEVNAME(sc));
			bzero(&xs->sense, sizeof(xs->sense));
			xs->sense.error_code = SSD_ERRCODE_VALID |
			    SSD_ERRCODE_CURRENT;
			xs->sense.flags = SKEY_ILLEGAL_REQUEST;
			xs->sense.add_sense_code = 0x20; /* invalid opcode */
			xs->error = XS_SENSE;
		}

#if 0
		printf("xs->data is %x\n",xs->data);
	{
	int	tem_i , tem_j;
		for(tem_i =0;tem_i<0x24; tem_i+=0x10 )
		{
			 for(tem_j=0; tem_j<0x10;tem_j++)
			printf("%02x ",xs->data[tem_i+tem_j]);
			printf("\n");
		}
	}
#endif
		scsi_done(xs);
		return;
	}
	if(mfi_mapcmd(sc,cm)){
//		if (mfi_poll(ccb)) {
			/* XXX check for sense in ccb->ccb_sense? */
			printf("%s: mfi_scsi_cmd poll failed\n",
			    DEVNAME(sc));
			}
//	mfi_send_frame(sc,cm);
//	mfi_start(sc, ccb);
	//DNPRINTF(MFI_D_DMA, "%s: mfi_scsi_cmd queued %d\n", DEVNAME(sc),
	//	ccb->ccb_dmamap->dm_nsegs);
	return;

stuffup:
	xs->error = XS_DRIVER_STUFFUP;
complete:
	scsi_done(xs);
}

int
mfi_dcmd_command(struct mfi_softc *sc, struct mfi_command **cmp,
    uint32_t opcode, void **bufp, size_t bufsize)
{
	struct mfi_command *cm;
	struct mfi_dcmd_frame *dcmd;
	void *buf = NULL;
	uint32_t context = 0;


	cm = mfi_dequeue_free(sc);


	DNPRINTF("cm id:%x cm->frame:%x cm->sense:%x\n\n ",cm->cm_index,cm->cm_frame,cm->cm_sense);
	DNPRINTF("cm context:%x cm->frame_busaddr:%x cm->sense_busaddr:%x\n\n ",cm->cm_frame->mfr_header.mfh_context,cm->cm_frame_busaddr,cm->cm_sense_busaddr);






	if (cm == NULL)
		return (EBUSY);

	/* Zero out the MFI frame */
	context = cm->cm_frame->mfr_header.mfh_context;
	bzero(cm->cm_frame, sizeof(union mfi_frame));
	cm->cm_frame->mfr_header.mfh_context = context;

	if ((bufsize > 0) && (bufp != NULL)) {
		if (*bufp == NULL) {
			buf = malloc(bufsize, M_DEVBUF, M_NOWAIT|M_ZERO);
			if (buf == NULL) {
				mfi_release_command(cm);
				return (ENOMEM);
			}
			*bufp = buf;
		} else {
			buf = *bufp;
		}
	}

	dcmd =  &cm->cm_frame->mfr_dcmd;
	bzero(dcmd->mdf_mbox, MFI_MBOX_SIZE);
	dcmd->mdf_header.mfh_cmd = MFI_CMD_DCMD;
	dcmd->mdf_header.mfh_timeout = 0;
	dcmd->mdf_header.mfh_flags = 0;
	dcmd->mdf_header.mfh_data_len = bufsize;
	dcmd->mdf_header.mfh_scsi_status = 0;
	dcmd->mdf_opcode = opcode;
//	dcmd->mbox[0]=1;
	cm->cm_sg = &dcmd->mdf_sgl;
	cm->cm_total_frame_size = MFI_DCMD_FRAME_SIZE;
	cm->cm_flags = 0;
	cm->cm_data = buf;
	cm->cm_private = buf;
	cm->cm_len = bufsize;

	*cmp = cm;
	if ((bufp != NULL) && (*bufp == NULL) && (buf != NULL))
		*bufp = buf;
	return (0);
}



static int
mfi_get_controller_info(struct mfi_softc *sc)
{
	struct mfi_command *cm = NULL;
	struct mfi_ctrl_info *ci = NULL;
	uint32_t max_sectors_1, max_sectors_2;
	int error;

	ci = sc->sc_info;
	//ci = malloc(sizeof(struct mfi_ctrl_info), M_DEVBUF, M_NOWAIT|M_ZERO);
//	printf("ci_info is %x\n",ci);
	error = mfi_dcmd_command(sc, &cm, MR_DCMD_CTRL_GET_INFO,
	    (void **)&ci, sizeof(struct mfi_ctrl_info));
	if (error)
		goto out;
	
	cm->cm_flags = MFI_CMD_DATAIN | MFI_CMD_POLLED;

	//cm->cm_flags = MFI_CMD_DATAIN ;
#if 1
	if ((error = mfi_mapcmd(sc, cm)) != 0) {
		device_printf(sc->mfi_dev, "Failed to get controller info\n");
		sc->mfi_max_io = (sc->mfi_max_sge - 1) * PAGE_SIZE /
		    MFI_SECTOR_LEN;
		error = 0;
		goto out;
	}
#endif
#if 0
	bus_dmamap_sync(sc->sc_dmat, cm->cm_dmamap,0,cm->cm_dmamap->dm_mapsize,
	    BUS_DMASYNC_POSTREAD);
//	bus_dmamap_sync(sc->sc_dmat, cm->cm_dmamap,0,sizeof(*ci),
//	    BUS_DMASYNC_POSTREAD);
	bus_dmamap_unload(sc->sc_dmat, cm->cm_dmamap);
#endif

//	memcpy(&sc->sc_info,ci,sizeof(struct mfi_ctrl_info));
//	free(ci);
#if 0
	int *tem_addr, tem_i;
	tem_addr=ci;
		for(tem_i=0;tem_i<0x10;tem_i++)
		printf("tem_addr[%x]=%x\n",tem_i,tem_addr[tem_i]);
#endif
	max_sectors_1 = (1 << ci->mci_stripe_sz_ops.max) * ci->mci_max_strips_per_io;
	max_sectors_2 = ci->mci_max_request_size;
	sc->mfi_max_io = min(max_sectors_1, max_sectors_2);
	sc->disableOnlineCtrlReset =
	    ci->mci_properties.OnOffProperties.disableOnlineCtrlReset;

out:
//	if (ci)
//		free(ci, M_DEVBUF);
	if (cm)
		mfi_release_command(cm);
	return (error);
}







int
mfi_scsi_ioctl_fury(struct scsi_link *link, u_long cmd, caddr_t addr, int flag)
{
	struct mfi_softc	*sc = (struct mfi_softc *)link->adapter_softc;

	DNPRINTF(MFI_D_IOCTL, "%s: mfi_scsi_ioctl\n", DEVNAME(sc));

	if (sc->sc_ioctl)
		return (sc->sc_ioctl(link->adapter_softc, cmd, addr));
	else
		return (ENOTTY);
}

#if NBIO > 0
int
mfi_ioctl(struct device *dev, u_long cmd, caddr_t addr)
{
	struct mfi_softc	*sc = (struct mfi_softc *)dev;
	int error = 0;

	DNPRINTF(MFI_D_IOCTL, "%s: mfi_ioctl ", DEVNAME(sc));

	rw_enter_write(&sc->sc_lock);

	switch (cmd) {
	case BIOCINQ:
		DNPRINTF(MFI_D_IOCTL, "inq\n");
		error = mfi_ioctl_inq(sc, (struct bioc_inq *)addr);
		break;

	case BIOCVOL:
		DNPRINTF(MFI_D_IOCTL, "vol\n");
		error = mfi_ioctl_vol(sc, (struct bioc_vol *)addr);
		break;

	case BIOCDISK:
		DNPRINTF(MFI_D_IOCTL, "disk\n");
		error = mfi_ioctl_disk(sc, (struct bioc_disk *)addr);
		break;

	case BIOCALARM:
		DNPRINTF(MFI_D_IOCTL, "alarm\n");
		error = mfi_ioctl_alarm(sc, (struct bioc_alarm *)addr);
		break;

	case BIOCBLINK:
		DNPRINTF(MFI_D_IOCTL, "blink\n");
		error = mfi_ioctl_blink(sc, (struct bioc_blink *)addr);
		break;

	case BIOCSETSTATE:
		DNPRINTF(MFI_D_IOCTL, "setstate\n");
		error = mfi_ioctl_setstate(sc, (struct bioc_setstate *)addr);
		break;

	default:
		DNPRINTF(MFI_D_IOCTL, " invalid ioctl\n");
/*
 * skinny specific changes
 */
#define MFI_SKINNY_IDB	0x00	/* Inbound doorbell is at 0x00 for skinny */
#define MFI_IQPL	0x000000c0
#define MFI_IQPH	0x000000c4
#define MFI_SKINNY_RM	0x00000001	/* reply skinny message interrupt */


		error = EINVAL;
	}

	rw_exit_write(&sc->sc_lock);

	return (error);
}

int
mfi_bio_getitall(struct mfi_softc *sc)
{
	int			i, d, size, rv = EINVAL;
	uint8_t			mbox[MFI_MBOX_SIZE];
	struct mfi_conf		*cfg = NULL;
	struct mfi_ld_details	*ld_det = NULL;

	/* get info */
	if (mfi_get_info(sc)) {
		DNPRINTF(MFI_D_IOCTL, "%s: mfi_get_info failed\n",
		    DEVNAME(sc));
		goto done;
	}

	/* send single element command to retrieve size for full structure */
	cfg = malloc(sizeof *cfg, M_DEVBUF, M_NOWAIT | M_ZERO);
	if (cfg == NULL)
		goto done;
	if (mfi_mgmt(sc, MD_DCMD_CONF_GET, MFI_DATA_IN, sizeof *cfg, cfg,
	    NULL))
		goto done;

	size = cfg->mfc_size;
	free(cfg, M_DEVBUF);

	/* memory for read config */
	cfg = malloc(size, M_DEVBUF, M_NOWAIT | M_ZERO);
	if (cfg == NULL)
		goto done;
	if (mfi_mgmt(sc, MD_DCMD_CONF_GET, MFI_DATA_IN, size, cfg, NULL))
		goto done;

	/* replace current pointer with enw one */
	if (sc->sc_cfg)
		free(sc->sc_cfg, M_DEVBUF);
	sc->sc_cfg = cfg;

	/* get all ld info */
	if (mfi_mgmt(sc, MR_DCMD_LD_GET_LIST, MFI_DATA_IN,
	    sizeof(sc->sc_ld_list), &sc->sc_ld_list, NULL))
		goto done;

	/* get memory for all ld structures */
	size = cfg->mfc_no_ld * sizeof(struct mfi_ld_details);
	if (sc->sc_ld_sz != size) {
		if (sc->sc_ld_details)
			free(sc->sc_ld_details, M_DEVBUF);

		ld_det = malloc( size, M_DEVBUF, M_NOWAIT | M_ZERO);
		if (ld_det == NULL)
			goto done;
		sc->sc_ld_sz = size;
		sc->sc_ld_details = ld_det;
	}

	/* find used physical disks */
	size = sizeof(struct mfi_ld_details);
	for (i = 0, d = 0; i < cfg->mfc_no_ld; i++) {
		mbox[0] = sc->sc_ld_list.mll_list[i].mll_ld.mld_target;
		if (mfi_mgmt(sc, MR_DCMD_LD_GET_INFO, MFI_DATA_IN, size,
		    &sc->sc_ld_details[i], mbox))
			goto done;

		d += sc->sc_ld_details[i].mld_cfg.mlc_parm.mpa_no_drv_per_span *
		    sc->sc_ld_details[i].mld_cfg.mlc_parm.mpa_span_depth;
	}
	sc->sc_no_pd = d;

	rv = 0;
done:
	return (rv);
}

int
mfi_ioctl_inq(struct mfi_softc *sc, struct bioc_inq *bi)
{
	int			rv = EINVAL;
	struct mfi_conf		*cfg = NULL;

	DNPRINTF(MFI_D_IOCTL, "%s: mfi_ioctl_inq\n", DEVNAME(sc));

	if (mfi_bio_getitall(sc)) {
		DNPRINTF(MFI_D_IOCTL, "%s: mfi_bio_getitall failed\n",
		    DEVNAME(sc));
		goto done;
	}

	/* count unused disks as volumes */
	if (sc->sc_cfg == NULL)
		goto done;
	cfg = sc->sc_cfg;

	bi->bi_nodisk = sc->sc_info.mci_pd_disks_present;
	bi->bi_novol = cfg->mfc_no_ld + cfg->mfc_no_hs;
#if notyet
	bi->bi_novol = cfg->mfc_no_ld + cfg->mfc_no_hs +
	    (bi->bi_nodisk - sc->sc_no_pd);
#endif
	/* tell bio who we are */
	strlcpy(bi->bi_dev, DEVNAME(sc), sizeof(bi->bi_dev));

	rv = 0;
done:
	return (rv);
}

int
mfi_ioctl_vol(struct mfi_softc *sc, struct bioc_vol *bv)
{
	int			i, per, rv = EINVAL;

	DNPRINTF(MFI_D_IOCTL, "%s: mfi_ioctl_vol %#x\n",
	    DEVNAME(sc), bv->bv_volid);

	/* we really could skip and expect that inq took care of it */
	if (mfi_bio_getitall(sc)) {
		DNPRINTF(MFI_D_IOCTL, "%s: mfi_bio_getitall failed\n",
		    DEVNAME(sc));
		goto done;
	}

	if (bv->bv_volid >= sc->sc_ld_list.mll_no_ld) {
		/* go do hotspares & unused disks */
		rv = mfi_bio_hs(sc, bv->bv_volid, MFI_MGMT_VD, bv);
		goto done;
	}

	i = bv->bv_volid;
	strlcpy(bv->bv_dev, sc->sc_ld[i].ld_dev, sizeof(bv->bv_dev));

	switch(sc->sc_ld_list.mll_list[i].mll_state) {
	case MFI_LD_OFFLINE:
		bv->bv_status = BIOC_SVOFFLINE;
		break;

	case MFI_LD_PART_DEGRADED:
	case MFI_LD_DEGRADED:
		bv->bv_status = BIOC_SVDEGRADED;
		break;

	case MFI_LD_ONLINE:
		bv->bv_status = BIOC_SVONLINE;
		break;

	default:
		bv->bv_status = BIOC_SVINVALID;
		DNPRINTF(MFI_D_IOCTL, "%s: invalid logical disk state %#x\n",
		    DEVNAME(sc),
		    sc->sc_ld_list.mll_list[i].mll_state);
	}

	/* additional status can modify MFI status */
	switch (sc->sc_ld_details[i].mld_progress.mlp_in_prog) {
	case MFI_LD_PROG_CC:
	case MFI_LD_PROG_BGI:
		bv->bv_status = BIOC_SVSCRUB;
		per = (int)sc->sc_ld_details[i].mld_progress.mlp_cc.mp_progress;
		bv->bv_percent = (per * 100) / 0xffff;
		bv->bv_seconds =
		    sc->sc_ld_details[i].mld_progress.mlp_cc.mp_elapsed_seconds;
		break;

	case MFI_LD_PROG_FGI:
	case MFI_LD_PROG_RECONSTRUCT:
		/* nothing yet */
		break;
	}

	/*
	 * The RAID levels are determined per the SNIA DDF spec, this is only
	 * a subset that is valid for the MFI controller.
	 */
	bv->bv_level = sc->sc_ld_details[i].mld_cfg.mlc_parm.mpa_pri_raid;
	if (sc->sc_ld_details[i].mld_cfg.mlc_parm.mpa_sec_raid ==
	    MFI_DDF_SRL_SPANNED)
		bv->bv_level *= 10;

	bv->bv_nodisk = sc->sc_ld_details[i].mld_cfg.mlc_parm.mpa_no_drv_per_span *
	    sc->sc_ld_details[i].mld_cfg.mlc_parm.mpa_span_depth;

	bv->bv_size = sc->sc_ld_details[i].mld_size * 512; /* bytes per block */

	rv = 0;
done:
	return (rv);
}

int
mfi_ioctl_disk(struct mfi_softc *sc, struct bioc_disk *bd)
{
	struct mfi_conf		*cfg;
	struct mfi_array	*ar;
	struct mfi_ld_cfg	*ld;
	struct mfi_pd_details	*pd;
	struct scsi_inquiry_data *inqbuf;
	char			vend[8+16+4+1], *vendp;
	int			rv = EINVAL;
	int			arr, vol, disk, span;
	uint8_t			mbox[MFI_MBOX_SIZE];

	DNPRINTF(MFI_D_IOCTL, "%s: mfi_ioctl_disk %#x\n",
	    DEVNAME(sc), bd->bd_diskid);

	/* we really could skip and expect that inq took care of it */
	if (mfi_bio_getitall(sc)) {
		DNPRINTF(MFI_D_IOCTL, "%s: mfi_bio_getitall failed\n",
		    DEVNAME(sc));
		return (rv);
	}
	cfg = sc->sc_cfg;

	pd = malloc(sizeof *pd, M_DEVBUF, M_WAITOK);

	ar = cfg->mfc_array;
	vol = bd->bd_volid;
	if (vol >= cfg->mfc_no_ld) {
		/* do hotspares */
		rv = mfi_bio_hs(sc, bd->bd_volid, MFI_MGMT_SD, bd);
		goto freeme;
	}

	/* calculate offset to ld structure */
	ld = (struct mfi_ld_cfg *)(
	    ((uint8_t *)cfg) + offsetof(struct mfi_conf, mfc_array) +
	    cfg->mfc_array_size * cfg->mfc_no_array);

	/* use span 0 only when raid group is not spanned */
	if (ld[vol].mlc_parm.mpa_span_depth > 1)
		span = bd->bd_diskid / ld[vol].mlc_parm.mpa_no_drv_per_span;
	else
		span = 0;
	arr = ld[vol].mlc_span[span].mls_index;

	/* offset disk into pd list */
	disk = bd->bd_diskid % ld[vol].mlc_parm.mpa_no_drv_per_span;
	bd->bd_target = ar[arr].pd[disk].mar_enc_slot;

	/* get status */
	switch (ar[arr].pd[disk].mar_pd_state){
	case MFI_PD_UNCONFIG_GOOD:
	case MFI_PD_FAILED:
		bd->bd_status = BIOC_SDFAILED;
		break;

	case MFI_PD_HOTSPARE: /* XXX dedicated hotspare part of array? */
		bd->bd_status = BIOC_SDHOTSPARE;
		break;

	case MFI_PD_OFFLINE:
		bd->bd_status = BIOC_SDOFFLINE;
		break;

	case MFI_PD_REBUILD:
		bd->bd_status = BIOC_SDREBUILD;
		break;

	case MFI_PD_ONLINE:
		bd->bd_status = BIOC_SDONLINE;
		break;

	case MFI_PD_UNCONFIG_BAD: /* XXX define new state in bio */
	default:
		bd->bd_status = BIOC_SDINVALID;
		break;
	}

	/* get the remaining fields */
	*((uint16_t *)&mbox) = ar[arr].pd[disk].mar_pd.mfp_id;
	if (mfi_mgmt(sc, MR_DCMD_PD_GET_INFO, MFI_DATA_IN,
	    sizeof *pd, pd, mbox)) {
		/* disk is missing but succeed command */
		rv = 0;
		goto freeme;
	}

	bd->bd_size = pd->mpd_size * 512; /* bytes per block */

	/* if pd->mpd_enc_idx is 0 then it is not in an enclosure */
	bd->bd_channel = pd->mpd_enc_idx;

	inqbuf = (struct scsi_inquiry_data *)&pd->mpd_inq_data;
	vendp = inqbuf->vendor;
	memcpy(vend, vendp, sizeof vend - 1);
	vend[sizeof vend - 1] = '\0';
	strlcpy(bd->bd_vendor, vend, sizeof(bd->bd_vendor));

	/* XXX find a way to retrieve serial nr from drive */
	/* XXX find a way to get bd_procdev */

	rv = 0;
freeme:
	free(pd, M_DEVBUF);

	return (rv);
}

int
mfi_ioctl_alarm(struct mfi_softc *sc, struct bioc_alarm *ba)
{
	uint32_t		opc, dir = MFI_DATA_NONE;
	int			rv = 0;
	int8_t			ret;

	switch(ba->ba_opcode) {
	case BIOC_SADISABLE:
		opc = MR_DCMD_SPEAKER_DISABLE;
		break;

	case BIOC_SAENABLE:
		opc = MR_DCMD_SPEAKER_ENABLE;
		break;

	case BIOC_SASILENCE:
		opc = MR_DCMD_SPEAKER_SILENCE;
		break;

	case BIOC_GASTATUS:
		opc = MR_DCMD_SPEAKER_GET;
		dir = MFI_DATA_IN;
		break;

	case BIOC_SATEST:
		opc = MR_DCMD_SPEAKER_TEST;
		break;

	default:
		DNPRINTF(MFI_D_IOCTL, "%s: mfi_ioctl_alarm biocalarm invalid "
		    "opcode %x\n", DEVNAME(sc), ba->ba_opcode);
		return (EINVAL);
	}

	if (mfi_mgmt(sc, opc, dir, sizeof(ret), &ret, NULL))
		rv = EINVAL;
	else
		if (ba->ba_opcode == BIOC_GASTATUS)
			ba->ba_status = ret;
		else
			ba->ba_status = 0;

	return (rv);
}

int
mfi_ioctl_blink(struct mfi_softc *sc, struct bioc_blink *bb)
{
	int			i, found, rv = EINVAL;
	uint8_t			mbox[MFI_MBOX_SIZE];
	uint32_t		cmd;
	struct mfi_pd_list	*pd;

	DNPRINTF(MFI_D_IOCTL, "%s: mfi_ioctl_blink %x\n", DEVNAME(sc),
	    bb->bb_status);

	/* channel 0 means not in an enclosure so can't be blinked */
	if (bb->bb_channel == 0)
		return (EINVAL);

	pd = malloc(MFI_PD_LIST_SIZE, M_DEVBUF, M_WAITOK);

	if (mfi_mgmt(sc, MR_DCMD_PD_GET_LIST, MFI_DATA_IN,
	    MFI_PD_LIST_SIZE, pd, NULL))
		goto done;

	for (i = 0, found = 0; i < pd->mpl_no_pd; i++)
		if (bb->bb_channel == pd->mpl_address[i].mpa_enc_index &&
		    bb->bb_target == pd->mpl_address[i].mpa_enc_slot) {
		    	found = 1;
			break;
		}

	if (!found)
		goto done;

	memset(mbox, 0, sizeof mbox);

	*((uint16_t *)&mbox) = pd->mpl_address[i].mpa_pd_id;

	switch (bb->bb_status) {
	case BIOC_SBUNBLINK:
		cmd = MR_DCMD_PD_UNBLINK;
		break;

	case BIOC_SBBLINK:
		cmd = MR_DCMD_PD_BLINK;
		break;

	case BIOC_SBALARM:
	default:
		DNPRINTF(MFI_D_IOCTL, "%s: mfi_ioctl_blink biocblink invalid "
		    "opcode %x\n", DEVNAME(sc), bb->bb_status);
		goto done;
	}


	if (mfi_mgmt(sc, cmd, MFI_DATA_NONE, 0, NULL, mbox))
		goto done;

	rv = 0;
done:
	free(pd, M_DEVBUF);
	return (rv);
}

int
mfi_ioctl_setstate(struct mfi_softc *sc, struct bioc_setstate *bs)
{
	struct mfi_pd_list	*pd;
	int			i, found, rv = EINVAL;
	uint8_t			mbox[MFI_MBOX_SIZE];

	DNPRINTF(MFI_D_IOCTL, "%s: mfi_ioctl_setstate %x\n", DEVNAME(sc),
	    bs->bs_status);

	pd = malloc(MFI_PD_LIST_SIZE, M_DEVBUF, M_WAITOK);

	if (mfi_mgmt(sc, MR_DCMD_PD_GET_LIST, MFI_DATA_IN,
	    MFI_PD_LIST_SIZE, pd, NULL))
		goto done;

	for (i = 0, found = 0; i < pd->mpl_no_pd; i++)
		if (bs->bs_channel == pd->mpl_address[i].mpa_enc_index &&
		    bs->bs_target == pd->mpl_address[i].mpa_enc_slot) {
		    	found = 1;
			break;
		}

	if (!found)
		goto done;

	memset(mbox, 0, sizeof mbox);

	*((uint16_t *)&mbox) = pd->mpl_address[i].mpa_pd_id;

	switch (bs->bs_status) {
	case BIOC_SSONLINE:
		mbox[2] = MFI_PD_ONLINE;
		break;

	case BIOC_SSOFFLINE:
		mbox[2] = MFI_PD_OFFLINE;
		break;

	case BIOC_SSHOTSPARE:
		mbox[2] = MFI_PD_HOTSPARE;
		break;
/*
	case BIOC_SSREBUILD:
		break;
*/
	default:
		DNPRINTF(MFI_D_IOCTL, "%s: mfi_ioctl_setstate invalid "
		    "opcode %x\n", DEVNAME(sc), bs->bs_status);
		goto done;
	}


	if (mfi_mgmt(sc, MD_DCMD_PD_SET_STATE, MFI_DATA_NONE, 0, NULL, mbox))
		goto done;

	rv = 0;
done:
	free(pd, M_DEVBUF);
	return (rv);
}

int
mfi_bio_hs(struct mfi_softc *sc, int volid, int type, void *bio_hs)
{
	struct mfi_conf		*cfg;
	struct mfi_hotspare	*hs;
	struct mfi_pd_details	*pd;
	struct bioc_disk	*sdhs;
	struct bioc_vol		*vdhs;
	struct scsi_inquiry_data *inqbuf;
	char			vend[8+16+4+1], *vendp;
	int			i, rv = EINVAL;
	uint32_t		size;
	uint8_t			mbox[MFI_MBOX_SIZE];

	DNPRINTF(MFI_D_IOCTL, "%s: mfi_vol_hs %d\n", DEVNAME(sc), volid);

	if (!bio_hs)
		return (EINVAL);

	pd = malloc(sizeof *pd, M_DEVBUF, M_WAITOK);

	/* send single element command to retrieve size for full structure */
	cfg = malloc(sizeof *cfg, M_DEVBUF, M_WAITOK);
	if (mfi_mgmt(sc, MD_DCMD_CONF_GET, MFI_DATA_IN, sizeof *cfg, cfg, NULL))
		goto freeme;

	size = cfg->mfc_size;
	free(cfg, M_DEVBUF);

	/* memory for read config */
	cfg = malloc(size, M_DEVBUF, M_WAITOK|M_ZERO);
	if (mfi_mgmt(sc, MD_DCMD_CONF_GET, MFI_DATA_IN, size, cfg, NULL))
		goto freeme;

	/* calculate offset to hs structure */
	hs = (struct mfi_hotspare *)(
	    ((uint8_t *)cfg) + offsetof(struct mfi_conf, mfc_array) +
	    cfg->mfc_array_size * cfg->mfc_no_array +
	    cfg->mfc_ld_size * cfg->mfc_no_ld);

	if (volid < cfg->mfc_no_ld)
		goto freeme; /* not a hotspare */

	if (volid > (cfg->mfc_no_ld + cfg->mfc_no_hs))
		goto freeme; /* not a hotspare */

	/* offset into hotspare structure */
	i = volid - cfg->mfc_no_ld;

	DNPRINTF(MFI_D_IOCTL, "%s: mfi_vol_hs i %d volid %d no_ld %d no_hs %d "
	    "hs %p cfg %p id %02x\n", DEVNAME(sc), i, volid, cfg->mfc_no_ld,
	    cfg->mfc_no_hs, hs, cfg, hs[i].mhs_pd.mfp_id);

	/* get pd fields */
	memset(mbox, 0, sizeof mbox);
	*((uint16_t *)&mbox) = hs[i].mhs_pd.mfp_id;
	if (mfi_mgmt(sc, MR_DCMD_PD_GET_INFO, MFI_DATA_IN,
	    sizeof *pd, pd, mbox)) {
		DNPRINTF(MFI_D_IOCTL, "%s: mfi_vol_hs illegal PD\n",
		    DEVNAME(sc));
		goto freeme;
	}

	switch (type) {
	case MFI_MGMT_VD:
		vdhs = bio_hs;
		vdhs->bv_status = BIOC_SVONLINE;
		vdhs->bv_size = pd->mpd_size / 2 * 1024; /* XXX why? */
		vdhs->bv_level = -1; /* hotspare */
		vdhs->bv_nodisk = 1;
		break;

	case MFI_MGMT_SD:
		sdhs = bio_hs;
		sdhs->bd_status = BIOC_SDHOTSPARE;
		sdhs->bd_size = pd->mpd_size / 2 * 1024; /* XXX why? */
		sdhs->bd_channel = pd->mpd_enc_idx;
		sdhs->bd_target = pd->mpd_enc_slot;
		inqbuf = (struct scsi_inquiry_data *)&pd->mpd_inq_data;
		vendp = inqbuf->vendor;
		memcpy(vend, vendp, sizeof vend - 1);
		vend[sizeof vend - 1] = '\0';
		strlcpy(sdhs->bd_vendor, vend, sizeof(sdhs->bd_vendor));
		break;

	default:
		goto freeme;
	}

	DNPRINTF(MFI_D_IOCTL, "%s: mfi_vol_hs 6\n", DEVNAME(sc));
	rv = 0;
freeme:
	free(pd, M_DEVBUF);
	free(cfg, M_DEVBUF);

	return (rv);
}

#ifndef SMALL_KERNEL
int
mfi_create_sensors(struct mfi_softc *sc)
{
	struct device		*dev;
	struct scsibus_softc	*ssc = NULL;
	struct scsi_link	*link;
	int			i;

	TAILQ_FOREACH(dev, &alldevs, dv_list) {
		if (dev->dv_parent != &sc->sc_dev)
			continue;

		/* check if this is the scsibus for the logical disks */
		ssc = (struct scsibus_softc *)dev;
		if (ssc->adapter_link == &sc->sc_link)
			break;
	}

	if (ssc == NULL)
		return (1);

	sc->sc_sensors = malloc(sizeof(struct ksensor) * sc->sc_ld_cnt,
	    M_DEVBUF, M_WAITOK | M_CANFAIL | M_ZERO);
	if (sc->sc_sensors == NULL)
		return (1);

	strlcpy(sc->sc_sensordev.xname, DEVNAME(sc),
	    sizeof(sc->sc_sensordev.xname));

	for (i = 0; i < sc->sc_ld_cnt; i++) {
		link = scsi_get_link(ssc, i, 0);
		if (link == NULL)
			goto bad;

		dev = link->device_softc;

		sc->sc_sensors[i].type = SENSOR_DRIVE;
		sc->sc_sensors[i].status = SENSOR_S_UNKNOWN;

		strlcpy(sc->sc_sensors[i].desc, dev->dv_xname,
		    sizeof(sc->sc_sensors[i].desc));

		sensor_attach(&sc->sc_sensordev, &sc->sc_sensors[i]);
	}

	if (sensor_task_register(sc, mfi_refresh_sensors, 10) == NULL)
		goto bad;

	sensordev_install(&sc->sc_sensordev);

	return (0);

bad:
	free(sc->sc_sensors, M_DEVBUF);

	return (1);
}

void
mfi_refresh_sensors(void *arg)
{
	struct mfi_softc	*sc = arg;
	int			i;
	struct bioc_vol		bv;


	for (i = 0; i < sc->sc_ld_cnt; i++) {
		bzero(&bv, sizeof(bv));
		bv.bv_volid = i;
		if (mfi_ioctl_vol(sc, &bv))
			return;

		switch(bv.bv_status) {
		case BIOC_SVOFFLINE:
			sc->sc_sensors[i].value = SENSOR_DRIVE_FAIL;
			sc->sc_sensors[i].status = SENSOR_S_CRIT;
			break;

		case BIOC_SVDEGRADED:
			sc->sc_sensors[i].value = SENSOR_DRIVE_PFAIL;
			sc->sc_sensors[i].status = SENSOR_S_WARN;
			break;

		case BIOC_SVSCRUB:
		case BIOC_SVONLINE:
			sc->sc_sensors[i].value = SENSOR_DRIVE_ONLINE;
			sc->sc_sensors[i].status = SENSOR_S_OK;
			break;

		case BIOC_SVINVALID:
			/* FALLTRHOUGH */
		default:
			sc->sc_sensors[i].value = 0; /* unknown */
			sc->sc_sensors[i].status = SENSOR_S_UNKNOWN;
		}

	}
}
#endif /* SMALL_KERNEL */
#endif /* NBIO > 0 */
#if 1

void
mfi_complete(struct mfi_softc *sc, struct mfi_command *cm)
{
	int dir;
#if 0
	if ((cm->cm_flags & MFI_CMD_MAPPED) != 0) {
		dir = 0;
		if ((cm->cm_flags & MFI_CMD_DATAIN) ||
		    (cm->cm_frame->header.cmd == MFI_CMD_STP))
			dir |= BUS_DMASYNC_POSTREAD;
		if (cm->cm_flags & MFI_CMD_DATAOUT)
			dir |= BUS_DMASYNC_POSTWRITE;

		bus_dmamap_sync(sc->mfi_buffer_dmat, cm->cm_dmamap, dir);
		bus_dmamap_unload(sc->mfi_buffer_dmat, cm->cm_dmamap);
		cm->cm_flags &= ~MFI_CMD_MAPPED;
	}
#endif
//printf("ok 8\n");
#if 0
	bus_dmamap_sync(sc->sc_dmat, MFIMEM_MAP(sc->sc_frames),
	    cm->ccb_pframe_offset, sc->sc_frames_size, BUS_DMASYNC_PREREAD);
#endif
	cm->cm_flags |= MFI_CMD_COMPLETED;
//printf("to cm->cm_complete is %x\n",cm->cm_complete !=NULL);
		if(cm->cm_complete !=NULL)
		cm->cm_complete(cm);
}


#endif
#if 0
u_int32_t
mfi_xscale_fw_state(struct mfi_softc *sc)
{
	return (mfi_read(sc, MFI_OMSG0));
}

void
mfi_xscale_intr_ena(struct mfi_softc *sc)
{
	mfi_write(sc, MFI_OMSK, MFI_ENABLE_INTR);
}

int
mfi_xscale_intr(struct mfi_softc *sc)
{
	u_int32_t status;

	status = mfi_read(sc, MFI_OSTS);
	if (!ISSET(status, MFI_OSTS_INTR_VALID))
		return (0);

	/* write status back to acknowledge interrupt */
	mfi_write(sc, MFI_OSTS, status);

	return (1);
}

void
mfi_xscale_post(struct mfi_softc *sc, struct mfi_ccb *ccb)
{
	mfi_write(sc, MFI_IQP, (ccb->ccb_pframe >> 3) |
	    ccb->ccb_extra_frames);
}

u_int32_t
mfi_ppc_fw_state(struct mfi_softc *sc)
{
	return (mfi_read(sc, MFI_OSP));
}

void
mfi_ppc_intr_ena(struct mfi_softc *sc)
{
	mfi_write(sc, MFI_ODC, 0xffffffff);
	mfi_write(sc, MFI_OMSK, ~0x80000004);
}

int
mfi_ppc_intr(struct mfi_softc *sc)
{
	u_int32_t status;

	status = mfi_read(sc, MFI_OSTS);
	if (!ISSET(status, MFI_OSTS_PPC_INTR_VALID))
		return (0);

	/* write status back to acknowledge interrupt */
	mfi_write(sc, MFI_ODC, status);

	return (1);
}

void
mfi_ppc_post(struct mfi_softc *sc, struct mfi_ccb *ccb)
{
	mfi_write(sc, MFI_IQP, 0x1 | ccb->ccb_pframe |
	    (ccb->ccb_extra_frames << 1));
}

u_int32_t
mfi_gen2_fw_state(struct mfi_softc *sc)
{
	return (mfi_read(sc, MFI_OSP));
}

void
mfi_gen2_intr_ena(struct mfi_softc *sc)
{
	mfi_write(sc, MFI_ODC, 0xffffffff);
	mfi_write(sc, MFI_OMSK, ~MFI_OSTS_GEN2_INTR_VALID);
}

int
mfi_gen2_intr(struct mfi_softc *sc)
{
	u_int32_t status;

	status = mfi_read(sc, MFI_OSTS);
	if (!ISSET(status, MFI_OSTS_GEN2_INTR_VALID))
		return (0);

	/* write status back to acknowledge interrupt */
	mfi_write(sc, MFI_ODC, status);

	return (1);
}

void
mfi_gen2_post(struct mfi_softc *sc, struct mfi_ccb *ccb)
{
	mfi_write(sc, MFI_IQP, 0x1 | ccb->ccb_pframe |
	    (ccb->ccb_extra_frames << 1));
}


#endif
//#define MFI_FUSION_ENABLE_INTERRUPT_MASK	(0x00000008)

u_int32_t
mfi_tbolt_fw_state(struct mfi_softc *sc)
{
	return (mfi_read(sc, MFI_OSP));
}

void
mfi_tbolt_intr_ena(struct mfi_softc *sc)
{
	mfi_write(sc, MFI_OMSK, ~MFI_FUSION_ENABLE_INTERRUPT_MASK);
	mfi_read(sc, MFI_OMSK);
}

int
mfi_tbolt_intr(struct mfi_softc *sc)
{
	u_int32_t status;

	status = mfi_read(sc, MFI_OSTS);
	if (!ISSET(status, MFI_FUSION_ENABLE_INTERRUPT_MASK))
		return (0);

	/* write status back to acknowledge interrupt */
	mfi_write(sc, MFI_OSTS, status);

	return (1);
}

void
mfi_tbolt_post(struct mfi_softc *sc, struct mfi_ccb *ccb)
{
	//mfi_write(sc, MFI_IQP, 0x1 | ccb->ccb_pframe |
	  //  (ccb->ccb_extra_frames << 1));
	bus_addr_t bus_add = ccb->ccb_pframe;
	bus_add |= (MFI_REQ_DESCRIPT_FLAGS_MFA
	    << MFI_REQ_DESCRIPT_FLAGS_TYPE_SHIFT);
	MFI_WRITE4(sc, MFI_IQPL, (uint32_t)bus_add);
	MFI_WRITE4(sc, MFI_IQPH, (uint32_t)((uint64_t)bus_add >> 32));
}

