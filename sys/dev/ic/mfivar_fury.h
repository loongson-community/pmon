#ifndef MFI_VAR_H

#define MFI_VAR_H

#ifdef MFI_DEBUG
extern uint32_t			mfi_debug;
#define DPRINTF(x...)		do {  printf(x); } while(0)
#define DNPRINTF(n,x...)	do {  printf(x); } while(0)
//#define device_printf(n,x...)   do {  printf(x); } while(0)
#define device_printf(n,x...)   
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
#define device_printf(n,x...)   
//#define DPRINTF(x...)		do {  printf(x); } while(0)
//#define DNPRINTF(n,x...)	do {  printf(x); } while(0)
//#define device_printf(n,x...)   do {  printf(x); } while(0)
#define	MFI_D_CMD		0x0001
#define	MFI_D_INTR		0x0002
#define	MFI_D_MISC		0x0004
#define	MFI_D_DMA		0x0008
#define	MFI_D_IOCTL		0x0010
#define	MFI_D_RW		0x0020
#define	MFI_D_MEM		0x0040
#define	MFI_D_CCB		0x0080

#endif


uint32_t	mfi_read(struct mfi_softc *, bus_size_t);
void		mfi_write(struct mfi_softc *, bus_size_t, uint32_t);
#define MFI_WRITE4 mfi_write
#define MFI_READ4 mfi_read
// TODO find the right definition
#define XXX_MFI_CMD_OP_INIT2                    0x9
/*
 * Request descriptor types
 */
#define MFI_REQ_DESCRIPT_FLAGS_LD_IO           0x7
#define MFI_REQ_DESCRIPT_FLAGS_MFA             0x1
#define MFI_REQ_DESCRIPT_FLAGS_TYPE_SHIFT	0x1
#define MFI_FUSION_FP_DEFAULT_TIMEOUT		0x14
#define MFI_LOAD_BALANCE_FLAG			0x1
#define MFI_DCMD_MBOX_PEND_FLAG			0x1

//#define MR_PROT_INFO_TYPE_CONTROLLER	0x08
#define	MEGASAS_SCSI_VARIABLE_LENGTH_CMD	0x7f
#define MEGASAS_SCSI_SERVICE_ACTION_READ32	0x9
#define MEGASAS_SCSI_SERVICE_ACTION_WRITE32	0xB
#define	MEGASAS_SCSI_ADDL_CDB_LEN   		0x18
#define MEGASAS_RD_WR_PROTECT_CHECK_ALL		0x20
#define MEGASAS_RD_WR_PROTECT_CHECK_NONE	0x60
#define MEGASAS_EEDPBLOCKSIZE			512
struct mfi_cmd_tbolt {
	union mfi_mpi2_request_descriptor *request_desc;
	struct mfi_mpi2_request_raid_scsi_io *io_request;
	bus_addr_t		io_request_phys_addr;
	bus_addr_t		sg_frame_phys_addr;
	bus_addr_t 		sense_phys_addr;
	MPI2_SGE_IO_UNION	*sg_frame;
	uint8_t			*sense;
	TAILQ_ENTRY(mfi_cmd_tbolt) next;
	/*
	 * Context for a MFI frame.
	 * Used to get the mfi cmd from list when a MFI cmd is completed
	 */
	uint32_t		sync_cmd_idx;
	uint16_t		index;
	uint8_t			status;
};

struct mfi_hwcomms {
	uint32_t		hw_pi;
	uint32_t		hw_ci;
	uint32_t		hw_reply_q[1];
};
#define	MEGASAS_MAX_NAME	32
//#define	MEGASAS_VERSION		"4.23"

struct mfi_softc;
struct ccb_hdr;

struct mfi_command {
	TAILQ_ENTRY(mfi_command) cm_link;
	time_t			cm_timestamp;
	struct mfi_softc	*cm_sc;
	union mfi_frame		*cm_frame;
	bus_addr_t		cm_frame_busaddr;
	struct mfi_sense	*cm_sense;
	bus_addr_t		cm_sense_busaddr;
	bus_dmamap_t		cm_dmamap;
	bus_addr_t		ccb_pframe_offset;
//	uint32_t		ccb_frame_size;
	union mfi_sgl		*cm_sg;
	void			*cm_data;
	int			cm_len;
	int			cm_stp_len;
	int			cm_total_frame_size;
	int			cm_extra_frames;
	int			cm_flags;
#define MFI_CMD_MAPPED		(1<<0)
#define MFI_CMD_DATAIN		(1<<1)
#define MFI_CMD_DATAOUT		(1<<2)
#define MFI_CMD_COMPLETED	(1<<3)
#define MFI_CMD_POLLED		(1<<4)
#define MFI_CMD_SCSI		(1<<5)
#define MFI_CMD_CCB		(1<<6)
#define	MFI_CMD_BIO		(1<<7)
#define MFI_CMD_TBOLT		(1<<8)
#define MFI_ON_MFIQ_FREE	(1<<9)
#define MFI_ON_MFIQ_READY	(1<<10)
#define MFI_ON_MFIQ_BUSY	(1<<11)
#define MFI_ON_MFIQ_MASK	(MFI_ON_MFIQ_FREE | MFI_ON_MFIQ_READY| \
    MFI_ON_MFIQ_BUSY)
#define MFI_CMD_FLAGS_FMT	"\20" \
    "\1MAPPED" \
    "\2DATAIN" \
    "\3DATAOUT" \
    "\4COMPLETED" \
    "\5POLLED" \
    "\6SCSI" \
    "\7BIO" \
    "\10TBOLT" \
    "\11Q_FREE" \
    "\12Q_READY" \
    "\13Q_BUSY"
	uint8_t			retry_for_fw_reset;
	void			(* cm_complete)(struct mfi_command *cm);
	void			*cm_private;
	int			cm_index;
	int			cm_error;
};



#define MFIQ_ADD(sc, qname)					\
	do {							\
		struct mfi_qstat *qs;				\
								\
		qs = &(sc)->mfi_qstat[qname];			\
		qs->q_length++;					\
		if (qs->q_length > qs->q_max)			\
			qs->q_max = qs->q_length;		\
	} while (0)

#define MFIQ_REMOVE(sc, qname)	(sc)->mfi_qstat[qname].q_length--

#define MFIQ_INIT(sc, qname)					\
	do {							\
		sc->mfi_qstat[qname].q_length = 0;		\
		sc->mfi_qstat[qname].q_max = 0;			\
	} while (0)

#define MFIQ_COMMAND_QUEUE(name, index)					\
	static __inline void						\
	mfi_initq_ ## name (struct mfi_softc *sc)			\
	{								\
		TAILQ_INIT(&sc->mfi_ ## name);				\
		MFIQ_INIT(sc, index);					\
	}								\
static __inline void *				\
	mfi_dequeue_ ## name (void *cs)			\
	{								\
		struct mfi_softc *sc=cs;							\
		struct mfi_command *cm;					\
	printf("i am in get cmd\n");								\
		if ((cm = TAILQ_FIRST(&sc->mfi_ ## name)) != NULL) {	\
			if ((cm->cm_flags & MFI_ON_ ## index) == 0) {	\
				panic("command %p not in queue, "	\
				    "flags = %#x, bit = %#x\n", cm,	\
				    cm->cm_flags, MFI_ON_ ## index);	\
			}						\
			TAILQ_REMOVE(&sc->mfi_ ## name, cm, cm_link);	\
			cm->cm_flags &= ~MFI_ON_ ## index;		\
			MFIQ_REMOVE(sc, index);				\
		}							\
		return (cm);						\
	}\
	static __inline void						\
	mfi_enqueue_ ## name (void *sc,void *cs)			\
	{								\
		struct mfi_command *cm=cs;							\
		if ((cm->cm_flags & MFI_ON_MFIQ_MASK) != 0) {		\
			panic("command %p is on another queue, "	\
			    "flags = %#x\n", cm, cm->cm_flags);		\
		}							\
		TAILQ_INSERT_TAIL(&cm->cm_sc->mfi_ ## name, cm, cm_link); \
		cm->cm_flags |= MFI_ON_ ## index;			\
		MFIQ_ADD(cm->cm_sc, index);				\
	}								\
	static __inline void						\
	mfi_requeue_ ## name (void *sc,struct mfi_command *cm)			\
	{								\
		if ((cm->cm_flags & MFI_ON_MFIQ_MASK) != 0) {		\
			panic("command %p is on another queue, "	\
			    "flags = %#x\n", cm, cm->cm_flags);		\
		}							\
		TAILQ_INSERT_HEAD(&cm->cm_sc->mfi_ ## name, cm, cm_link); \
		cm->cm_flags |= MFI_ON_ ## index;			\
		MFIQ_ADD(cm->cm_sc, index);				\
	}								\
									\
	static __inline void						\
	mfi_remove_ ## name (struct mfi_command *cm)			\
	{								\
		if ((cm->cm_flags & MFI_ON_ ## index) == 0) {		\
			panic("command %p not in queue, flags = %#x, " \
			    "bit = %#x\n", cm, cm->cm_flags,		\
			    MFI_ON_ ## index);				\
		}							\
		TAILQ_REMOVE(&cm->cm_sc->mfi_ ## name, cm, cm_link);	\
		cm->cm_flags &= ~MFI_ON_ ## index;			\
		MFIQ_REMOVE(cm->cm_sc, index);				\
	}								\
struct hack

//MFIQ_COMMAND_QUEUE(free, MFIQ_FREE);
MFIQ_COMMAND_QUEUE(ready, MFIQ_READY);
MFIQ_COMMAND_QUEUE(busy, MFIQ_BUSY);
#if 0
static __inline void *				\
	mfi_dequeue_ ## name (void *cs)			\
	{								\
		struct mfi_softc *sc=cs;							\
		struct mfi_command *cm;					\
	printf("i am in get cmd\n");								\
		if ((cm = TAILQ_FIRST(&sc->mfi_ ## name)) != NULL) {	\
			if ((cm->cm_flags & MFI_ON_ ## index) == 0) {	\
				panic("command %p not in queue, "	\
				    "flags = %#x, bit = %#x\n", cm,	\
				    cm->cm_flags, MFI_ON_ ## index);	\
			}						\
			TAILQ_REMOVE(&sc->mfi_ ## name, cm, cm_link);	\
			cm->cm_flags &= ~MFI_ON_ ## index;		\
			MFIQ_REMOVE(sc, index);				\
		}							\
		return (cm);						\
	}
#endif
static void	mfi_initq_free (struct mfi_softc *sc)			
	{								\
		TAILQ_INIT(&sc->mfi_free);				\
		MFIQ_INIT(sc, MFIQ_FREE);					\
	}								
static void *				
	mfi_dequeue_free(void *cs)			
	{								\
		struct mfi_softc *sc=cs;							\
		struct mfi_command *cm;					\
		if ((cm = TAILQ_FIRST(&sc->mfi_free)) != NULL) {	\
			if ((cm->cm_flags & MFI_ON_MFIQ_FREE) == 0) {	\
				panic("command %p not in queue, "	\
				    "flags = %#x, bit = %#x\n", cm,	\
				    cm->cm_flags, MFI_ON_MFIQ_FREE);	\
			}						\
			TAILQ_REMOVE(&sc->mfi_free, cm, cm_link);	\
			cm->cm_flags &= ~MFI_ON_MFIQ_FREE;		\
			MFIQ_REMOVE(sc, MFIQ_FREE);				\
		}							\
		return (cm);						\
	}
static void						\
	mfi_enqueue_free (void *sc,void *cs)			\
	{								\
		struct mfi_command *cm=cs;							\
		if ((cm->cm_flags & MFI_ON_MFIQ_MASK) != 0) {		\
			panic("command %p is on another queue, "	\
			    "flags = %#x\n", cm, cm->cm_flags);		\
		}							\
		TAILQ_INSERT_TAIL(&cm->cm_sc->mfi_free, cm, cm_link); \
		cm->cm_flags |= MFI_ON_MFIQ_FREE;			\
		MFIQ_ADD(cm->cm_sc, MFIQ_FREE);				\
	}								
static void						
	mfi_requeue_free (void *sc,struct mfi_command *cm)			\
	{								\
		if ((cm->cm_flags & MFI_ON_MFIQ_MASK) != 0) {		\
			panic("command %p is on another queue, "	\
			    "flags = %#x\n", cm, cm->cm_flags);		\
		}							\
		TAILQ_INSERT_HEAD(&cm->cm_sc->mfi_free, cm, cm_link); \
		cm->cm_flags |= MFI_ON_MFIQ_FREE;			\
		MFIQ_ADD(cm->cm_sc, MFIQ_FREE);				\
	}								\
									
static  void						\
	mfi_remove_free (struct mfi_command *cm)			\
	{								\
		if ((cm->cm_flags & MFI_ON_MFIQ_FREE) == 0) {		\
			panic("command %p not in queue, flags = %#x, " \
			    "bit = %#x\n", cm, cm->cm_flags,		\
			    MFI_ON_MFIQ_FREE);				\
		}							\
		TAILQ_REMOVE(&cm->cm_sc->mfi_free, cm, cm_link);	\
		cm->cm_flags &= ~MFI_ON_MFIQ_FREE;			\
		MFIQ_REMOVE(cm->cm_sc, MFIQ_FREE);				\
	}	
#if 0
 void *				
	mfi_dequeue_free (void *cs)			
	{								
		struct mfi_softc *sc=cs;				
		struct mfi_command *cm;					
	printf("i am in get cmd\n");					
		if ((cm = TAILQ_FIRST(&sc->mfi_free)) != NULL) {	
			if ((cm->cm_flags & MFI_ON_MFIQ_FREE) == 0) {	
				panic("command %p not in queue, "	
				    "flags = %#x, bit = %#x\n", cm,	
				    cm->cm_flags, MFI_ON_MFIQ_FREE);	
			}						
			TAILQ_REMOVE(&sc->mfi_free, cm, cm_link);	
			cm->cm_flags &= ~MFI_ON_MFIQ_FREE;		
			MFIQ_REMOVE(sc, MFIQ_FREE);				
		}							
		return (cm);						
	}
#endif

int	mfi_std_send_frame(struct mfi_softc*,struct mfi_command*);
int mfi_transition_firmware(struct mfi_softc *sc);
 void mfi_complete(struct mfi_softc *sc, struct mfi_command *cm);
 int mfi_mapcmd(struct mfi_softc *sc,struct mfi_command *cm);
extern void mfi_tbolt_enable_intr_ppc(struct mfi_softc *);
extern void mfi_tbolt_disable_intr_ppc(struct mfi_softc *);
extern int32_t mfi_tbolt_read_fw_status_ppc(struct mfi_softc *);
extern int32_t mfi_tbolt_check_clear_intr_ppc(struct mfi_softc *);
extern void mfi_tbolt_issue_cmd_ppc(struct mfi_softc *, bus_addr_t, uint32_t);
extern void mfi_tbolt_init_globals(struct mfi_softc*);
extern uint32_t mfi_tbolt_get_memory_requirement(struct mfi_softc *);
extern int mfi_tbolt_init_desc_pool(struct mfi_softc *, uint8_t *, uint32_t);
extern int mfi_tbolt_init_MFI_queue(struct mfi_softc *);
extern void mfi_intr_tbolt(void *arg);
extern int mfi_tbolt_alloc_cmd(struct mfi_softc *sc);
extern int mfi_tbolt_send_frame(struct mfi_softc *sc, struct mfi_command *cm);
extern int mfi_tbolt_adp_reset(struct mfi_softc *sc);
extern int mfi_tbolt_reset(struct mfi_softc *sc);
extern void mfi_tbolt_sync_map_info(struct mfi_softc *sc);
 int mfi_dcmd_command(struct mfi_softc *, struct mfi_command **,
     uint32_t, void **, size_t);


extern void mfi_release_command(void *);
extern void mfi_tbolt_return_cmd(struct mfi_softc *,
    struct mfi_cmd_tbolt *, struct mfi_command *);

union desc_value {
	uint64_t	word;
	struct {
		uint32_t	low;
		uint32_t	high;
	}u;
};


#define	READ_6			0x08
#define	WRITE_6			0x0A

#define	READ_10			0x28
#define	WRITE_10		0x2A
//#define	READ_12			0xA8
//#define	WRITE_12		0xAA
//#define	READ_16			0x88
//#define	WRITE_16		0x8A


struct scsi_rw_6
{
	u_int8_t opcode;
	u_int8_t addr[3];
/* only 5 bits are valid in the MSB address byte */
#define	SRW_TOPADDR	0x1F
	u_int8_t length;
	u_int8_t control;
};

struct scsi_rw_10
{
	u_int8_t opcode;
#define	SRW10_RELADDR	0x01
/* EBP defined for WRITE(10) only */
#define	SRW10_EBP	0x04
#define	SRW10_FUA	0x08
#define	SRW10_DPO	0x10
	u_int8_t byte2;
	u_int8_t addr[4];
	u_int8_t reserved;
	u_int8_t length[2];
	u_int8_t control;
};
struct scsi_write_atomic_16
{
	uint8_t	opcode;
	uint8_t	byte2;
	uint8_t	addr[8];
	uint8_t	boundary[2];
	uint8_t	length[2];
	uint8_t	group;
	uint8_t	control;
};

static __inline void
scsi_ulto2b(u_int32_t val, u_int8_t *bytes)
{

	bytes[0] = (val >> 8) & 0xff;
	bytes[1] = val & 0xff;
}

static __inline void
scsi_ulto3b(u_int32_t val, u_int8_t *bytes)
{

	bytes[0] = (val >> 16) & 0xff;
	bytes[1] = (val >> 8) & 0xff;
	bytes[2] = val & 0xff;
}

static __inline void
scsi_ulto4b(u_int32_t val, u_int8_t *bytes)
{

	bytes[0] = (val >> 24) & 0xff;
	bytes[1] = (val >> 16) & 0xff;
	bytes[2] = (val >> 8) & 0xff;
	bytes[3] = val & 0xff;
}

static __inline void
scsi_u64to8b(u_int64_t val, u_int8_t *bytes)
{

	bytes[0] = (val >> 56) & 0xff;
	bytes[1] = (val >> 48) & 0xff;
	bytes[2] = (val >> 40) & 0xff;
	bytes[3] = (val >> 32) & 0xff;
	bytes[4] = (val >> 24) & 0xff;
	bytes[5] = (val >> 16) & 0xff;
	bytes[6] = (val >> 8) & 0xff;
	bytes[7] = val & 0xff;
}

static __inline uint32_t
scsi_2btoul(const uint8_t *bytes)
{
	uint32_t rv;

	rv = (bytes[0] << 8) |
	     bytes[1];
	return (rv);
}

static __inline uint32_t
scsi_3btoul(const uint8_t *bytes)
{
	uint32_t rv;

	rv = (bytes[0] << 16) |
	     (bytes[1] << 8) |
	     bytes[2];
	return (rv);
}

static __inline int32_t 
scsi_3btol(const uint8_t *bytes)
{
	uint32_t rc = scsi_3btoul(bytes);
 
	if (rc & 0x00800000)
		rc |= 0xff000000;

	return (int32_t) rc;
}

static __inline uint32_t
scsi_4btoul(const uint8_t *bytes)
{
	uint32_t rv;

	rv = (bytes[0] << 24) |
	     (bytes[1] << 16) |
	     (bytes[2] << 8) |
	     bytes[3];
	return (rv);
}

static __inline uint64_t
scsi_8btou64(const uint8_t *bytes)
{
        uint64_t rv;
 
	rv = (((uint64_t)bytes[0]) << 56) |
	     (((uint64_t)bytes[1]) << 48) |
	     (((uint64_t)bytes[2]) << 40) |
	     (((uint64_t)bytes[3]) << 32) |
	     (((uint64_t)bytes[4]) << 24) |
	     (((uint64_t)bytes[5]) << 16) |
	     (((uint64_t)bytes[6]) << 8) |
	     bytes[7];
	return (rv);
}



static int
mfi_build_cdb(int readop, uint8_t byte2, u_int64_t lba, u_int32_t block_count, uint8_t *cdb)
{
	int cdb_len;

	if (((lba & 0x1fffff) == lba)
         && ((block_count & 0xff) == block_count)
         && (byte2 == 0)) {
		/* We can fit in a 6 byte cdb */
		struct scsi_rw_6 *scsi_cmd;

		scsi_cmd = (struct scsi_rw_6 *)cdb;
		scsi_cmd->opcode = readop ? READ_6 : WRITE_6;
		scsi_ulto3b(lba, scsi_cmd->addr);
		scsi_cmd->length = block_count & 0xff;
		scsi_cmd->control = 0;
		cdb_len = sizeof(*scsi_cmd);
	} else if (((block_count & 0xffff) == block_count) && ((lba & 0xffffffff) == lba)) {
		/* Need a 10 byte CDB */
		struct scsi_rw_10 *scsi_cmd;

		scsi_cmd = (struct scsi_rw_10 *)cdb;
		scsi_cmd->opcode = readop ? READ_10 : WRITE_10;
		scsi_cmd->byte2 = byte2;
		scsi_ulto4b(lba, scsi_cmd->addr);
		scsi_cmd->reserved = 0;
		scsi_ulto2b(block_count, scsi_cmd->length);
		scsi_cmd->control = 0;
		cdb_len = sizeof(*scsi_cmd);
	} else if (((block_count & 0xffffffff) == block_count) &&
	    ((lba & 0xffffffff) == lba)) {
		/* Block count is too big for 10 byte CDB use a 12 byte CDB */
		struct scsi_rw_12 *scsi_cmd;

		scsi_cmd = (struct scsi_rw_12 *)cdb;
		scsi_cmd->opcode = readop ? READ_12 : WRITE_12;
		scsi_cmd->byte2 = byte2;
		scsi_ulto4b(lba, scsi_cmd->addr);
		scsi_cmd->reserved = 0;
		scsi_ulto4b(block_count, scsi_cmd->length);
		scsi_cmd->control = 0;
		cdb_len = sizeof(*scsi_cmd);
	} else {
		/*
		 * 16 byte CDB.  We'll only get here if the LBA is larger
		 * than 2^32
		 */
		struct scsi_rw_16 *scsi_cmd;

		scsi_cmd = (struct scsi_rw_16 *)cdb;
		scsi_cmd->opcode = readop ? READ_16 : WRITE_16;
		scsi_cmd->byte2 = byte2;
		scsi_u64to8b(lba, scsi_cmd->addr);
		scsi_cmd->reserved = 0;
		scsi_ulto4b(block_count, scsi_cmd->length);
		scsi_cmd->control = 0;
		cdb_len = sizeof(*scsi_cmd);
	}

	return cdb_len;
}


#define MFI_RESET_WAIT_TIME 180
#define MFI_CMD_TIMEOUT 30
#define MFI_SYS_PD_IO	0
#define MFI_LD_IO	1
#define MFI_SKINNY_MEMORY 0x02000000
#define MFI_MAXPHYS (128 * 1024)


//#ifdef MFI_DEBUG
#if 0
extern void mfi_print_cmd(struct mfi_command *cm);
extern void mfi_dump_cmds(struct mfi_softc *sc);
extern void mfi_validate_sg(struct mfi_softc *, struct mfi_command *,
    const char *, int);
#define MFI_PRINT_CMD(cm)	mfi_print_cmd(cm)
#define MFI_DUMP_CMDS(sc)	mfi_dump_cmds(sc)
#define MFI_VALIDATE_CMD(sc, cm) mfi_validate_sg(sc, cm, __FUNCTION__, __LINE__)
#else
#define MFI_PRINT_CMD(cm)
#define MFI_DUMP_CMDS(sc)
#define MFI_VALIDATE_CMD(sc, cm)
#endif




#endif
