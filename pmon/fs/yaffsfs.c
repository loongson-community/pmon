/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2007 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <string.h>
#include <termio.h>
#include <fcntl.h>
#include <file.h>
#include <ctype.h>
#include <sys/unistd.h>
#include <stdlib.h>
#include <include/linux/mtd/mtd.h>
#include <sys/sys/stat.h>
#include <sys/queue.h>
#include <sys/dev/nand/fcr-nand.h>
#include <mtdfile.h>

const char *yaffsfs_c_version="$Id: yaffsfs.c,v 1.21 2008/07/03 20:06:05 charles Exp $";
//################################### definition #########################
#define YAFFSFS_MAX_SYMLINK_DEREFERENCES 5
#ifndef NULL
#define NULL ((void *)0)
#endif
#define YAFFSFS_N_HANDLES 200

#define YAFFS_OK	1
#define YAFFS_FAIL  0

#ifndef O_RDONLY
#define O_RDONLY        00
#endif

#ifndef O_WRONLY
#define O_WRONLY	01
#endif

#ifndef O_RDWR
#define O_RDWR		02
#endif

#ifndef O_CREAT		
#define O_CREAT 	0100
#endif

#ifndef O_EXCL
#define O_EXCL		0200
#endif

#ifndef O_TRUNC
#define O_TRUNC		01000
#endif

#ifndef O_APPEND
#define O_APPEND	02000
#endif

#ifndef SEEK_SET
#define SEEK_SET	0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR	1
#endif

#ifndef SEEK_END
#define SEEK_END	2
#endif

#ifndef EBUSY
#define EBUSY	16
#endif

#ifndef ENODEV
#define ENODEV	19
#endif

#ifndef EINVAL
#define EINVAL	22
#endif

#ifndef EBADF
#define EBADF	9
#endif

#ifndef EACCES
#define EACCES	13
#endif

#ifndef EXDEV	
#define EXDEV	18
#endif

#ifndef ENOENT
#define ENOENT	2
#endif

#ifndef ENOSPC
#define ENOSPC	28
#endif

#ifndef ENOTEMPTY
#define ENOTEMPTY 39
#endif

#ifndef ENOMEM
#define ENOMEM 12
#endif

#ifndef EEXIST
#define EEXIST 17
#endif

#ifndef ENOTDIR
#define ENOTDIR 20
#endif

#ifndef EISDIR
#define EISDIR 21
#endif


// Mode flags

#ifndef S_IFMT
#define S_IFMT		0170000
#endif

#ifndef S_IFLNK
#define S_IFLNK		0120000
#endif

#ifndef S_IFDIR
#define S_IFDIR		0040000
#endif

#ifndef S_IFREG
#define S_IFREG		0100000
#endif

#ifndef S_IREAD 
#define S_IREAD		0000400
#endif

#ifndef S_IWRITE
#define	S_IWRITE	0000200
#endif

#ifndef S_IEXEC
#define	S_IEXEC	0000100
#endif

#ifndef R_OK
#define R_OK	4
#define W_OK	2
#define X_OK	1
#define F_OK	0
#endif

/* Give us a  Y=0x59, 
 * Give us an A=0x41, 
 * Give us an FF=0xFF 
 * Give us an S=0x53
 * And what have we got... 
 */
#define YAFFS_MAGIC			0x5941FF53

#define YAFFS_NTNODES_LEVEL0	  	16
#define YAFFS_TNODES_LEVEL0_BITS	4
#define YAFFS_TNODES_LEVEL0_MASK	0xf

#define YAFFS_NTNODES_INTERNAL 		(YAFFS_NTNODES_LEVEL0 / 2)
#define YAFFS_TNODES_INTERNAL_BITS 	(YAFFS_TNODES_LEVEL0_BITS - 1)
#define YAFFS_TNODES_INTERNAL_MASK	0x7
#define YAFFS_TNODES_MAX_LEVEL		6

#define YAFFS_MAX_CHUNK_ID		0x000FFFFF

#define YAFFS_UNUSED_OBJECT_ID		0x0003FFFF

#define YAFFS_ALLOCATION_NOBJECTS	100
#define YAFFS_ALLOCATION_NTNODES	100
#define YAFFS_ALLOCATION_NLINKS		100

#define YAFFS_NOBJECT_BUCKETS		256


#define YAFFS_OBJECT_SPACE		0x40000

#define YAFFS_CHECKPOINT_VERSION 	3

#define YAFFS_MAX_NAME_LENGTH		255
#define YAFFS_MAX_ALIAS_LENGTH		159


#define YAFFS_SHORT_NAME_LENGTH		15

/* Some special object ids for pseudo objects */
#define YAFFS_OBJECTID_ROOT		1
#define YAFFS_OBJECTID_LOSTNFOUND	2
#define YAFFS_OBJECTID_UNLINKED		3
#define YAFFS_OBJECTID_DELETED		4

/* Sseudo object ids for checkpointing */
#define YAFFS_OBJECTID_SB_HEADER	0x10
#define YAFFS_OBJECTID_CHECKPOINT_DATA	0x20
#define YAFFS_SEQUENCE_CHECKPOINT_DATA  0x21

#define YAFFS_MAX_SHORT_OP_CACHES	20

#define YAFFS_N_TEMP_BUFFERS		6

#define YAFFS_LOSTNFOUND_NAME	"lost+found"
#define YAFFS_LOSTNFOUND_PREFIX	"obj"

#define YAFFS_ROOT_MODE		0666
#define YAFFS_LOSTNFOUND_MODE	0666

#define DT_UNKNOWN	0
#define DT_FIFO	0
#define DT_CHR	0
#define DT_DIR	0
#define DT_BLK	0
#define DT_REG	0
#define DT_LNK	0
#define DT_SOCK	0
#define DT_WHT	0



/* We limit the number attempts at sucessfully saving a chunk of data.
 * Small-page devices have 32 pages per block; large-page devices have 64.
 * Default to something in the order of 5 to 10 blocks worth of chunks.
 */
#define YAFFS_WR_ATTEMPTS		(5*64)

/* Sequence numbers are used in YAFFS2 to determine block allocation order.
 * The range is limited slightly to help distinguish bad numbers from good.
 * This also allows us to perhaps in the future use special numbers for
 * special purposes.
 * EFFFFF00 allows the allocation of 8 blocks per second (~1Mbytes) for 15 years, 
 * and is a larger number than the lifetime of a 2GB device.
 */
#define YAFFS_LOWEST_SEQUENCE_NUMBER	0x00001000
#define YAFFS_HIGHEST_SEQUENCE_NUMBER	0xEFFFFF00

/*definition for list*/
typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned __u32;

struct ylist_head {
	struct ylist_head *next; /* next in chain */
	struct ylist_head *prev; /* previous in chain */
};

/* Initialise a static list */
#define YLIST_HEAD(name) \
struct ylist_head name = { &(name),&(name)}

/* Initialise a list head to an empty list */
#define YINIT_LIST_HEAD(p) \
do { \
 (p)->next = (p);\
  (p)->prev = (p); \
  } while(0)

/* Add an element to a list */
static __inline__ void ylist_add(struct ylist_head *newEntry, struct ylist_head *list)
{
	struct ylist_head *listNext = list->next;
	
	list->next = newEntry;
	newEntry->prev = list;
	newEntry->next = listNext;
	listNext->prev = newEntry;
}

static __inline__ void ylist_add_tail(struct ylist_head *newEntry, struct ylist_head *list)
{
	struct ylist_head *listPrev = list->prev;
															
	list->prev = newEntry;
	newEntry->next = list;
	newEntry->prev = listPrev;
	listPrev->next = newEntry;
																
}

/* Take an element out of its current list, with or without
 * reinitialising the links.of the entry*/
static __inline__ void ylist_del(struct ylist_head *entry)
{
	struct ylist_head *listNext = entry->next;
	struct ylist_head *listPrev = entry->prev;
	listNext->prev = listPrev;
	listPrev->next = listNext;
}

static __inline__ void ylist_del_init(struct ylist_head *entry)
{
	ylist_del(entry);
	entry->next = entry->prev = entry;
}

/* Test if the list is empty */
static __inline__ int ylist_empty(struct ylist_head *entry)
{
	return (entry->next == entry);
}

/* ylist_entry takes a pointer to a list entry and offsets it to that
 * we can find a pointer to the object it is embedded in.
 */
#define ylist_entry(entry, type, member) \
	((type *)((char *)(entry)-(unsigned long)(&((type *)NULL)->member)))

/* ylist_for_each and list_for_each_safe  iterate over lists.
 * ylist_for_each_safe uses temporary storage to make the list delete safe
 */
#define ylist_for_each(itervar, list) \
	for (itervar = (list)->next; itervar != (list); itervar = itervar->next )

#define ylist_for_each_safe(itervar,saveVar, list) \
	for (itervar = (list)->next, saveVar = (list)->next->next; itervar != (list); \
		itervar = saveVar, saveVar = saveVar->next)


typedef enum {
	YAFFS_OBJECT_TYPE_UNKNOWN,
	YAFFS_OBJECT_TYPE_FILE,
	YAFFS_OBJECT_TYPE_SYMLINK,
	YAFFS_OBJECT_TYPE_DIRECTORY,
	YAFFS_OBJECT_TYPE_HARDLINK,
	YAFFS_OBJECT_TYPE_SPECIAL
} yaffs_ObjectType;

/* This is the object structure as stored on NAND */
typedef struct {
	yaffs_ObjectType type;
	
	/* Apply to everything  */
	int parentObjectId;
	__u16 sum__NoLongerUsed;        /* checksum of name. No longer used */
	char name[YAFFS_MAX_NAME_LENGTH + 1];

	/* The following apply to directories, files, symlinks - not hard links */
        __u32 yst_mode;         /* protection */
	__u32 yst_uid;
	__u32 yst_gid;
	__u32 yst_atime;
	__u32 yst_mtime;
	__u32 yst_ctime;

	/* File size  applies to files only */
	int fileSize;
	
	/* Equivalent object id applies to hard links only. */
	int equivalentObjectId;

	/* Alias is for symlinks only. */
	char alias[YAFFS_MAX_ALIAS_LENGTH + 1];

	__u32 yst_rdev;		/* device stuff for block and char devices (major/min) */

	__u32 roomToGrow[6];
	__u32 inbandShadowsObject;
	__u32 inbandIsShrink;
	__u32 reservedSpace[2];
	int shadowsObject;	/* This object header shadows the specified object if > 0 */

	/* isShrink applies to object headers written when we shrink the file (ie resize) */
	__u32 isShrink;
} yaffs_ObjectHeader;


typedef enum {
	YAFFS_ECC_RESULT_UNKNOWN,
	YAFFS_ECC_RESULT_NO_ERROR,
	YAFFS_ECC_RESULT_FIXED,
	YAFFS_ECC_RESULT_UNFIXED
} yaffs_ECCResult;

typedef struct {
	unsigned char colParity;
	unsigned lineParity;
	unsigned lineParityPrime;
} yaffs_ECCOther;


/*definition for tags*/
typedef struct {
	unsigned validMarker0;
	unsigned chunkUsed;	/*  Status of the chunk: used or unused */
	unsigned objectId;	/* If 0 then this is not part of an object (unused) */
	unsigned chunkId;	/* If 0 then this is a header, else a data chunk */
	unsigned byteCount;	/* Only valid for data chunks */
	
	/* The following stuff only has meaning when we read */
	yaffs_ECCResult eccResult;
	unsigned blockBad;	
	
	/* YAFFS 1 stuff */
	unsigned chunkDeleted;	/* The chunk is marked deleted */
	unsigned serialNumber;	/* Yaffs1 2-bit serial number */

	/* YAFFS2 stuff */
	unsigned sequenceNumber;	/* The sequence number of this block */
	
	/* Extra info if this is an object header (YAFFS2 only) */
	unsigned extraHeaderInfoAvailable;	/* There is extra info available if this is not zero */
	unsigned extraParentObjectId;	/* The parent object */
	unsigned extraIsShrinkHeader;	/* Is it a shrink header? */
	unsigned extraShadows;		/* Does this shadow another object? */
	yaffs_ObjectType extraObjectType;	/* What object type? */
	unsigned extraFileLength;		/* Length if it is a file */
	unsigned extraEquivalentObjectId;	/* Equivalent object Id if it is a hard link */
	
	unsigned validMarker1;
} yaffs_ExtendedTags;


typedef struct {
	unsigned sequenceNumber;
	unsigned objectId;
	unsigned chunkId;
	unsigned byteCount;
} yaffs_PackedTags2TagsPart;

typedef struct {
	yaffs_PackedTags2TagsPart t;
	yaffs_ECCOther ecc;
} yaffs_PackedTags2;

/* The CheckpointDevice structure holds the device information that changes at runtime and
 *  * must be preserved over unmount/mount cycles.
 *   */
typedef struct {
       int structType;
	int nErasedBlocks;
	int allocationBlock;	/* Current block being allocated off */
	__u32 allocationPage;
	int nFreeChunks;
	int nDeletedFiles;		/* Count of files awaiting deletion;*/
	int nUnlinkedFiles;		/* Count of unlinked files. */
	int nBackgroundDeletions;	/* Count of background deletions. */
	
	/* yaffs2 runtime stuff */
	unsigned sequenceNumber;	/* Sequence number of currently allocating block */
	unsigned oldestDirtySequence;
} yaffs_CheckpointDevice;

/* yaffs_CheckpointObject holds the definition of an object as dumped 
 *  * by checkpointing.
 *   */

typedef struct {
        int structType;
	__u32 objectId;		
	__u32 parentId;
	int hdrChunk;
	yaffs_ObjectType variantType:3;
	__u8 deleted:1;		
	__u8 softDeleted:1;	
	__u8 unlinked:1;	
	__u8 fake:1;		
	__u8 renameAllowed:1;
	__u8 unlinkAllowed:1;
	__u8 serial;		
	int nDataChunks;	
	__u32 fileSizeOrEquivalentObjectId;
} yaffs_CheckpointObject;

typedef struct {
	int structType;
	__u32 magic;
	__u32 version;
	__u32 head;
} yaffs_CheckpointValidity;

typedef enum {
	YAFFS_BLOCK_STATE_UNKNOWN = 0,

	YAFFS_BLOCK_STATE_SCANNING,
	YAFFS_BLOCK_STATE_NEEDS_SCANNING,
	/* The block might have something on it (ie it is allocating or full, perhaps empty)
	 * but it needs to be scanned to determine its true state.
	 * This state is only valid during yaffs_Scan.
	 * NB We tolerate empty because the pre-scanner might be incapable of deciding
	 * However, if this state is returned on a YAFFS2 device, then we expect a sequence number
	 */
	
	YAFFS_BLOCK_STATE_EMPTY,
	/* This block is empty */

	YAFFS_BLOCK_STATE_ALLOCATING,
	/* This block is partially allocated. 
	 * At least one page holds valid data.
	 * This is the one currently being used for page
	 * allocation. Should never be more than one of these
	 */

	YAFFS_BLOCK_STATE_FULL,	
	/* All the pages in this block have been allocated.
	 */

	YAFFS_BLOCK_STATE_DIRTY,
	/* All pages have been allocated and deleted. 
	 * Erase me, reuse me.
	 */

	YAFFS_BLOCK_STATE_CHECKPOINT,	
	/* This block is assigned to holding checkpoint data.
	 */

	YAFFS_BLOCK_STATE_COLLECTING,	
	/* This block is being garbage collected */

	YAFFS_BLOCK_STATE_DEAD	
	/* This block has failed and is not in use */
} yaffs_BlockState;
	

typedef struct {
	int softDeletions:10;	/* number of soft deleted pages */
	int pagesInUse:10;	/* number of pages in use */
	unsigned blockState:4;	/* One of the above block states. NB use unsigned because enum is sometimes an int */
	__u32 needsRetiring:1;	/* Data has failed on this block, need to get valid data off */
	                      	/* and retire the block. */
	__u32 skipErasedCheck: 1; /* If this is set we can skip the erased check on this block */
	__u32 gcPrioritise: 1; 	/* An ECC check or blank check has failed on this block.  It should be prioritised for GC */
	__u32 chunkErrorStrikes:3; /* How many times we've had ecc etc failures on this block and tried to reuse it */
															   
	__u32 hasShrinkHeader:1; /* This block has at least one shrink object header */
	__u32 sequenceNumber;	 /* block sequence number for yaffs2 */					
}yaffs_BlockInfo;


typedef struct {
	int seq;
	int block;
} yaffs_BlockIndex;


/*--------------------------- Tnode -------------------------- */
union yaffs_Tnode_union {
	union yaffs_Tnode_union *internal[YAFFS_NTNODES_INTERNAL];
	/*	__u16 level0[YAFFS_NTNODES_LEVEL0]; */
};

typedef union yaffs_Tnode_union yaffs_Tnode;
struct yaffs_TnodeList_struct {
	struct yaffs_TnodeList_struct *next;
	yaffs_Tnode *tnodes;
};

typedef struct yaffs_TnodeList_struct yaffs_TnodeList;

/*------------------------  Object -----------------------------*/
/* An object can be one of:
 * - a directory (no data, has children links
 * - a regular file (data.... not prunes :->).
 * - a symlink [symbolic link] (the alias).
 * - a hard link
 */
typedef struct {
	__u32 fileSize;
	__u32 scannedFileSize;
	__u32 shrinkSize;
	int topLevel;
	yaffs_Tnode *top;
}yaffs_FileStructure;

typedef struct {
	struct ylist_head children;     /* list of child links */
} yaffs_DirectoryStructure;
		
typedef struct {
	char *alias;
} yaffs_SymLinkStructure;
		
typedef struct {
	struct yaffs_ObjectStruct *equivalentObject;
	__u32 equivalentObjectId;
}yaffs_HardLinkStructure;

typedef union {
	yaffs_FileStructure fileVariant;
	yaffs_DirectoryStructure directoryVariant;
	yaffs_SymLinkStructure symLinkVariant;
	yaffs_HardLinkStructure hardLinkVariant;
}yaffs_ObjectVariant;

struct yaffs_ObjectStruct {
	__u8 deleted:1;		/* This should only apply to unlinked files. */
	__u8 softDeleted:1;	/* it has also been soft deleted */
	__u8 unlinked:1;	/* An unlinked file. The file should be in the unlinked directory.*/
	__u8 fake:1;		/* A fake object has no presence on NAND. */
	__u8 renameAllowed:1;	/* Some objects are not allowed to be renamed. */
	__u8 unlinkAllowed:1;
	__u8 dirty:1;		/* the object needs to be written to flash */
	__u8 valid:1;		/* When the file system is being loaded up, this 
				 * object might be created before the data
				 * 				 * is available (ie. file data records appear before the header).
				 */
	__u8 lazyLoaded:1;	/* This object has been lazy loaded and is missing some detail */
	__u8 deferedFree:1;	/* For Linux kernel. Object is removed from NAND, but is
				 * still in the inode cache. Free of object is defered.
 				 * until the inode is released.
 				 */

	__u8 serial;		/* serial number of chunk in NAND. Cached here */
	__u16 sum;		/* sum of the name to speed searching */
        struct yaffs_DeviceStruct *myDev;       /* The device I'm on */
        struct ylist_head hashLink;     /* list of objects in this hash bucket */
        struct ylist_head hardLinks;    /* all the equivalent hard linked objects */

	/* directory structure stuff */
	/* also used for linking up the free list */
	struct yaffs_ObjectStruct *parent; 
	struct ylist_head siblings;

	/* Where's my object header in NAND? */
	int hdrChunk;
	int nDataChunks;	/* Number of data chunks attached to the file. */
	__u32 objectId;		/* the object id value */
	__u32 yst_mode;
	__u32 inUse;
	__u32 yst_uid;
	__u32 yst_gid;
	__u32 yst_atime;
	__u32 yst_mtime;
	__u32 yst_ctime;
	
	__u32 yst_rdev;	
	yaffs_ObjectType variantType;

	yaffs_ObjectVariant variant;
};

typedef struct yaffs_ObjectStruct yaffs_Object;

typedef struct
{
	__u8  inUse:1;		// this handle is in use
	__u8  readOnly:1;	// this handle is read only
	__u8  append:1;		// append only
	__u8  exclusive:1;	// exclusive
	__u32 position;		// current position in file
	yaffs_Object *obj;	// the object
}yaffsfs_Handle;

struct yaffs_ShadowFixerStruct {
	int objectId;
	int shadowedId;
	struct yaffs_ShadowFixerStruct *next;
};

typedef struct {
	__u8 *buffer;
	int line;	/* track from whence this buffer was allocated */
	int maxLine;
} yaffs_TempBuffer;

typedef struct {
        struct ylist_head list;
        int count;
} yaffs_ObjectBucket;

struct yaffs_ObjectList_struct {
	yaffs_Object *objects;
	struct yaffs_ObjectList_struct *next;
};
typedef struct yaffs_ObjectList_struct yaffs_ObjectList;

/* Spare structure for YAFFS1 */
typedef struct {
	__u8 tagByte0;
	__u8 tagByte1;
	__u8 tagByte2;
	__u8 tagByte3;
	__u8 pageStatus;	/* set to 0 to delete the chunk */
	__u8 blockStatus;
	__u8 tagByte4;
	__u8 tagByte5;
	__u8 ecc1[3];
	__u8 tagByte6;
	__u8 tagByte7;
	__u8 ecc2[3];
} yaffs_Spare;

/*Special structure for passing through to mtd */
struct yaffs_NANDSpare {
	yaffs_Spare spare;
	int eccres1;
	int eccres2;
};

struct yaffs_DeviceStruct {
        struct ylist_head devList;
        const char *name;

        /* Entry parameters set up way early. Yaffs sets up the rest.*/
        int nDataBytesPerChunk; /* Should be a power of 2 >= 512 */
        int nChunksPerBlock;    /* does not need to be a power of 2 */
        int spareBytesPerChunk;/* spare area size */
        int startBlock;         /* Start block we're allowed to use */
        int endBlock;           /* End block we're allowed to use */
        int nReservedBlocks;    /* We want this tuneable so that we can reduce */
				/* reserved blocks on NOR and RAM. */
	
	 __u8 *spareBuffer;
	/* Stuff used by the shared space checkpointing mechanism */
	/* If this value is zero, then this mechanism is disabled */
	
//	int nCheckpointReservedBlocks; /* Blocks to reserve for checkpoint data */

	


	int nShortOpCaches;	/* If <= 0, then short op caching is disabled, else
				 * the number of short op caches (don't use too many)
				 */

	int useHeaderFileSize;	/* Flag to determine if we should use file sizes from the header */

	int useNANDECC;		/* Flag to decide whether or not to use NANDECC */

	void *genericDevice;	/* Pointer to device context
				 * On an mtd this holds the mtd pointer.
				 */
        void *superBlock;
        
	/* NAND access functions (Must be set before calling YAFFS)*/

	int (*writeChunkToNAND) (struct yaffs_DeviceStruct * dev,
				 int chunkInNAND, const __u8 * data,
				 const yaffs_Spare * spare);
	int (*readChunkFromNAND) (struct yaffs_DeviceStruct * dev,
				  int chunkInNAND, __u8 * data,
				  yaffs_Spare * spare);
	int (*eraseBlockInNAND) (struct yaffs_DeviceStruct * dev,
				 int blockInNAND);
	int (*initialiseNAND) (struct yaffs_DeviceStruct * dev);

	int (*writeChunkWithTagsToNAND) (struct yaffs_DeviceStruct * dev,
					 int chunkInNAND, const __u8 * data,
					 const yaffs_ExtendedTags * tags);
	int (*readChunkWithTagsFromNAND) (struct yaffs_DeviceStruct * dev,
					  int chunkInNAND, __u8 * data,
					  yaffs_ExtendedTags * tags);
	int (*markNANDBlockBad) (struct yaffs_DeviceStruct * dev, int blockNo);
	int (*queryNANDBlock) (struct yaffs_DeviceStruct * dev, int blockNo,
			       yaffs_BlockState * state, __u32 *sequenceNumber);

	int isYaffs2;
	
	/* The removeObjectCallback function must be supplied by OS flavours that 
	 * need it. The Linux kernel does not use this, but yaffs direct does use
	 * it to implement the faster readdir
	 */
	void (*removeObjectCallback)(struct yaffs_ObjectStruct *obj);
	
	/* Callback to mark the superblock dirsty */
	void (*markSuperBlockDirty)(void * superblock);
	
	int wideTnodesDisabled; /* Set to disable wide tnodes */
	
	char *pathDividers;	/* String of legal path dividers */
	

	/* End of stuff that must be set before initialisation. */
	
	/* Checkpoint control. Can be set before or after initialisation */
	__u8 skipCheckpointRead;
	__u8 skipCheckpointWrite;

	/* Runtime parameters. Set up by YAFFS. */

	__u16 chunkGroupBits;	/* 0 for devices <= 32MB. else log2(nchunks) - 16 */
	__u16 chunkGroupSize;	/* == 2^^chunkGroupBits */
	
	/* Stuff to support wide tnodes */
	__u32 tnodeWidth;
	__u32 tnodeMask;
	
	/* Stuff for figuring out file offset to chunk conversions */
	__u32 chunkShift; /* Shift value */
	__u32 chunkDiv;   /* Divisor after shifting: 1 for power-of-2 sizes */
	__u32 chunkMask;  /* Mask to use for power-of-2 case */

	/* Stuff to handle inband tags */
	int inbandTags;
	__u32 totalBytesPerChunk;

	int isMounted;
	
	int isCheckpointed;


	/* Stuff to support block offsetting to support start block zero */
	int internalStartBlock;
	int internalEndBlock;
	int blockOffset;
	int chunkOffset;
	

	/* Runtime checkpointing stuff */
	int checkpointPageSequence;   /* running sequence number of checkpoint pages */
	int checkpointByteCount;
	int checkpointByteOffset;
	__u8 *checkpointBuffer;
	int checkpointOpenForWrite;
	int blocksInCheckpoint;
	int checkpointCurrentChunk;
	int checkpointCurrentBlock;
	int checkpointNextBlock;
	int *checkpointBlockList;
	int checkpointMaxBlocks;
	__u32 checkpointSum;
	__u32 checkpointXor;
	
	int nCheckpointBlocksRequired; /* Number of blocks needed to store current checkpoint set */
	
	/* Block Info */
	yaffs_BlockInfo *blockInfo;
	__u8 *chunkBits;	/* bitmap of chunks in use */
	unsigned blockInfoAlt:1;	/* was allocated using alternative strategy */
	unsigned chunkBitsAlt:1;	/* was allocated using alternative strategy */
	int chunkBitmapStride;	/* Number of bytes of chunkBits per block. 
				 * Must be consistent with nChunksPerBlock.
				 */

	int nErasedBlocks;
	int allocationBlock;	/* Current block being allocated off */
	__u32 allocationPage;
	int allocationBlockFinder;	/* Used to search for next allocation block */

	/* Runtime state */
	int nTnodesCreated;
	yaffs_Tnode *freeTnodes;
	int nFreeTnodes;
	yaffs_TnodeList *allocatedTnodeList;

	int isDoingGC;

	int nObjectsCreated;
	yaffs_Object *freeObjects;
	int nFreeObjects;
	
	int nHardLinks;

	yaffs_ObjectList *allocatedObjectList;

	yaffs_ObjectBucket objectBucket[YAFFS_NOBJECT_BUCKETS];

	int nFreeChunks;

	int currentDirtyChecker;	/* Used to find current dirtiest block */

	__u32 *gcCleanupList;	/* objects to delete at the end of a GC. */
	int nonAggressiveSkip;	/* GC state/mode */

	/* Statistcs */
	int nPageWrites;
	int nPageReads;
	int nBlockErasures;
	int nErasureFailures;
	int nGCCopies;
	int garbageCollections;
	int passiveGarbageCollections;
	int nRetriedWrites;
	int nRetiredBlocks;
	int eccFixed;
	int eccUnfixed;
	int tagsEccFixed;
	int tagsEccUnfixed;
	int nDeletions;
	int nUnmarkedDeletions;
	
	int hasPendingPrioritisedGCs; /* We think this device might have pending prioritised gcs */

	/* Special directories */
	yaffs_Object *rootDir;
	yaffs_Object *lostNFoundDir;

	/* Buffer areas for storing data to recover from write failures TODO
	 *      __u8            bufferedData[YAFFS_CHUNKS_PER_BLOCK][YAFFS_BYTES_PER_CHUNK];
	 *      yaffs_Spare bufferedSpare[YAFFS_CHUNKS_PER_BLOCK];
	 */
	
	int bufferedBlock;	/* Which block is buffered here? */
	int doingBufferedBlockRewrite;

	//yaffs_ChunkCache *srCache;
	int srLastUse;

	int cacheHits;

	/* Stuff for background deletion and unlinked files.*/
	yaffs_Object *unlinkedDir;	/* Directory where unlinked and deleted files live. */
	yaffs_Object *deletedDir;	/* Directory where deleted objects are sent to disappear. */
	yaffs_Object *unlinkedDeletion;	/* Current file being background deleted.*/
	int nDeletedFiles;		/* Count of files awaiting deletion;*/
	int nUnlinkedFiles;		/* Count of unlinked files. */
	int nBackgroundDeletions;	/* Count of background deletions. */

	
	/* Temporary buffer management */
	yaffs_TempBuffer tempBuffer[YAFFS_N_TEMP_BUFFERS];
	int maxTemp;
	int tempInUse;
	int unmanagedTempAllocations;
	int unmanagedTempDeallocations;

	/* yaffs2 runtime stuff */
	unsigned sequenceNumber;	/* Sequence number of currently allocating block */
	unsigned oldestDirtySequence;

};

typedef struct yaffs_DeviceStruct yaffs_Device;

//################################globe definition#####################

static yaffsfs_Handle yaffsfs_handle[YAFFSFS_N_HANDLES];
yaffs_Device gdev;

int yaffs_open(int fd, const char *path, int oflag, int mode);
int yaffs_read(int fd, void *buf, unsigned int nbyte);
int yaffs_write(int fd, const void *buf, unsigned int nbyte);
int yaffs_lseek(int fd, off_t offset, int where);
int yaffs_close(int fd);

//################################# init yaffs ######################
static FileSystem yaffsfs = {
	"yaffs",FS_FILE,
	yaffs_open,
	yaffs_read,
	yaffs_write,
	yaffs_lseek,
	yaffs_close,
	NULL
};

static void init_fs(void) __attribute__ ((constructor));

static void init_fs()
{
	filefs_init(&yaffsfs);
}

//################################ process handle ############
static int yaffsfs_InitHandles(void)
{
	int i;
	for(i = 0; i < YAFFSFS_N_HANDLES; i++)
	{
		yaffsfs_handle[i].inUse = 0;
		yaffsfs_handle[i].obj = NULL;
	}
	return 0;
}

yaffsfs_Handle *yaffsfs_GetHandlePointer(int h)
{
	if(h < 0 || h >= YAFFSFS_N_HANDLES)
	{
		return NULL;
	}

	return &yaffsfs_handle[h];
}

yaffs_Object *yaffsfs_GetHandleObject(int handle)
{
	yaffsfs_Handle *h = yaffsfs_GetHandlePointer(handle);

	if(h && h->inUse)
	{
		return h->obj;
	}

	return NULL;
}

static int yaffsfs_GetHandle(void)
{
	int i;
	yaffsfs_Handle *h;

	for(i = 0; i < YAFFSFS_N_HANDLES; i++)
	{
		h = yaffsfs_GetHandlePointer(i);
		if(!h)
		{
			// todo bug: should never happen
		}
		if(!h->inUse)
		{
			memset(h,0,sizeof(yaffsfs_Handle));
			h->inUse=1;
			return i;
		}
	}
	return -1;
}

// yaffs_PutHandle
// Let go of a handle (when closing a file)
static int yaffsfs_PutHandle(int handle)
{
	yaffsfs_Handle *h = yaffsfs_GetHandlePointer(handle);

	if(h)
	{
		h->inUse = 0;
		h->obj = NULL;
	}
	return 0;
}

//######################################## ECC function ##############################################
static const unsigned char column_parity_table[] = {
	0x00, 0x55, 0x59, 0x0c, 0x65, 0x30, 0x3c, 0x69,
	0x69, 0x3c, 0x30, 0x65, 0x0c, 0x59, 0x55, 0x00,
	0x95, 0xc0, 0xcc, 0x99, 0xf0, 0xa5, 0xa9, 0xfc,
	0xfc, 0xa9, 0xa5, 0xf0, 0x99, 0xcc, 0xc0, 0x95,
	0x99, 0xcc, 0xc0, 0x95, 0xfc, 0xa9, 0xa5, 0xf0,
	0xf0, 0xa5, 0xa9, 0xfc, 0x95, 0xc0, 0xcc, 0x99,
	0x0c, 0x59, 0x55, 0x00, 0x69, 0x3c, 0x30, 0x65,
	0x65, 0x30, 0x3c, 0x69, 0x00, 0x55, 0x59, 0x0c,
	0xa5, 0xf0, 0xfc, 0xa9, 0xc0, 0x95, 0x99, 0xcc,
	0xcc, 0x99, 0x95, 0xc0, 0xa9, 0xfc, 0xf0, 0xa5,
	0x30, 0x65, 0x69, 0x3c, 0x55, 0x00, 0x0c, 0x59,
	0x59, 0x0c, 0x00, 0x55, 0x3c, 0x69, 0x65, 0x30,
	0x3c, 0x69, 0x65, 0x30, 0x59, 0x0c, 0x00, 0x55,
	0x55, 0x00, 0x0c, 0x59, 0x30, 0x65, 0x69, 0x3c,
	0xa9, 0xfc, 0xf0, 0xa5, 0xcc, 0x99, 0x95, 0xc0,
	0xc0, 0x95, 0x99, 0xcc, 0xa5, 0xf0, 0xfc, 0xa9,
	0xa9, 0xfc, 0xf0, 0xa5, 0xcc, 0x99, 0x95, 0xc0,
	0xc0, 0x95, 0x99, 0xcc, 0xa5, 0xf0, 0xfc, 0xa9,
	0x3c, 0x69, 0x65, 0x30, 0x59, 0x0c, 0x00, 0x55,
	0x55, 0x00, 0x0c, 0x59, 0x30, 0x65, 0x69, 0x3c,
	0x30, 0x65, 0x69, 0x3c, 0x55, 0x00, 0x0c, 0x59,
	0x59, 0x0c, 0x00, 0x55, 0x3c, 0x69, 0x65, 0x30,
	0xa5, 0xf0, 0xfc, 0xa9, 0xc0, 0x95, 0x99, 0xcc,
	0xcc, 0x99, 0x95, 0xc0, 0xa9, 0xfc, 0xf0, 0xa5,
	0x0c, 0x59, 0x55, 0x00, 0x69, 0x3c, 0x30, 0x65,
	0x65, 0x30, 0x3c, 0x69, 0x00, 0x55, 0x59, 0x0c,
	0x99, 0xcc, 0xc0, 0x95, 0xfc, 0xa9, 0xa5, 0xf0,
	0xf0, 0xa5, 0xa9, 0xfc, 0x95, 0xc0, 0xcc, 0x99,
	0x95, 0xc0, 0xcc, 0x99, 0xf0, 0xa5, 0xa9, 0xfc,
	0xfc, 0xa9, 0xa5, 0xf0, 0x99, 0xcc, 0xc0, 0x95,
	0x00, 0x55, 0x59, 0x0c, 0x65, 0x30, 0x3c, 0x69,
	0x69, 0x3c, 0x30, 0x65, 0x0c, 0x59, 0x55, 0x00,
};

/* Count the bits in an unsigned char or a U32 */
static int yaffs_CountBits(unsigned char x)
{
	int r = 0;
	while (x)
	{
		if (x & 1)
			r++;
		x >>= 1;
	}
	return r;
}

static int yaffs_CountBits32(unsigned x)
{
	int r = 0;
	while (x)
	{
		if (x & 1)
			r++;
		x >>= 1;
	}
	
	return r;
}

/* Calculate the ECC for a 256-byte block of data */
void yaffs_ECCCalculate(const unsigned char *data, unsigned char *ecc)
{
	unsigned int i;
	unsigned char col_parity = 0;
	unsigned char line_parity = 0;
	unsigned char line_parity_prime = 0;
	unsigned char t;
	unsigned char b;
	
	for (i = 0; i < 256; i++) 
	{
		b = column_parity_table[*data++];
		col_parity ^= b;
		
		if (b & 0x01)	// odd number of bits in the byte
		{
			line_parity ^= i;
			line_parity_prime ^= ~i;
		}
	}
	
	ecc[2] = (~col_parity) | 0x03;
	t = 0;
	
	if (line_parity & 0x80)
		t |= 0x80;
	if (line_parity_prime & 0x80)
		t |= 0x40;
	if (line_parity & 0x40)
		t |= 0x20;
	if (line_parity_prime & 0x40)
		t |= 0x10;
	if (line_parity & 0x20)
		t |= 0x08;
	if (line_parity_prime & 0x20)
		t |= 0x04;
	if (line_parity & 0x10)
		t |= 0x02;
	if (line_parity_prime & 0x10)
		t |= 0x01;
	
	ecc[1] = ~t;
	t = 0;
	
	if (line_parity & 0x08)	
		t |= 0x80;
	if (line_parity_prime & 0x08)
		t |= 0x40;
	if (line_parity & 0x04)
		t |= 0x20;
	if (line_parity_prime & 0x04)
		t |= 0x10;
	if (line_parity & 0x02)
		t |= 0x08;
	if (line_parity_prime & 0x02)
		t |= 0x04;
	if (line_parity & 0x01)
		t |= 0x02;
	if (line_parity_prime & 0x01)
		t |= 0x01;
	
	ecc[0] = ~t;
}

/* Correct the ECC on a 256 byte block of data */

int yaffs_ECCCorrect(unsigned char *data, unsigned char *read_ecc, const unsigned char *test_ecc)
{
	unsigned char d0, d1, d2;	/* deltas */
	
	d0 = read_ecc[0] ^ test_ecc[0];
	d1 = read_ecc[1] ^ test_ecc[1];
	d2 = read_ecc[2] ^ test_ecc[2];

	if ((d0 | d1 | d2) == 0)
		return 0; /* no error */
	if (((d0 ^ (d0 >> 1)) & 0x55) == 0x55 && ((d1 ^ (d1 >> 1)) & 0x55) == 0x55 && ((d2 ^ (d2 >> 1)) & 0x54) == 0x54) 
	{
		/* Single bit (recoverable) error in data */
		unsigned byte;
		unsigned bit;

		bit = byte = 0;

		if (d1 & 0x80)
			byte |= 0x80;
		if (d1 & 0x20)
			byte |= 0x40;
		if (d1 & 0x08)
			byte |= 0x20;
		if (d1 & 0x02)
			byte |= 0x10;
		if (d0 & 0x80)
			byte |= 0x08;
		if (d0 & 0x20)
			byte |= 0x04;
		if (d0 & 0x08)
			byte |= 0x02;
		if (d0 & 0x02)
			byte |= 0x01;
		if (d2 & 0x80)
			bit |= 0x04;
		if (d2 & 0x20)
			bit |= 0x02;
		if (d2 & 0x08)
			bit |= 0x01;
			
		data[byte] ^= (1 << bit);
		return 1; /* Corrected the error */
	}
	
	if ((yaffs_CountBits(d0) + yaffs_CountBits(d1) + yaffs_CountBits(d2)) ==  1)
	{
		/* Reccoverable error in ecc */
		read_ecc[0] = test_ecc[0];
		read_ecc[1] = test_ecc[1];
		read_ecc[2] = test_ecc[2];
		
		return 1; /* Corrected the error */
	}
	
	/* Unrecoverable error */
	return -1;
}

/*
 * ECCxxxOther does ECC calcs on arbitrary n bytes of data
 */
void yaffs_ECCCalculateOther(const unsigned char *data, unsigned nBytes, yaffs_ECCOther * eccOther)
{
	unsigned int i;

	unsigned char col_parity = 0;
	unsigned line_parity = 0;
	unsigned line_parity_prime = 0;
	unsigned char b;

	for (i = 0; i < nBytes; i++) 
	{
		b = column_parity_table[*data++];
		col_parity ^= b;

		if (b & 0x01)	 
		{
			/* odd number of bits in the byte */
			line_parity ^= i;
			line_parity_prime ^= ~i;
		}

	}

	eccOther->colParity = (col_parity >> 2) & 0x3f;
	eccOther->lineParity = line_parity;
	eccOther->lineParityPrime = line_parity_prime;
}

int yaffs_ECCCorrectOther(unsigned char *data, unsigned nBytes, yaffs_ECCOther * read_ecc,
			  const yaffs_ECCOther * test_ecc)
{
	unsigned char cDelta;	/* column parity delta */
	unsigned lDelta;	/* line parity delta */
	unsigned lDeltaPrime;	/* line parity delta */
	unsigned bit;

	cDelta = read_ecc->colParity ^ test_ecc->colParity;
	lDelta = read_ecc->lineParity ^ test_ecc->lineParity;
	lDeltaPrime = read_ecc->lineParityPrime ^ test_ecc->lineParityPrime;

	if ((cDelta | lDelta | lDeltaPrime) == 0)
		return 0; /* no error */

	if (lDelta == ~lDeltaPrime && (((cDelta ^ (cDelta >> 1)) & 0x15) == 0x15))
	{
		/* Single bit (recoverable) error in data */

		bit = 0;

		if (cDelta & 0x20)
			bit |= 0x04;
		if (cDelta & 0x08)
			bit |= 0x02;
		if (cDelta & 0x02)
			bit |= 0x01;

		if(lDelta >= nBytes)
			return -1;

		data[lDelta] ^= (1 << bit);

		return 1; /* corrected */
	}

	if ((yaffs_CountBits32(lDelta) + yaffs_CountBits32(lDeltaPrime) + yaffs_CountBits(cDelta)) == 1) 
	{
		/* Reccoverable error in ecc */
		*read_ecc = *test_ecc;
		return 1; /* corrected */
	}

	return -1;

}



//######################################## pack tags #################################################
#define EXTRA_HEADER_INFO_FLAG	0x80000000
#define EXTRA_SHRINK_FLAG	0x40000000
#define EXTRA_SHADOWS_FLAG	0x20000000
#define EXTRA_SPARE_FLAGS	0x10000000

#define ALL_EXTRA_FLAGS		0xF0000000

/* Also, the top 4 bits of the object Id are set to the object type. */
#define EXTRA_OBJECT_TYPE_SHIFT (28)
#define EXTRA_OBJECT_TYPE_MASK  ((0x0F) << EXTRA_OBJECT_TYPE_SHIFT)

void yaffs_InitialiseTags(yaffs_ExtendedTags *tags)
{
	memset(tags, 0 ,sizeof(yaffs_ExtendedTags));
	tags->validMarker0 = 0xAAAAAAAA;
	tags->validMarker1 = 0x55555555;
}


void yaffs_PackTags2TagsPart(yaffs_PackedTags2TagsPart * ptt, const yaffs_ExtendedTags * t)
{
	ptt->chunkId = t->chunkId;
	ptt->sequenceNumber = t->sequenceNumber;
	ptt->byteCount = t->byteCount;
	ptt->objectId = t->objectId;
	
	if (t->chunkId == 0 && t->extraHeaderInfoAvailable) 
	{
		/* Store the extra header info instead */
		/* We save the parent object in the chunkId */
		ptt->chunkId = EXTRA_HEADER_INFO_FLAG | t->extraParentObjectId;
		
		if (t->extraIsShrinkHeader) 
			ptt->chunkId |= EXTRA_SHRINK_FLAG;
		
		if (t->extraShadows)
			ptt->chunkId |= EXTRA_SHADOWS_FLAG;
		
		ptt->objectId &= ~EXTRA_OBJECT_TYPE_MASK;
		ptt->objectId |= (t->extraObjectType << EXTRA_OBJECT_TYPE_SHIFT);
		
		if (t->extraObjectType == YAFFS_OBJECT_TYPE_HARDLINK) 
			ptt->byteCount = t->extraEquivalentObjectId;
		else if (t->extraObjectType == YAFFS_OBJECT_TYPE_FILE)
		 	ptt->byteCount = t->extraFileLength;
		else
			ptt->byteCount = 0;
	}
}

void yaffs_PackTags2(yaffs_PackedTags2 * pt, const yaffs_ExtendedTags * t)
{
	yaffs_PackTags2TagsPart(&pt->t,t);
}

void yaffs_UnpackTags2TagsPart(yaffs_ExtendedTags * t, yaffs_PackedTags2TagsPart * ptt)
{
	memset(t, 0, sizeof(yaffs_ExtendedTags));
	yaffs_InitialiseTags(t);
	if (ptt->sequenceNumber != 0xFFFFFFFF) 
	{
		t->blockBad = 0;
		t->chunkUsed = 1;
		t->objectId = ptt->objectId;
		t->chunkId = ptt->chunkId;
		t->byteCount = ptt->byteCount;
		t->chunkDeleted = 0;
		t->serialNumber = 0;
		t->sequenceNumber = ptt->sequenceNumber;
		
		/* Do extra header info stuff */
		if (ptt->chunkId & EXTRA_HEADER_INFO_FLAG)
		{
			t->chunkId = 0;
			t->byteCount = 0;
			t->extraHeaderInfoAvailable = 1;
			t->extraParentObjectId = ptt->chunkId & (~(ALL_EXTRA_FLAGS));
			t->extraIsShrinkHeader = (ptt->chunkId & EXTRA_SHRINK_FLAG) ? 1 : 0;
			t->extraShadows = (ptt->chunkId & EXTRA_SHADOWS_FLAG) ? 1 : 0;
			t->extraObjectType = ptt->objectId >> EXTRA_OBJECT_TYPE_SHIFT;
			t->objectId &= ~EXTRA_OBJECT_TYPE_MASK;
			
			if (t->extraObjectType == YAFFS_OBJECT_TYPE_HARDLINK) 
				t->extraEquivalentObjectId = ptt->byteCount;
			else
				t->extraFileLength = ptt->byteCount;
				
		}
	}
	
}

void yaffs_UnpackTags2(yaffs_ExtendedTags * t, yaffs_PackedTags2 * pt)
{
	yaffs_UnpackTags2TagsPart(t,&pt->t);

	if (pt->t.sequenceNumber != 0xFFFFFFFF) 
	{
		/* Page is in use */
		yaffs_ECCOther ecc;
		int result;

		yaffs_ECCCalculateOther((unsigned char *)&pt->t, sizeof(yaffs_PackedTags2TagsPart), &ecc);
		result = yaffs_ECCCorrectOther((unsigned char *)&pt->t, sizeof(yaffs_PackedTags2TagsPart), &pt->ecc, &ecc);
		
		switch(result){
			case 0:
				t->eccResult = YAFFS_ECC_RESULT_NO_ERROR;
				break;
			case 1:
				t->eccResult = YAFFS_ECC_RESULT_FIXED;
				break;
			case -1:
				t->eccResult = YAFFS_ECC_RESULT_UNFIXED;
				break;
			default:
				t->eccResult = YAFFS_ECC_RESULT_UNKNOWN;
		}
	}
}


//######################################## guts function #############################################
/* Function to calculate chunk and offset */
static void yaffs_AddrToChunk(yaffs_Device *dev, loff_t addr, int *chunkOut, __u32 *offsetOut)
{
	int chunk;
	__u32 offset;
	
	chunk  = (__u32)(addr >> dev->chunkShift);
		
	if(dev->chunkDiv == 1)
	{
		/* easy power of 2 case */
		offset = (__u32)(addr & dev->chunkMask);
	}
	else
	{
		/* Non power-of-2 case */
		
		loff_t chunkBase;
		
		chunk /= dev->chunkDiv;
		chunkBase = ((loff_t)chunk) * dev->nDataBytesPerChunk;
		offset = (__u32)(addr - chunkBase);
	}

	*chunkOut = chunk;
	*offsetOut = offset;
}

/* Function to return the number of shifts for a power of 2 greater than or equal 
 * to the given number
 * Note we don't try to cater for all possible numbers and this does not have to
 * be hellishly efficient.
 */
 
static __u32 ShiftsGE(__u32 x)
{
	int extraBits;
	int nShifts;
	
	nShifts = extraBits = 0;
	
	while(x>1)
	{
		if(x & 1) extraBits++;
		x>>=1;
		nShifts++;
	}

	if(extraBits) 
		nShifts++;
		
	return nShifts;
}

/* Function to return the number of shifts to get a 1 in bit 0
 */ 
static __u32 Shifts(__u32 x)
{
	int nShifts;
	
	nShifts =  0;
	
	if(!x) return 0;
	
	while( !(x&1))
	{
		x>>=1;
		nShifts++;
	}
		
	return nShifts;
}


/* Temporary buffer manipulations.
 */
static int yaffs_InitialiseTempBuffers(yaffs_Device *dev)	
{
	int i;
	__u8 *buf = (__u8 *)1;
		
	memset(dev->tempBuffer,0,sizeof(dev->tempBuffer));
		
	for (i = 0; buf && i < YAFFS_N_TEMP_BUFFERS; i++) 
	{
		dev->tempBuffer[i].line = 0;	/* not in use */
		dev->tempBuffer[i].buffer = buf = malloc(dev->totalBytesPerChunk);
	}
		
	return buf ? YAFFS_OK : YAFFS_FAIL;
	
}

__u8 *yaffs_GetTempBuffer(yaffs_Device * dev, int lineNo)
{
	int i, j;

	dev->tempInUse++;
	if(dev->tempInUse > dev->maxTemp)
		dev->maxTemp = dev->tempInUse;

	for (i = 0; i < YAFFS_N_TEMP_BUFFERS; i++) 
	{
		if (dev->tempBuffer[i].line == 0) 
		{
			dev->tempBuffer[i].line = lineNo;
			if ((i + 1) > dev->maxTemp) 
			{
				dev->maxTemp = i + 1;
				for (j = 0; j <= i; j++)
					dev->tempBuffer[j].maxLine = dev->tempBuffer[j].line;
			}

			return dev->tempBuffer[i].buffer;
		}
	}

	/*
	 * If we got here then we have to allocate an unmanaged one
	 * This is not good.
	 */
	dev->unmanagedTempAllocations++;
	return malloc(dev->nDataBytesPerChunk);

}

void yaffs_ReleaseTempBuffer(yaffs_Device * dev, __u8 * buffer,
				    int lineNo)
{
	int i;
	
	dev->tempInUse--;
	
	for (i = 0; i < YAFFS_N_TEMP_BUFFERS; i++) 
	{
		if (dev->tempBuffer[i].buffer == buffer) 
		{
			dev->tempBuffer[i].line = 0;
			return;
		}
	}

	if (buffer)
		/* assume it is an unmanaged one. */
		dev->unmanagedTempDeallocations++;

}


/*
 * Chunk bitmap manipulations
 */

static inline __u8 *yaffs_BlockBits(yaffs_Device * dev, int blk)
{
	if (blk < dev->internalStartBlock || blk > dev->internalEndBlock)
		  printf("**>> yaffs: BlockBits block %d is not valid\n", blk);
	
	return dev->chunkBits + (dev->chunkBitmapStride * (blk - dev->internalStartBlock));
}

static inline void yaffs_VerifyChunkBitId(yaffs_Device *dev, int blk, int chunk)
{
	if(blk < dev->internalStartBlock || blk > dev->internalEndBlock ||
	   chunk < 0 || chunk >= dev->nChunksPerBlock) 
	  	printf("**>> yaffs: Chunk Id (%d:%d) invalid\n",blk,chunk);
}

static inline void yaffs_ClearChunkBits(yaffs_Device * dev, int blk)
{
	__u8 *blkBits = yaffs_BlockBits(dev, blk);

	memset(blkBits, 0, dev->chunkBitmapStride);
}

static inline void yaffs_ClearChunkBit(yaffs_Device * dev, int blk, int chunk)
{
	__u8 *blkBits = yaffs_BlockBits(dev, blk);

	yaffs_VerifyChunkBitId(dev,blk,chunk);

	blkBits[chunk / 8] &= ~(1 << (chunk & 7));
}

static inline void yaffs_SetChunkBit(yaffs_Device * dev, int blk, int chunk)
{
	__u8 *blkBits = yaffs_BlockBits(dev, blk);
	
	yaffs_VerifyChunkBitId(dev,blk,chunk);

	blkBits[chunk / 8] |= (1 << (chunk & 7));
}

static inline int yaffs_CheckChunkBit(yaffs_Device * dev, int blk, int chunk)
{
	__u8 *blkBits = yaffs_BlockBits(dev, blk);
	yaffs_VerifyChunkBitId(dev,blk,chunk);

	return (blkBits[chunk / 8] & (1 << (chunk & 7))) ? 1 : 0;
}


static int yaffs_CountChunkBits(yaffs_Device * dev, int blk)
{
	__u8 *blkBits = yaffs_BlockBits(dev, blk);
	int i;
	int n = 0;
	for (i = 0; i < dev->chunkBitmapStride; i++) 
	{
		__u8 x = *blkBits;
		while(x)
		{
			if(x & 1)
				n++;
			x >>=1;
		}
			
		blkBits++;
	}
	return n;
}


static const char * blockStateName[] = {
"Unknown",
"Needs scanning",
"Scanning",
"Empty",
"Allocating",
"Full",
"Dirty",
"Checkpoint",
"Collecting",
"Dead"
};

/*
 *  Simple hash function. Needs to have a reasonable spread
 */
static inline int yaffs_HashFunction(int n)
{
	n = abs(n);
	return (n % YAFFS_NOBJECT_BUCKETS);
}

/*
 * Access functions to useful fake objects.
 * Note that root might have a presence in NAND if permissions are set.
 */
 
yaffs_Object *yaffs_Root(yaffs_Device * dev)
{
	return dev->rootDir;
}

yaffs_Object *yaffs_LostNFound(yaffs_Device * dev)
{
	return dev->lostNFoundDir;
}


/*
 *  Erased NAND checking functions
 */
 
int yaffs_CheckFF(__u8 * buffer, int nBytes)
{
	/* Horrible, slow implementation */
	while (nBytes--)
	{
		if (*buffer != 0xFF)
			return 0;
		buffer++;
	}
	return 1;
}


/*---------------- Name handling functions ------------*/ 

static __u16 yaffs_CalcNameSum(const char * name)
{
	__u16 sum = 0;
	__u16 i = 1;

	char *bname = (char *) name;
	if (bname) {
		while ((*bname) && (i < (YAFFS_MAX_NAME_LENGTH/2))) {

#ifdef CONFIG_YAFFS_CASE_INSENSITIVE
			sum += toupper(*bname) * i;
#else
			sum += (*bname) * i;
#endif
			i++;
			bname++;
		}
	}
	return sum;
}

static void yaffs_SetObjectName(yaffs_Object * obj, const char * name)
{
	obj->sum = yaffs_CalcNameSum(name);
}

/*-------------------- TNODES -------------------

 * List of spare tnodes
 * The list is hooked together using the first pointer
 * in the tnode.
 */
 
/* yaffs_CreateTnodes creates a bunch more tnodes and
 * adds them to the tnode free list.
 * Don't use this function directly
 */

static int yaffs_CreateTnodes(yaffs_Device * dev, int nTnodes)
{
	int i;
	int tnodeSize;
	yaffs_Tnode *newTnodes;
	__u8 *mem;
	yaffs_Tnode *curr;
	yaffs_Tnode *next;
	yaffs_TnodeList *tnl;

	if (nTnodes < 1)
		return YAFFS_OK;
		
	/* Calculate the tnode size in bytes for variable width tnode support.
	 * Must be a multiple of 32-bits  */
	tnodeSize = (dev->tnodeWidth * YAFFS_NTNODES_LEVEL0)/8;

	if(tnodeSize < sizeof(yaffs_Tnode))
		tnodeSize = sizeof(yaffs_Tnode);
		

	/* make these things */

	newTnodes = malloc(nTnodes * tnodeSize);
	mem = (__u8 *)newTnodes;

	if (!newTnodes)
	{
		printf("malloc Tnodes failed!\n");
		return YAFFS_FAIL;
	}
	

	/* Hook them into the free list */
	/* New hookup for wide tnodes */
	for(i = 0; i < nTnodes -1; i++) 
	{
		curr = (yaffs_Tnode *) &mem[i * tnodeSize];
		next = (yaffs_Tnode *) &mem[(i+1) * tnodeSize];
		curr->internal[0] = next;
	}
	
	curr = (yaffs_Tnode *) &mem[(nTnodes - 1) * tnodeSize];
	curr->internal[0] = dev->freeTnodes;
	dev->freeTnodes = (yaffs_Tnode *)mem;

	dev->nFreeTnodes += nTnodes;
	dev->nTnodesCreated += nTnodes;

	/* Now add this bunch of tnodes to a list for freeing up.
	 * NB If we can't add this to the management list it isn't fatal
	 * but it just means we can't free this bunch of tnodes later.
	 */
	 
	tnl = malloc(sizeof(yaffs_TnodeList));
	if (!tnl) 
	{
		   printf("yaffs: Could not add tnodes to management list!\n" );
		   return YAFFS_FAIL;
	}
	else
	{
		tnl->tnodes = newTnodes;
		tnl->next = dev->allocatedTnodeList;
		dev->allocatedTnodeList = tnl;
	}

	return YAFFS_OK;
}

/* GetTnode gets us a clean tnode. Tries to make allocate more if we run out */

static yaffs_Tnode *yaffs_GetTnodeRaw(yaffs_Device * dev)
{
	yaffs_Tnode *tn = NULL;

	/* If there are none left make more */
	if (!dev->freeTnodes)
		yaffs_CreateTnodes(dev, YAFFS_ALLOCATION_NTNODES);

	if (dev->freeTnodes) 
	{
		tn = dev->freeTnodes;
		dev->freeTnodes = dev->freeTnodes->internal[0];
		dev->nFreeTnodes--;
	}

	dev->nCheckpointBlocksRequired = 0; /* force recalculation*/

	return tn;
}

static yaffs_Tnode *yaffs_GetTnode(yaffs_Device * dev)
{
	yaffs_Tnode *tn = yaffs_GetTnodeRaw(dev);
	int tnodeSize = (dev->tnodeWidth * YAFFS_NTNODES_LEVEL0)/8;

	if(tnodeSize < sizeof(yaffs_Tnode))
		tnodeSize = sizeof(yaffs_Tnode);
	
	if(tn)
		memset(tn, 0, tnodeSize);

	return tn;	
}

/* FreeTnode frees up a tnode and puts it back on the free list */
static void yaffs_FreeTnode(yaffs_Device * dev, yaffs_Tnode * tn)
{
	if (tn)
	{
		tn->internal[0] = dev->freeTnodes;
		dev->freeTnodes = tn;
		dev->nFreeTnodes++;
	}
	dev->nCheckpointBlocksRequired = 0; /* force recalculation*/
	
}

static void yaffs_DeinitialiseTnodes(yaffs_Device * dev)
{
	/* Free the list of allocated tnodes */
	yaffs_TnodeList *tmp;

	while (dev->allocatedTnodeList) 
	{
		tmp = dev->allocatedTnodeList->next;

		free(dev->allocatedTnodeList->tnodes);
		free(dev->allocatedTnodeList);
		dev->allocatedTnodeList = tmp;

	}

	dev->freeTnodes = NULL;
	dev->nFreeTnodes = 0;
}

static void yaffs_InitialiseTnodes(yaffs_Device * dev)
{
	dev->allocatedTnodeList = NULL;
	dev->freeTnodes = NULL;
	dev->nFreeTnodes = 0;
	dev->nTnodesCreated = 0;

}


void yaffs_PutLevel0Tnode(yaffs_Device *dev, yaffs_Tnode *tn, unsigned pos, unsigned val)
{
	__u32 *map = (__u32 *)tn;
  	__u32 bitInMap;
  	__u32 bitInWord;
	__u32 wordInMap;
	__u32 mask;
  
  	pos &= YAFFS_TNODES_LEVEL0_MASK;
  	val >>= dev->chunkGroupBits;
  
  	bitInMap = pos * dev->tnodeWidth;
	wordInMap = bitInMap /32;
	bitInWord = bitInMap & (32 -1);
	mask = dev->tnodeMask << bitInWord;
	map[wordInMap] &= ~mask;
	map[wordInMap] |= (mask & (val << bitInWord));
  	if(dev->tnodeWidth > (32-bitInWord)) 
	{
    		bitInWord = (32 - bitInWord);
    		wordInMap++;;
 		mask = dev->tnodeMask >> (/*dev->tnodeWidth -*/ bitInWord);
		map[wordInMap] &= ~mask;
    		map[wordInMap] |= (mask & (val >> bitInWord));
  	}	
}

static __u32 yaffs_GetChunkGroupBase(yaffs_Device *dev, yaffs_Tnode *tn, unsigned pos)
{
	__u32 *map = (__u32 *)tn;
	__u32 bitInMap;
	__u32 bitInWord;
	__u32 wordInMap;
	__u32 val;
  
  	pos &= YAFFS_TNODES_LEVEL0_MASK;
	bitInMap = pos * dev->tnodeWidth;
	wordInMap = bitInMap /32;
	bitInWord = bitInMap & (32 -1);
  
  	val = map[wordInMap] >> bitInWord;
  
	if(dev->tnodeWidth > (32-bitInWord)) 
	{
    		bitInWord = (32 - bitInWord);
		wordInMap++;;
    		val |= (map[wordInMap] << bitInWord);
  	}
	
	val &= dev->tnodeMask;
	val <<= dev->chunkGroupBits;
  
  	return val;
}

/* ------------------- End of individual tnode manipulation -----------------*/

/* ---------Functions to manipulate the look-up tree (made up of tnodes) ------
 * The look up tree is represented by the top tnode and the number of topLevel
 * in the tree. 0 means only the level 0 tnode is in the tree.
 */

/* FindLevel0Tnode finds the level 0 tnode, if one exists. */
static yaffs_Tnode *yaffs_FindLevel0Tnode(yaffs_Device * dev,
					  yaffs_FileStructure * fStruct,
					  __u32 chunkId)
{

	yaffs_Tnode *tn = fStruct->top;
	__u32 i;
	int requiredTallness;
	int level = fStruct->topLevel;

	/* Check sane level and chunk Id */
	if (level < 0 || level > YAFFS_TNODES_MAX_LEVEL)
		return NULL;

	if (chunkId > YAFFS_MAX_CHUNK_ID)
		return NULL;

	/* First check we're tall enough (ie enough topLevel) */

	i = chunkId >> YAFFS_TNODES_LEVEL0_BITS;
	requiredTallness = 0;
	while (i) 
	{
		i >>= YAFFS_TNODES_INTERNAL_BITS;
		requiredTallness++;
	}

	if (requiredTallness > fStruct->topLevel) 
		/* Not tall enough, so we can't find it, return NULL. */
		return NULL;

	/* Traverse down to level 0 */
	while (level > 0 && tn)
	{
		tn = tn->internal[(chunkId >> ( YAFFS_TNODES_LEVEL0_BITS + (level - 1) * YAFFS_TNODES_INTERNAL_BITS)) &
			     YAFFS_TNODES_INTERNAL_MASK];
		level--;
	}

	return tn;
}

/* AddOrFindLevel0Tnode finds the level 0 tnode if it exists, otherwise first expands the tree.
 * This happens in two steps:
 *  1. If the tree isn't tall enough, then make it taller.
 *  2. Scan down the tree towards the level 0 tnode adding tnodes if required.
 *
 * Used when modifying the tree.
 *
 *  If the tn argument is NULL, then a fresh tnode will be added otherwise the specified tn will
 *  be plugged into the ttree.
 */
 
static yaffs_Tnode *yaffs_AddOrFindLevel0Tnode(yaffs_Device * dev,
					       yaffs_FileStructure * fStruct,
					       __u32 chunkId,
					       yaffs_Tnode *passedTn)
{

	int requiredTallness;
	int i;
	int l;
	yaffs_Tnode *tn;

	__u32 x;


	/* Check sane level and page Id */
	if (fStruct->topLevel < 0 || fStruct->topLevel > YAFFS_TNODES_MAX_LEVEL)
		return NULL;
	
	if (chunkId > YAFFS_MAX_CHUNK_ID)
		return NULL;

	/* First check we're tall enough (ie enough topLevel) */

	x = chunkId >> YAFFS_TNODES_LEVEL0_BITS;
	requiredTallness = 0;
	while (x) 
	{
		x >>= YAFFS_TNODES_INTERNAL_BITS;
		requiredTallness++;
	}


	if (requiredTallness > fStruct->topLevel) 
	{
		/* Not tall enough,gotta make the tree taller */
		for (i = fStruct->topLevel; i < requiredTallness; i++) 
		{
		
			tn = yaffs_GetTnode(dev);

			if (tn) 
			{
				tn->internal[0] = fStruct->top;
				fStruct->top = tn;
			}
			else
			{
				printf("yaffs: no more tnodes\n");
			}
		}

		fStruct->topLevel = requiredTallness;
	}

	/* Traverse down to level 0, adding anything we need */

	l = fStruct->topLevel;
	tn = fStruct->top;
	
	if(l > 0) 
	{
		while (l > 0 && tn) 
		{
			x = (chunkId >> ( YAFFS_TNODES_LEVEL0_BITS + (l - 1) * YAFFS_TNODES_INTERNAL_BITS)) &
			    YAFFS_TNODES_INTERNAL_MASK;

			if((l>1) && !tn->internal[x])
				/* Add missing non-level-zero tnode */
				tn->internal[x] = yaffs_GetTnode(dev);
			else if(l == 1)
			{
				/* Looking from level 1 at level 0 */
			 	if (passedTn)
				{
					/* If we already have one, then release it.*/
					if(tn->internal[x])
						yaffs_FreeTnode(dev,tn->internal[x]);
				
					tn->internal[x] = passedTn;
			
				} 
				else if(!tn->internal[x]) 
				{
					/* Don't have one, none passed in */
					tn->internal[x] = yaffs_GetTnode(dev);
				}
			}
		
			tn = tn->internal[x];
			l--;
		}
	}
	else
	{
		/* top is level 0 */
		if(passedTn)
		{
			memcpy(tn,passedTn,(dev->tnodeWidth * YAFFS_NTNODES_LEVEL0)/8);
			yaffs_FreeTnode(dev,passedTn);
		}
	}

	return tn;
}

static int yaffs_FindChunkInGroup(yaffs_Device * dev, int theChunk,
				  yaffs_ExtendedTags * tags, int objectId,
				  int chunkInInode)
{
	int j;

	for (j = 0; theChunk && j < dev->chunkGroupSize; j++) 
	{
		if (yaffs_CheckChunkBit(dev, theChunk/dev->nChunksPerBlock, theChunk%dev->nChunksPerBlock)) 
		{
			yaffs_ReadChunkWithTagsFromNAND(dev, theChunk, NULL, tags);
			if (yaffs_TagsMatch(tags, objectId, chunkInInode)) 
				/* found it; */
				return theChunk;
		}
		theChunk++;
	}
	return -1;
}



/*-------------------- End of File Structure functions.-------------------*/

/* yaffs_CreateFreeObjects creates a bunch more objects and
 * adds them to the object free list.
 */
static int yaffs_CreateFreeObjects(yaffs_Device * dev, int nObjects)
{
	int i;
	yaffs_Object *newObjects;
	yaffs_ObjectList *list;

	if (nObjects < 1)
		return YAFFS_OK;

	/* make these things */
	newObjects = malloc(nObjects * sizeof(yaffs_Object));
	list = malloc(sizeof(yaffs_ObjectList));

	if (!newObjects || !list) 
	{
		if(newObjects)
			free(newObjects);
		if(list)
			free(list);
		printf("yaffs: Could not allocate more objects\n" );
		return YAFFS_FAIL;
	}
	
        /* Hook them into the free list */
        for (i = 0; i < nObjects - 1; i++) 
                newObjects[i].siblings.next = (struct ylist_head *)(&newObjects[i + 1]);

        newObjects[nObjects - 1].siblings.next = (void *)dev->freeObjects;
	dev->freeObjects = newObjects;
	dev->nFreeObjects += nObjects;
	dev->nObjectsCreated += nObjects;

	/* Now add this bunch of Objects to a list for freeing up. */

	list->objects = newObjects;
	list->next = dev->allocatedObjectList;
	dev->allocatedObjectList = list;

	return YAFFS_OK;
}


static void yaffs_AddObjectToDirectory(yaffs_Object * directory, yaffs_Object * obj);
/* AllocateEmptyObject gets us a clean Object. Tries to make allocate more if we run out */
static yaffs_Object *yaffs_AllocateEmptyObject(yaffs_Device * dev)
{
	yaffs_Object *tn = NULL;

	/* If there are none left make more */
	if (!dev->freeObjects)
		yaffs_CreateFreeObjects(dev, YAFFS_ALLOCATION_NOBJECTS);

	if (dev->freeObjects) 
	{
		tn = dev->freeObjects;
		dev->freeObjects = (yaffs_Object *) (dev->freeObjects->siblings.next);
		dev->nFreeObjects--;

		/* Now sweeten it up... */

		memset(tn, 0, sizeof(yaffs_Object));
		tn->myDev = dev;
		tn->hdrChunk = 0;
		tn->variantType = YAFFS_OBJECT_TYPE_UNKNOWN;
		YINIT_LIST_HEAD(&(tn->hardLinks));
		YINIT_LIST_HEAD(&(tn->hashLink));
		YINIT_LIST_HEAD(&tn->siblings);

                /* Add it to the lost and found directory.
                 * NB Can't put root or lostNFound in lostNFound so
		 * check if lostNFound exists first
		 */
		if (dev->lostNFoundDir)
			yaffs_AddObjectToDirectory(dev->lostNFoundDir, tn);
	}
	
	dev->nCheckpointBlocksRequired = 0; /* force recalculation*/

	return tn;
}

yaffs_Object *yaffs_CreateNewObject(yaffs_Device * dev, int number,
				    yaffs_ObjectType type);
static yaffs_Object *yaffs_CreateFakeDirectory(yaffs_Device * dev, int number, __u32 mode)
{

	yaffs_Object *obj = yaffs_CreateNewObject(dev, number, YAFFS_OBJECT_TYPE_DIRECTORY);
	if (obj) 
	{
		obj->fake = 1;		/* it is fake so it might have no NAND presence... */
		obj->renameAllowed = 0;	/* ... and we're not allowed to rename it... */
		obj->unlinkAllowed = 0;	/* ... or unlink it */
		obj->deleted = 0;
		obj->unlinked = 0;
		obj->yst_mode = mode;
		obj->myDev = dev;
		obj->hdrChunk = 0;	/* Not a valid chunk. */
	}

	return obj;

}


static void yaffs_DeinitialiseObjects(yaffs_Device * dev)
{
	/* Free the list of allocated Objects */

	yaffs_ObjectList *tmp;

	while (dev->allocatedObjectList) 
	{
		tmp = dev->allocatedObjectList->next;
		free(dev->allocatedObjectList->objects);
		free(dev->allocatedObjectList);

		dev->allocatedObjectList = tmp;
	}

	dev->freeObjects = NULL;
	dev->nFreeObjects = 0;
}

static void yaffs_InitialiseObjects(yaffs_Device * dev)
{
	int i;

	dev->allocatedObjectList = NULL;
	dev->freeObjects = NULL;
        dev->nFreeObjects = 0;

        for (i = 0; i < YAFFS_NOBJECT_BUCKETS; i++) 
	{
                YINIT_LIST_HEAD(&dev->objectBucket[i].list);
                dev->objectBucket[i].count = 0;
        }

}

static void yaffs_HashObject(yaffs_Object * in)
{
        int bucket = yaffs_HashFunction(in->objectId);
        yaffs_Device *dev = in->myDev;

        ylist_add(&in->hashLink, &dev->objectBucket[bucket].list);
        dev->objectBucket[bucket].count++;

}

yaffs_Object *yaffs_FindObjectByNumber(yaffs_Device * dev, __u32 number)
{
        int bucket = yaffs_HashFunction(number);
        struct ylist_head *i;
        yaffs_Object *in;

        ylist_for_each(i, &dev->objectBucket[bucket].list) 
	{
                /* Look if it is in the list */
                if (i) 
		{
                        in = ylist_entry(i, yaffs_Object, hashLink);
                        if (in->objectId == number)
				return in;
		}
	}

	return NULL;
}

static int yaffs_CreateNewObjectNumber(yaffs_Device * dev)
{
	int bucket = yaffs_FindNiceObjectBucket(dev);

	/* Now find an object value that has not already been taken
 	 * by scanning the list.
         */
	int found = 0;
	struct ylist_head *i;
	
	__u32 n = (__u32) bucket;
	
	/* yaffs_CheckObjectHashSanity();  */
	while (!found) 
	{
		found = 1;
		n += YAFFS_NOBJECT_BUCKETS;
		if (1 || dev->objectBucket[bucket].count > 0) 
		{
			ylist_for_each(i, &dev->objectBucket[bucket].list) 
			{
				/* If there is already one in the list */
				if (i&& ylist_entry(i, yaffs_Object, hashLink)->objectId == n) 
					found = 0;
			}
		}
	}
	
	return n;
}


yaffs_Object *yaffs_CreateNewObject(yaffs_Device * dev, int number,
				    yaffs_ObjectType type)
{

	yaffs_Object *theObject;
	yaffs_Tnode *tn = NULL;

	if (number < 0)
		number = yaffs_CreateNewObjectNumber(dev);

	theObject = yaffs_AllocateEmptyObject(dev);
	if(!theObject)
		return NULL;
		
	if(type == YAFFS_OBJECT_TYPE_FILE)
	{
		tn = yaffs_GetTnode(dev);
		if(!tn)
		{
			printf("can't malloc Tnode!\n");
			return NULL;
		}
	}
		
	

	if (theObject) 
	{
		theObject->fake = 0;
		theObject->renameAllowed = 1;
		theObject->unlinkAllowed = 1;
		theObject->objectId = number;
		yaffs_HashObject(theObject);
		theObject->variantType = type;

		theObject->yst_atime = theObject->yst_mtime = theObject->yst_ctime = 0;
		
		switch (type) 
		{
		case YAFFS_OBJECT_TYPE_FILE:
			theObject->variant.fileVariant.fileSize = 0;
			theObject->variant.fileVariant.scannedFileSize = 0;
			theObject->variant.fileVariant.shrinkSize = 0xFFFFFFFF;	/* max __u32 */
			theObject->variant.fileVariant.topLevel = 0;
                        theObject->variant.fileVariant.top = tn;
                        break;
                case YAFFS_OBJECT_TYPE_DIRECTORY:
                        YINIT_LIST_HEAD(&theObject->variant. directoryVariant. children);
                        break;
                case YAFFS_OBJECT_TYPE_SYMLINK:
		case YAFFS_OBJECT_TYPE_HARDLINK:
		case YAFFS_OBJECT_TYPE_SPECIAL:
			/* No action required */
			break;
		case YAFFS_OBJECT_TYPE_UNKNOWN:
			/* todo this should not happen */
			break;
		}
	}

	return theObject;
}

static yaffs_Object *yaffs_FindOrCreateObjectByNumber(yaffs_Device * dev, int number, yaffs_ObjectType type)
{
	yaffs_Object *theObject = NULL;

	if (number > 0)
		theObject = yaffs_FindObjectByNumber(dev, number);

	if (!theObject)
		theObject = yaffs_CreateNewObject(dev, number, type);

	return theObject;
}
			

static char *yaffs_CloneString(const char * str)
{
	char *newStr = NULL;

	if (str && *str) 
	{
		newStr = malloc((strlen(str) + 1) * sizeof(char));
		if(newStr)
			strcpy(newStr, str);
	}

	return newStr;

}


/*------------------------- Block Management and Page Allocation ----------------*/

static int yaffs_InitialiseBlocks(yaffs_Device * dev)
{
	int nBlocks = dev->internalEndBlock - dev->internalStartBlock + 1;
	
	dev->blockInfo = NULL;
	dev->chunkBits = NULL;
	
	dev->allocationBlock = -1;	/* force it to get a new one */

	/* If the first allocation strategy fails, thry the alternate one */
	dev->blockInfo = malloc(nBlocks * sizeof(yaffs_BlockInfo));
	if(!dev->blockInfo)
	{
		dev->blockInfo = malloc(nBlocks * sizeof(yaffs_BlockInfo));
		dev->blockInfoAlt = 1;
	}
	else
		dev->blockInfoAlt = 0;
		
	if(dev->blockInfo)
	{
	
		/* Set up dynamic blockinfo stuff. */
		dev->chunkBitmapStride = (dev->nChunksPerBlock + 7) / 8; /* round up bytes */
		dev->chunkBits = malloc(dev->chunkBitmapStride * nBlocks);
		if(!dev->chunkBits)
		{
			dev->chunkBits = malloc(dev->chunkBitmapStride * nBlocks);
			dev->chunkBitsAlt = 1;
		}
		else
			dev->chunkBitsAlt = 0;
	}
	
	if (dev->blockInfo && dev->chunkBits) 
	{
		memset(dev->blockInfo, 0, nBlocks * sizeof(yaffs_BlockInfo));
		memset(dev->chunkBits, 0, dev->chunkBitmapStride * nBlocks);
		return YAFFS_OK;
	}

	return YAFFS_FAIL;

}

static void yaffs_DeinitialiseBlocks(yaffs_Device * dev)
{
	if(dev->blockInfoAlt && dev->blockInfo)
		free(dev->blockInfo);
	else if(dev->blockInfo)
		free(dev->blockInfo);

	dev->blockInfoAlt = 0;
	dev->blockInfo = NULL;
	
	if(dev->chunkBitsAlt && dev->chunkBits)
		free(dev->chunkBits);
	else if(dev->chunkBits)
		free(dev->chunkBits);
	
	dev->chunkBitsAlt = 0;
	dev->chunkBits = NULL;
}


/*-------------------------  TAGS --------------------------------*/

static int yaffs_TagsMatch(const yaffs_ExtendedTags * tags, int objectId, int chunkInObject)
{
	return (tags->chunkId == chunkInObject && tags->objectId == objectId && !tags->chunkDeleted) ? 1 : 0;
}

/*------------------------Checkpoint-----------------------------*/
static void yaffs_CheckpointFindNextCheckpointBlock(yaffs_Device *dev)
{
	int  i;
	yaffs_ExtendedTags tags;
	
	if(dev->blocksInCheckpoint < dev->checkpointMaxBlocks)
		for(i = dev->checkpointNextBlock; i <= dev->internalEndBlock; i++)
		{
			int chunk = i * dev->nChunksPerBlock;
			int realignedChunk = chunk - dev->chunkOffset;
			
			dev->readChunkWithTagsFromNAND(dev,realignedChunk,NULL,&tags);
			if(tags.sequenceNumber == YAFFS_SEQUENCE_CHECKPOINT_DATA)
			{
				/* Right kind of block */
				dev->checkpointNextBlock = tags.objectId;
				dev->checkpointCurrentBlock = i;
				dev->checkpointBlockList[dev->blocksInCheckpoint] = i;
				dev->blocksInCheckpoint++;
				return;
			}
		}		
		
	dev->checkpointNextBlock = -1;
	dev->checkpointCurrentBlock = -1;
}

int yaffs_CheckpointOpen(yaffs_Device *dev, int forWriting)
{
	
	int i;
	if(!dev->checkpointBuffer)
		dev->checkpointBuffer = malloc(dev->totalBytesPerChunk);
	
	if(!dev->checkpointBuffer)
		return 0;
	
	dev->checkpointPageSequence = 0;
	dev->checkpointOpenForWrite = forWriting;
	dev->checkpointByteCount = 0;
	dev->checkpointSum = 0;
	dev->checkpointXor = 0;
	dev->checkpointCurrentBlock = -1;
	dev->checkpointCurrentChunk = -1;
	dev->checkpointNextBlock = dev->internalStartBlock;


	/* Set to a value that will kick off a read */
	dev->checkpointByteOffset = dev->nDataBytesPerChunk;
	
	/* A checkpoint block list of 1 checkpoint block per 16 block is (hopefully) going to be way more than we need */
	dev->blocksInCheckpoint = 0;
	dev->checkpointMaxBlocks = (dev->internalEndBlock - dev->internalStartBlock)/16 + 2;
	dev->checkpointBlockList = malloc(sizeof(int) * dev->checkpointMaxBlocks);
	for(i = 0; i < dev->checkpointMaxBlocks; i++)
		dev->checkpointBlockList[i] = -1;
	
	return 1;
}

int yaffs_GetCheckpointSum(yaffs_Device *dev, __u32 *sum)
{
	__u32 compositeSum;
	
	compositeSum =  (dev->checkpointSum << 8) | (dev->checkpointXor & 0xFF);
	*sum = compositeSum;
	return 1;
}

int yaffs_CheckpointRead(yaffs_Device *dev, void *data, int nBytes)
{
	int i=0;
	int ok = 1;
	yaffs_ExtendedTags tags;
	int chunk;
	int realignedChunk;
	__u8 *dataBytes = (__u8 *)data;
	
	if(!dev->checkpointBuffer)
		return 0;
	if(dev->checkpointOpenForWrite)
		return -1;
	while(i < nBytes && ok)
	{
		if(dev->checkpointByteOffset < 0 || dev->checkpointByteOffset >= dev->nDataBytesPerChunk) 
		{
			if(dev->checkpointCurrentBlock < 0)
			{
				yaffs_CheckpointFindNextCheckpointBlock(dev);
				dev->checkpointCurrentChunk = 0;
			}
			if(dev->checkpointCurrentBlock < 0)
				ok = 0;
			else
			{
				chunk = dev->checkpointCurrentBlock * dev->nChunksPerBlock + dev->checkpointCurrentChunk;
				realignedChunk = chunk - dev->chunkOffset;
				
				/* read in the next chunk */
				/* printf("read checkpoint page %d\n",dev->checkpointPage); */
				dev->readChunkWithTagsFromNAND(dev, realignedChunk, dev->checkpointBuffer,&tags);
				if(tags.chunkId != (dev->checkpointPageSequence + 1) 
					|| tags.sequenceNumber != YAFFS_SEQUENCE_CHECKPOINT_DATA)
					ok = 0;
				
				dev->checkpointByteOffset = 0;
				dev->checkpointPageSequence++;
				dev->checkpointCurrentChunk++;
				if(dev->checkpointCurrentChunk >= dev->nChunksPerBlock)
					dev->checkpointCurrentBlock = -1;
			}
		}
		
		if(ok)
		{
			*dataBytes = dev->checkpointBuffer[dev->checkpointByteOffset];
			dev->checkpointSum += *dataBytes;
			dev->checkpointXor ^= *dataBytes;
			dev->checkpointByteOffset++;
			i++;
			dataBytes++;
			dev->checkpointByteCount++;
		}
	}
	
	return 	i;
}

int yaffs_CheckpointClose(yaffs_Device *dev)
{ 
	int i;
		
	for(i = 0; i < dev->blocksInCheckpoint && dev->checkpointBlockList[i] >= 0; i++)
	{
		yaffs_BlockInfo *bi = &dev->blockInfo[dev->checkpointBlockList[i] - dev->internalStartBlock];
		if(bi->blockState == YAFFS_BLOCK_STATE_EMPTY)
			bi->blockState = YAFFS_BLOCK_STATE_CHECKPOINT;
		else 
		{
			// Todo this looks odd...
		}
	}
	
	free(dev->checkpointBlockList);
	dev->checkpointBlockList = NULL;
	dev->nFreeChunks -= dev->blocksInCheckpoint * dev->nChunksPerBlock;
	dev->nErasedBlocks -= dev->blocksInCheckpoint; 
	
	if(dev->checkpointBuffer){
		/* free the buffer */
		free(dev->checkpointBuffer);
		dev->checkpointBuffer = NULL;
		return 1;
	}
	else
		return 0;
}


int yaffs_ReadChunkWithTagsFromNAND(yaffs_Device * dev, int chunkInNAND, __u8 * buffer, yaffs_ExtendedTags * tags)
{
	int result;
	yaffs_ExtendedTags localTags;

	int realignedChunkInNAND = chunkInNAND - dev->chunkOffset;

	/* If there are no tags provided, use local tags to get prioritised gc working */
	if(!tags)
		tags = &localTags;

	result = dev->readChunkWithTagsFromNAND(dev, realignedChunkInNAND, buffer, tags);

	/*if(tags && tags->eccResult > YAFFS_ECC_RESULT_NO_ERROR){

		yaffs_BlockInfo *bi = yaffs_GetBlockInfo(dev, chunkInNAND/dev->nChunksPerBlock);
                yaffs_HandleChunkError(dev,bi);
	}
	*/
	return result;
}

/*-------------------- Data file manipulation -----------------*/

static int yaffs_FindChunkInFile(yaffs_Object * in, int chunkInInode, yaffs_ExtendedTags * tags)
{
	/*Get the Tnode, then get the level 0 offset chunk offset */
	yaffs_Tnode *tn;
	int theChunk = -1;
	yaffs_ExtendedTags localTags;
	int retVal = -1;

	yaffs_Device *dev = in->myDev;

	if (!tags)
		tags = &localTags;

	tn = yaffs_FindLevel0Tnode(dev, &in->variant.fileVariant, chunkInInode);

	if (tn) 
	{
		theChunk = yaffs_GetChunkGroupBase(dev,tn,chunkInInode);

		retVal = yaffs_FindChunkInGroup(dev, theChunk, tags, in->objectId, chunkInInode);
	}
	return retVal;
}


static int yaffs_PutChunkIntoFile(yaffs_Object * in, int chunkInInode, int chunkInNAND, int inScan)
{
	/* NB inScan is zero unless scanning. 
	 * For forward scanning, inScan is > 0; 
	 * for backward scanning inScan is < 0
	 */
	 
	yaffs_Tnode *tn;
	yaffs_Device *dev = in->myDev;
	int existingChunk;
	yaffs_ExtendedTags existingTags;
	yaffs_ExtendedTags newTags;
	unsigned existingSerial, newSerial;

	if (in->variantType != YAFFS_OBJECT_TYPE_FILE) 
	{
		/* Just ignore an attempt at putting a chunk into a non-file during scanning
		 * If it is not during Scanning then something went wrong!
		 */
		if (!inScan)
			printf("yaffs tragedy:attempt to put data chunk into a non-file\n");
			
		//yaffs_DeleteChunk(dev, chunkInNAND, 1, __LINE__);
		return YAFFS_OK;
	}

	tn = yaffs_AddOrFindLevel0Tnode(dev, &in->variant.fileVariant,	chunkInInode, NULL);
	if (!tn)
		return YAFFS_FAIL;

	existingChunk = yaffs_GetChunkGroupBase(dev,tn,chunkInInode);

	if (inScan != 0) {
		/* If we're scanning then we need to test for duplicates
		 * NB This does not need to be efficient since it should only ever 
		 * happen when the power fails during a write, then only one
		 * chunk should ever be affected.
		 *
		 * Correction for YAFFS2: This could happen quite a lot and we need to think about efficiency! TODO
		 * Update: For backward scanning we don't need to re-read tags so this is quite cheap.
		 */

		if (existingChunk > 0)
		{
			/* NB Right now existing chunk will not be real chunkId if the device >= 32MB
			 *    thus we have to do a FindChunkInFile to get the real chunk id.
			 *
			 * We have a duplicate now we need to decide which one to use:
			 *
			 * Backwards scanning YAFFS2: The old one is what we use, dump the new one.
			 * Forward scanning YAFFS2: The new one is what we use, dump the old one.
			 * YAFFS1: Get both sets of tags and compare serial numbers.
			 */

			if (inScan > 0) 
			{
				/* Only do this for forward scanning */
				yaffs_ReadChunkWithTagsFromNAND(dev, chunkInNAND, NULL, &newTags);

				/* Do a proper find */
				existingChunk = yaffs_FindChunkInFile(in, chunkInInode, &existingTags);
			}

			if (existingChunk <= 0) 
				printf("yaffs tragedy: existing chunk < 0 in scan\n");

			/* NB The deleted flags should be false, otherwise the chunks will 
			 * not be loaded during a scan
			 */
			newSerial = newTags.serialNumber;
			existingSerial = existingTags.serialNumber;

			if ((inScan > 0) && (in->myDev->isYaffs2 || existingChunk <= 0 || ((existingSerial + 1) & 3) == newSerial)) 
			{
				/* Forward scanning.                            
				 * Use new
				 * Delete the old one and drop through to update the tnode
				 */
				//yaffs_DeleteChunk(dev, existingChunk, 1, __LINE__);
			} 
			else 
			{
				/* Backward scanning or we want to use the existing one
				 * Use existing.
				 * Delete the new one and return early so that the tnode isn't changed
				 */
				//yaffs_DeleteChunk(dev, chunkInNAND, 1, __LINE__);
				return YAFFS_OK;
			}
		}

	}

	if (existingChunk == 0) {
		in->nDataChunks++;
	}

	yaffs_PutLevel0Tnode(dev,tn,chunkInInode,chunkInNAND);

	return YAFFS_OK;
}

static int yaffs_ReadChunkDataFromObject(yaffs_Object * in, int chunkInInode, __u8 * buffer)
{
	int chunkInNAND = yaffs_FindChunkInFile(in, chunkInInode, NULL);

	if (chunkInNAND >= 0) 
		return yaffs_ReadChunkWithTagsFromNAND(in->myDev, chunkInNAND, buffer,NULL);
	else
	{
		printf("Chunk %d not found zero instead\n", chunkInNAND);

		/* get sane (zero) data if you read a hole */
		memset(buffer, 0, in->myDev->nDataBytesPerChunk);	
		return 0;
	}

}

static void yaffs_CheckpointObjectToObject( yaffs_Object *obj,yaffs_CheckpointObject *cp)
{

	yaffs_Object *parent;	
	obj->objectId = cp->objectId;
	
	if(cp->parentId)
		parent = yaffs_FindOrCreateObjectByNumber(obj->myDev, cp->parentId, YAFFS_OBJECT_TYPE_DIRECTORY);
	else
		parent = NULL;
	
	if(parent)
		yaffs_AddObjectToDirectory(parent, obj);
		
	obj->hdrChunk = cp->hdrChunk;
	obj->variantType = cp->variantType;
	obj->deleted = cp->deleted;
	obj->softDeleted = cp->softDeleted;
	obj->unlinked = cp->unlinked;
	obj->fake = cp->fake;
	obj->renameAllowed = cp->renameAllowed;
	obj->unlinkAllowed = cp->unlinkAllowed;
	obj->serial = cp->serial;
	obj->nDataChunks = cp->nDataChunks;
	
	if(obj->variantType == YAFFS_OBJECT_TYPE_FILE)
		obj->variant.fileVariant.fileSize = cp->fileSizeOrEquivalentObjectId;
	else if(obj->variantType == YAFFS_OBJECT_TYPE_HARDLINK)
		obj->variant.hardLinkVariant.equivalentObjectId = cp->fileSizeOrEquivalentObjectId;
	
	if(obj->hdrChunk > 0)
		obj->lazyLoaded = 1;
}

static int yaffs_ReadCheckpointValidityMarker(yaffs_Device *dev, int head)
{
	yaffs_CheckpointValidity cp;
	int ok;
	
	ok = (yaffs_CheckpointRead(dev,&cp,sizeof(cp)) == sizeof(cp));
	
	if(ok)
		ok = (cp.structType == sizeof(cp)) &&
		     (cp.magic == YAFFS_MAGIC) &&
		     (cp.version == YAFFS_CHECKPOINT_VERSION) &&
		     (cp.head == ((head) ? 1 : 0));
	return ok ? 1 : 0;
}

static void yaffs_DeviceToCheckpointDevice(yaffs_CheckpointDevice *cp, yaffs_Device *dev)
{
	cp->nErasedBlocks = dev->nErasedBlocks;
	cp->allocationBlock = dev->allocationBlock;
	cp->allocationPage = dev->allocationPage;
	cp->nFreeChunks = dev->nFreeChunks;
	
	cp->nDeletedFiles = dev->nDeletedFiles;
	cp->nUnlinkedFiles = dev->nUnlinkedFiles;
	cp->nBackgroundDeletions = dev->nBackgroundDeletions;
	cp->sequenceNumber = dev->sequenceNumber;
	cp->oldestDirtySequence = dev->oldestDirtySequence;
	
}


static void yaffs_CheckpointDeviceToDevice(yaffs_Device *dev, yaffs_CheckpointDevice *cp);
static int yaffs_ReadCheckpointDevice(yaffs_Device *dev)
{
	yaffs_CheckpointDevice cp;
	__u32 nBytes;
	__u32 nBlocks = (dev->internalEndBlock - dev->internalStartBlock + 1);

	int ok;	
	
	ok = (yaffs_CheckpointRead(dev,&cp,sizeof(cp)) == sizeof(cp));
	if(!ok)
		return 0;
		
	if(cp.structType != sizeof(cp))
		return 0;
		
	
	yaffs_CheckpointDeviceToDevice(dev,&cp);
	
	nBytes = nBlocks * sizeof(yaffs_BlockInfo);
	
	ok = (yaffs_CheckpointRead(dev,dev->blockInfo,nBytes) == nBytes);
	
	if(!ok)
		return 0;
	nBytes = nBlocks * dev->chunkBitmapStride;
	
	ok = (yaffs_CheckpointRead(dev,dev->chunkBits,nBytes) == nBytes);
	
	return ok ? 1 : 0;
}


static void yaffs_CheckpointDeviceToDevice(yaffs_Device *dev, yaffs_CheckpointDevice *cp)
{
	dev->nErasedBlocks = cp->nErasedBlocks;
	dev->allocationBlock = cp->allocationBlock;
	dev->allocationPage = cp->allocationPage;
	dev->nFreeChunks = cp->nFreeChunks;
	dev->nDeletedFiles = cp->nDeletedFiles;
	dev->nUnlinkedFiles = cp->nUnlinkedFiles;
	dev->nBackgroundDeletions = cp->nBackgroundDeletions;
	dev->sequenceNumber = cp->sequenceNumber;
	dev->oldestDirtySequence = cp->oldestDirtySequence;
}


static int yaffs_ReadCheckpointTnodes(yaffs_Object *obj)
{
	__u32 baseChunk;
	int ok = 1;
	yaffs_Device *dev = obj->myDev;
	yaffs_FileStructure *fileStructPtr = &obj->variant.fileVariant;
	yaffs_Tnode *tn;
	int nread = 0;
	int tnodeSize = (dev->tnodeWidth * YAFFS_NTNODES_LEVEL0)/8;

	if(tnodeSize < sizeof(yaffs_Tnode))
		tnodeSize = sizeof(yaffs_Tnode);

	ok = (yaffs_CheckpointRead(dev,&baseChunk,sizeof(baseChunk)) == sizeof(baseChunk));
	
	while(ok && (~baseChunk))
	{
		nread++;	
		
		tn = yaffs_GetTnodeRaw(dev);
		if(tn)
			ok = (yaffs_CheckpointRead(dev,tn,tnodeSize) == tnodeSize);
		else
			ok = 0;
			
		if(tn && ok)
			ok = yaffs_AddOrFindLevel0Tnode(dev, fileStructPtr, baseChunk, tn) ? 1 : 0;
			
		if(ok)
			ok = (yaffs_CheckpointRead(dev,&baseChunk,sizeof(baseChunk)) == sizeof(baseChunk));
		
	}

	return ok ? 1 : 0;	
}
 

static void yaffs_HardlinkFixup(yaffs_Device *dev, yaffs_Object *hardList);
static int yaffs_ReadCheckpointObjects(yaffs_Device *dev)
{
	yaffs_Object *obj;
	yaffs_CheckpointObject cp;
	int ok = 1;
	int done = 0;
	yaffs_Object *hardList = NULL;
	
	while(ok && !done) 
	{
		ok = (yaffs_CheckpointRead(dev,&cp,sizeof(cp)) == sizeof(cp));
		if(cp.structType != sizeof(cp))
			ok = 0;			
		if(ok && cp.objectId == ~0)
			done = 1;
		else if(ok)
		{
			obj = yaffs_FindOrCreateObjectByNumber(dev,cp.objectId, cp.variantType);
			if(obj) 
			{
				yaffs_CheckpointObjectToObject(obj,&cp);
				if(obj->variantType == YAFFS_OBJECT_TYPE_FILE) 
                                        ok = yaffs_ReadCheckpointTnodes(obj);
				else if(obj->variantType == YAFFS_OBJECT_TYPE_HARDLINK) 
				{
                                        obj->hardLinks.next = (struct ylist_head *)hardList;
                                        hardList = obj;
                                }
			}
		}
	}
	
	if(ok)
		yaffs_HardlinkFixup(dev,hardList);
	
	return ok ? 1 : 0;
}


static int yaffs_ReadCheckpointSum(yaffs_Device *dev)
{
	__u32 checkpointSum0;
	__u32 checkpointSum1;
	int ok;
	
	yaffs_GetCheckpointSum(dev,&checkpointSum0);
	
	ok = (yaffs_CheckpointRead(dev,&checkpointSum1,sizeof(checkpointSum1)) == sizeof(checkpointSum1));
	
	if(!ok)
		return 0;
		
	if(checkpointSum0 != checkpointSum1)
		return 0;
	
	return 1;
}


static int yaffs_ReadCheckpointData(yaffs_Device *dev)
{
	int ok = 1;
	
	if(dev->skipCheckpointRead || !dev->isYaffs2)
	{
		printf("skip checkpoint!\n");
		ok = 0;
	}
	
	if(ok)
		ok = yaffs_CheckpointOpen(dev,0); /* open for read */
	
	if(ok)
	{
		//printf("read checkpoint validity\n");	
		ok = yaffs_ReadCheckpointValidityMarker(dev,1);
	}
	if(ok)
	{
		//printf("read checkpoint device!\n");
		ok = yaffs_ReadCheckpointDevice(dev);
	}
	if(ok)
	{
		//printf("read checkpoint objects\n");	
		ok = yaffs_ReadCheckpointObjects(dev);
	}
	if(ok)
	{
		//printf("read checkpoint validity\n");
		ok = yaffs_ReadCheckpointValidityMarker(dev,0);
	}
	
	if(ok){
		ok = yaffs_ReadCheckpointSum(dev);
		//printf("read checkpoint checksum \n");
	}

	if(!yaffs_CheckpointClose(dev))
		ok = 0;

	if(ok)
	    	dev->isCheckpointed = 1;
	 else 
	 	dev->isCheckpointed = 0;

	return ok ? 1 : 0;

}

int yaffs_CheckpointRestore(yaffs_Device *dev)
{
	int retval;
		
	retval = yaffs_ReadCheckpointData(dev);

/*	if(dev->isCheckpointed){
		yaffs_VerifyObjects(dev);
		yaffs_VerifyBlocks(dev);
		yaffs_VerifyFreeChunks(dev);
	}
	
	T(YAFFS_TRACE_CHECKPOINT,(TSTR("restore exit: isCheckpointed %d"TENDSTR),dev->isCheckpointed));
*/	
	return retval;
}

/*--------------------- File read/write ------------------------
 * Read and write have very similar structures.
 * In general the read/write has three parts to it
 * An incomplete chunk to start with (if the read/write is not chunk-aligned)
 * Some complete chunks
 * An incomplete chunk to end off with
 *
 * Curve-balls: the first chunk might also be the last chunk.
 */

int yaffs_ReadDataFromFile(yaffs_Object * in, __u8 * buffer, loff_t offset,int nBytes)
{

	int chunk;
	__u32 start;
	int nToCopy;
	int n = nBytes;
	int nDone = 0;

	yaffs_Device *dev;

	dev = in->myDev;

	while (n > 0)
	{
		//chunk = offset / dev->nDataBytesPerChunk + 1;
		//start = offset % dev->nDataBytesPerChunk;
		yaffs_AddrToChunk(dev,offset,&chunk,&start);
		chunk++;

		/* OK now check for the curveball where the start and end are in
		 * the same chunk.      
		 */
		if ((start + n) < dev->nDataBytesPerChunk)
			nToCopy = n;
		else
			nToCopy = dev->nDataBytesPerChunk - start;

		/*copy to buffer directly*/
		if(nToCopy == dev->nDataBytesPerChunk)
			yaffs_ReadChunkDataFromObject(in, chunk, buffer);
		else
		{
			/* Read into the local buffer then copy..*/
			__u8 *localBuffer =yaffs_GetTempBuffer(dev, __LINE__);		
			yaffs_ReadChunkDataFromObject(in, chunk, localBuffer);
			memcpy(buffer, &localBuffer[start], nToCopy);
			yaffs_ReleaseTempBuffer(dev, localBuffer,__LINE__);
		}

		n -= nToCopy;
		offset += nToCopy;
		buffer += nToCopy;
		nDone += nToCopy;
	}

	return nDone;
}


/*----------------------- Initialisation Scanning ---------------------- */
static void yaffs_HandleShadowedObject(yaffs_Device * dev, int objId, int backwardScanning)
{
	yaffs_Object *obj;

	if (!backwardScanning) {
		/* Handle YAFFS1 forward scanning case
		 * For YAFFS1 we always do the deletion
		 */
		return;
	} else 
	{
		/* Handle YAFFS2 case (backward scanning)
		 * If the shadowed object exists then ignore.
		 */
		if (yaffs_FindObjectByNumber(dev, objId))
			return;
	}

	/* Let's create it (if it does not exist) assuming it is a file so that it can do shrinking etc.
	 * We put it in unlinked dir to be cleaned up after the scanning
	 */
	obj = yaffs_FindOrCreateObjectByNumber(dev, objId, YAFFS_OBJECT_TYPE_FILE);
	yaffs_AddObjectToDirectory(dev->unlinkedDir, obj);
	obj->variant.fileVariant.shrinkSize = 0;
	obj->valid = 1;		/* So that we don't read any other info for this file */

}



static void yaffs_HardlinkFixup(yaffs_Device *dev, yaffs_Object *hardList)
{
	yaffs_Object *hl;
	yaffs_Object *in;
	
	while (hardList) 
	{
		hl = hardList;
		hardList = (yaffs_Object *) (hardList->hardLinks.next);

		in = yaffs_FindObjectByNumber(dev, hl->variant.hardLinkVariant. equivalentObjectId);

              if (in) 
		{
                        /* Add the hardlink pointers */
                        hl->variant.hardLinkVariant.equivalentObject = in;
                        ylist_add(&hl->hardLinks, &in->hardLinks);
                } 
		else 
		{
                        /* Todo Need to report/handle this better.
                         * Got a problem... hardlink to a non-existant object
                         */
                        hl->variant.hardLinkVariant.equivalentObject = NULL;
                        YINIT_LIST_HEAD(&hl->hardLinks);
                }
	}
}


static int ybicmp(const void *a, const void *b){
    register int aseq = ((yaffs_BlockIndex *)a)->seq;
    register int bseq = ((yaffs_BlockIndex *)b)->seq;
    register int ablock = ((yaffs_BlockIndex *)a)->block;
    register int bblock = ((yaffs_BlockIndex *)b)->block;
    if( aseq == bseq )
        return ablock - bblock;
    else
        return aseq - bseq;

}

static void yaffs_CheckObjectDetailsLoaded(yaffs_Object *in)
{
	__u8 *chunkData;
	yaffs_ObjectHeader *oh;
	yaffs_Device *dev = in->myDev;
	yaffs_ExtendedTags tags;
	int result;
	int alloc_failed = 0;

	if(!in)
		return;
		

	if(in->lazyLoaded && in->hdrChunk > 0){
		in->lazyLoaded = 0;
		chunkData = yaffs_GetTempBuffer(dev, __LINE__);

		result = yaffs_ReadChunkWithTagsFromNAND(dev,in->hdrChunk,chunkData,&tags);
		oh = (yaffs_ObjectHeader *) chunkData;

		in->yst_mode = oh->yst_mode;

		in->yst_uid = oh->yst_uid;
		in->yst_gid = oh->yst_gid;
		in->yst_atime = oh->yst_atime;
		in->yst_mtime = oh->yst_mtime;
		in->yst_ctime = oh->yst_ctime;
		in->yst_rdev = oh->yst_rdev;

		yaffs_SetObjectName(in, oh->name);
		
		if(in->variantType == YAFFS_OBJECT_TYPE_SYMLINK)
		{
			in->variant.symLinkVariant.alias = yaffs_CloneString(oh->alias);
			if(!in->variant.symLinkVariant.alias)
				alloc_failed = 1; /* Not returned to caller */
		}
						    
		yaffs_ReleaseTempBuffer(dev,chunkData, __LINE__);
	}
}

int yaffs_QueryInitialBlockState(yaffs_Device *dev, int blockNo, yaffs_BlockState *state, __u32 *sequenceNumber)
{
	blockNo -= dev->blockOffset;

	return dev->queryNANDBlock(dev, blockNo, state, sequenceNumber);
}

static int yaffs_ScanBackwards(yaffs_Device * dev)
{
	yaffs_ExtendedTags tags;
	int blk;
	int blockIterator;
	int startIterator;
	int endIterator;
	int nBlocksToScan = 0;

	int chunk;
	int result;
	int c;
	int deleted;
	yaffs_BlockState state;
	yaffs_Object *hardList = NULL;
	yaffs_BlockInfo *bi;
	__u32 sequenceNumber;
	yaffs_ObjectHeader *oh;
	yaffs_Object *in;
	yaffs_Object *parent;
	int nBlocks = dev->internalEndBlock - dev->internalStartBlock + 1;
	int itsUnlinked;
	__u8 *chunkData;
	
	int fileSize;
	int isShrink;
	int foundChunksInBlock;
	int equivalentObjectId;
	int alloc_failed = 0;
	

	yaffs_BlockIndex *blockIndex = NULL;
	int altBlockIndex = 0;

	if (!dev->isYaffs2) 
	{
		printf("yaffs_ScanBackwards is only for YAFFS2!\n");
		return YAFFS_FAIL;
	}

	dev->sequenceNumber = YAFFS_LOWEST_SEQUENCE_NUMBER;
	blockIndex = malloc(nBlocks * sizeof(yaffs_BlockIndex));	
	if(!blockIndex) 
	{
		blockIndex = malloc(nBlocks * sizeof(yaffs_BlockIndex));
		altBlockIndex = 1;
	}
	if(!blockIndex) 
	{
		printf("yaffs_ScanBackwards() could not allocate block index!\n");
		return YAFFS_FAIL;
	}
	
	dev->blocksInCheckpoint = 0;
	
	chunkData = yaffs_GetTempBuffer(dev, __LINE__);

	/* Scan all the blocks to determine their state */
	for (blk = dev->internalStartBlock; blk <= dev->internalEndBlock; blk++) 
	{
		bi = &dev->blockInfo[blk - dev->internalStartBlock];
		yaffs_ClearChunkBits(dev, blk);
		bi->pagesInUse = 0;
		bi->softDeletions = 0;

		yaffs_QueryInitialBlockState(dev, blk, &state, &sequenceNumber);

		bi->blockState = state;
		bi->sequenceNumber = sequenceNumber;

		if(bi->sequenceNumber == YAFFS_SEQUENCE_CHECKPOINT_DATA)
			bi->blockState = state = YAFFS_BLOCK_STATE_CHECKPOINT;
			
		if(state == YAFFS_BLOCK_STATE_CHECKPOINT)
			dev->blocksInCheckpoint++;
		else if (state == YAFFS_BLOCK_STATE_DEAD)
			  printf("block %d is bad\n",blk);
		else if (state == YAFFS_BLOCK_STATE_EMPTY) 
		{
			dev->nErasedBlocks++;
			dev->nFreeChunks += dev->nChunksPerBlock;
		} 
		else if (state == YAFFS_BLOCK_STATE_NEEDS_SCANNING) 
		{

			/* Determine the highest sequence number */
			if (sequenceNumber >= YAFFS_LOWEST_SEQUENCE_NUMBER && sequenceNumber < YAFFS_HIGHEST_SEQUENCE_NUMBER) 
			{
				blockIndex[nBlocksToScan].seq = sequenceNumber;
				blockIndex[nBlocksToScan].block = blk;
				nBlocksToScan++;

				if (sequenceNumber >= dev->sequenceNumber)
					dev->sequenceNumber = sequenceNumber;
			} 
			else 
			{
				/* TODO: Nasty sequence number! */
				printf("Block scanning block %d has bad sequence number %d\n", blk, sequenceNumber);
			}
		}
	}

	{
		/* Dungy old bubble sort... */
	 	yaffs_BlockIndex temp;
		int i;
		int j;

		for (i = 0; i < nBlocksToScan; i++)
			for (j = i + 1; j < nBlocksToScan; j++)
				if (blockIndex[i].seq > blockIndex[j].seq)
				{
					temp = blockIndex[j];
					blockIndex[j] = blockIndex[i];
					blockIndex[i] = temp;
				}
	}

	/* Now scan the blocks looking at the data. */
	startIterator = 0;
	endIterator = nBlocksToScan - 1;

	/* For each block.... backwards */
	for (blockIterator = endIterator; !alloc_failed && blockIterator >= startIterator; blockIterator--) 
	{
	        /* Cooperative multitasking! This loop can run for so
		   long that watchdog timers expire. */

		/* get the block to scan in the correct order */
		blk = blockIndex[blockIterator].block;
		bi = &dev->blockInfo[blk - dev->internalStartBlock];
		
		state = bi->blockState;
		deleted = 0;

		/* For each chunk in each block that needs scanning.... */
		foundChunksInBlock = 0;
		for (c = dev->nChunksPerBlock - 1; !alloc_failed && c >= 0 &&
		     (state == YAFFS_BLOCK_STATE_NEEDS_SCANNING ||
		      state == YAFFS_BLOCK_STATE_ALLOCATING); c--) 
		{
			/* Scan backwards... 
			 * Read the tags and decide what to do
			 */
			
			chunk = blk * dev->nChunksPerBlock + c;

			result = yaffs_ReadChunkWithTagsFromNAND(dev, chunk, NULL, &tags);

			/* Let's have a good look at this chunk... */

			if (!tags.chunkUsed) 
			{
				/* An unassigned chunk in the block.
				 * If there are used chunks after this one, then
				 * it is a chunk that was skipped due to failing the erased
				 * check. Just skip it so that it can be deleted.
				 * But, more typically, We get here when this is an unallocated
				 * chunk and his means that either the block is empty or 
				 * this is the one being allocated from
				 */

				if(foundChunksInBlock)
				{
					/* This is a chunk that was skipped due to failing the erased check */
					
				} else if (c == 0) 
				{
					/* We're looking at the first chunk in the block so the block is unused */
					state = YAFFS_BLOCK_STATE_EMPTY;
					dev->nErasedBlocks++;
				}
				else
				{
					if (state == YAFFS_BLOCK_STATE_NEEDS_SCANNING || state == YAFFS_BLOCK_STATE_ALLOCATING) 
					{
					    	if(dev->sequenceNumber == bi->sequenceNumber) 
						{
							/* this is the block being allocated from */
							state = YAFFS_BLOCK_STATE_ALLOCATING;
							dev->allocationBlock = blk;
							dev->allocationPage = c;
							dev->allocationBlockFinder = blk;	
						}
						else 
						{
							/* This is a partially written block that is not
							 * the current allocation block. This block must have
							 * had a write failure, so set up for retirement.
							 */
						  
							 /* bi->needsRetiring = 1; ??? TODO */
							 bi->gcPrioritise = 1;
						}
					}
				}
				dev->nFreeChunks++;
			} 
			else if (tags.chunkId > 0) 
			{
				/* chunkId > 0 so it is a data chunk... */
				unsigned int endpos;
				__u32 chunkBase = (tags.chunkId - 1) * dev->nDataBytesPerChunk;
								
				foundChunksInBlock = 1;


				yaffs_SetChunkBit(dev, blk, c);
				bi->pagesInUse++;

				in = yaffs_FindOrCreateObjectByNumber(dev, tags. objectId, YAFFS_OBJECT_TYPE_FILE);
				if(!in)
					alloc_failed = 1;
				
				if (in && in->variantType == YAFFS_OBJECT_TYPE_FILE 
					&& chunkBase < in->variant.fileVariant.shrinkSize) 
				{
					/* This has not been invalidated by a resize */
					if(!yaffs_PutChunkIntoFile(in, tags.chunkId, chunk, -1))
						alloc_failed = 1;

					/* File size is calculated by looking at the data chunks if we have not 
					 * seen an object header yet. Stop this practice once we find an object header.
					 */
					endpos = (tags.chunkId - 1) * dev->nDataBytesPerChunk + tags.byteCount;
					    
					if (!in->valid && in->variant.fileVariant.scannedFileSize < endpos) 
					{
						in->variant.fileVariant.scannedFileSize = endpos;
						in->variant.fileVariant.fileSize = in->variant.fileVariant.scannedFileSize;
					}

				} 
				else if(in) 
				{
					/* This chunk has been invalidated by a resize, so delete */
				//	yaffs_DeleteChunk(dev, chunk, 1, __LINE__);

				}
			} 
			else 
			{
				/* chunkId == 0, so it is an ObjectHeader.
				 * Thus, we read in the object header and make the object
				 */
				foundChunksInBlock = 1;

				yaffs_SetChunkBit(dev, blk, c);
				bi->pagesInUse++;

				oh = NULL;
				in = NULL;

				if (tags.extraHeaderInfoAvailable) 
					in = yaffs_FindOrCreateObjectByNumber(dev, tags.objectId, tags.extraObjectType);

				if (!in || tags.extraShadows || (!in->valid && (tags.objectId == YAFFS_OBJECTID_ROOT ||
				     tags.objectId == YAFFS_OBJECTID_LOSTNFOUND))) 
				{

					/* If we don't have  valid info then we need to read the chunk
					 * TODO In future we can probably defer reading the chunk and 
					 * living with invalid data until needed.
					 */

					result = yaffs_ReadChunkWithTagsFromNAND(dev, chunk, chunkData,	NULL);

					oh = (yaffs_ObjectHeader *) chunkData;
					
					if(dev->inbandTags)
					{
						/* Fix up the header if they got corrupted by inband tags */
						oh->shadowsObject = oh->inbandShadowsObject;
						oh->isShrink = oh->inbandIsShrink;
					}

					if (!in)
						in = yaffs_FindOrCreateObjectByNumber(dev, tags.objectId, oh->type);

				}

				if (!in)
					printf("yaffs tragedy: Could not make object for object  %d at chunk %d during scan", tags.objectId, chunk);


				if (in->valid) 
				{
					/* We have already filled this one.
					 * We have a duplicate that will be discarded, but 
					 * we first have to suck out resize info if it is a file.
					 */

					if ((in->variantType == YAFFS_OBJECT_TYPE_FILE) 
						&& ((oh && oh-> type == YAFFS_OBJECT_TYPE_FILE)||
						(tags.extraHeaderInfoAvailable && tags.extraObjectType == YAFFS_OBJECT_TYPE_FILE)))
					{
						__u32 thisSize = (oh) ? oh->fileSize : tags.extraFileLength;
						__u32 parentObjectId = (oh) ? oh->parentObjectId : tags.extraParentObjectId;
						unsigned isShrink = (oh) ? oh->isShrink : tags.extraIsShrinkHeader;

						/* If it is deleted (unlinked at start also means deleted)
						 * we treat the file size as being zeroed at this point.
						 */
						if (parentObjectId == YAFFS_OBJECTID_DELETED 
							|| parentObjectId == YAFFS_OBJECTID_UNLINKED) 
						{
							thisSize = 0;
							isShrink = 1;
						}

						if (isShrink && in->variant.fileVariant.shrinkSize > thisSize) 
							in->variant.fileVariant.shrinkSize = thisSize;

						if (isShrink)
							bi->hasShrinkHeader = 1;

					}
				}

				if (!in->valid && (tags.objectId == YAFFS_OBJECTID_ROOT ||
				     tags.objectId == YAFFS_OBJECTID_LOSTNFOUND)) 
				{
					/* We only load some info, don't fiddle with directory structure */
					in->valid = 1;
					
					if(oh)
					{
						in->variantType = oh->type;
						in->yst_uid = oh->yst_uid;
						in->yst_gid = oh->yst_gid;
						in->yst_atime = oh->yst_atime;
						in->yst_mtime = oh->yst_mtime;
						in->yst_ctime = oh->yst_ctime;
						in->yst_rdev = oh->yst_rdev;
		
					} 
					else 
					{
						in->variantType = tags.extraObjectType;
						in->lazyLoaded = 1;
					}

					in->hdrChunk = chunk;

				} 
				else if (!in->valid) 
				{
					/* we need to load this info */

					in->valid = 1;
					in->hdrChunk = chunk;

					if(oh) 
					{
						in->variantType = oh->type;
						in->yst_mode = oh->yst_mode;
						in->yst_uid = oh->yst_uid;
						in->yst_gid = oh->yst_gid;
						in->yst_atime = oh->yst_atime;
						in->yst_mtime = oh->yst_mtime;
						in->yst_ctime = oh->yst_ctime;
						in->yst_rdev = oh->yst_rdev;

						if (oh->shadowsObject > 0) 
							yaffs_HandleShadowedObject(dev, oh->shadowsObject, 1);
					
						yaffs_SetObjectName(in, oh->name);
						parent = yaffs_FindOrCreateObjectByNumber(dev, oh->parentObjectId, 
							YAFFS_OBJECT_TYPE_DIRECTORY);

						fileSize = oh->fileSize;
 						 isShrink = oh->isShrink;
						 equivalentObjectId = oh->equivalentObjectId;
					}
					else 
					{
						in->variantType = tags.extraObjectType;
						parent = yaffs_FindOrCreateObjectByNumber(dev, tags.extraParentObjectId,
					     		 YAFFS_OBJECT_TYPE_DIRECTORY);
					  	fileSize = tags.extraFileLength;
						isShrink = tags.extraIsShrinkHeader;
						equivalentObjectId = tags.extraEquivalentObjectId;
						in->lazyLoaded = 1;

					}
					in->dirty = 0;

					/* directory stuff...
					 * hook up to parent
					 */

					if (parent->variantType == YAFFS_OBJECT_TYPE_UNKNOWN) 
					{
                                                /* Set up as a directory */
                                                parent->variantType = YAFFS_OBJECT_TYPE_DIRECTORY;
                                                YINIT_LIST_HEAD(&parent->variant.directoryVariant.children);
                                        } 
					else if (parent->variantType != YAFFS_OBJECT_TYPE_DIRECTORY)
					{
						printf("yaffs tragedy: attempting to use non-directory as a directory in scan. Put in lost+found.");
						parent = dev->lostNFoundDir;
					}

					yaffs_AddObjectToDirectory(parent, in);

					itsUnlinked = (parent == dev->deletedDir) || (parent == dev->unlinkedDir);

					if (isShrink)
						/* Mark the block as having a shrinkHeader */
						bi->hasShrinkHeader = 1;

					/* Note re hardlinks.
					 * Since we might scan a hardlink before its equivalent object is scanned
					 * we put them all in a list.
					 * After scanning is complete, we should have all the objects, so we run
					 * through this list and fix up all the chains.              
					 */

					switch (in->variantType) 
					{
					case YAFFS_OBJECT_TYPE_UNKNOWN:	
						/* Todo got a problem */
						break;
					case YAFFS_OBJECT_TYPE_FILE:

						if (in->variant.fileVariant.
						    scannedFileSize < fileSize) 
						{
							/* This covers the case where the file size is greater
							 * than where the data is
							 * This will happen if the file is resized to be larger 
							 * than its current data extents.
							 */
							in->variant.fileVariant.fileSize = fileSize;
							in->variant.fileVariant.scannedFileSize = in->variant.fileVariant.fileSize;
						}

						if (isShrink && in->variant.fileVariant.shrinkSize > fileSize)
							in->variant.fileVariant.shrinkSize = fileSize;
						break;
					case YAFFS_OBJECT_TYPE_HARDLINK:
						if(!itsUnlinked) 
						{
                                                	in->variant.hardLinkVariant.equivalentObjectId = equivalentObjectId;
	                                                in->hardLinks.next = (struct ylist_head *) hardList;
                                                 	 hardList = in;
						 }
                                                 break;
					case YAFFS_OBJECT_TYPE_DIRECTORY:
						/* Do nothing */
						break;
					case YAFFS_OBJECT_TYPE_SPECIAL:
						/* Do nothing */
						break;
					case YAFFS_OBJECT_TYPE_SYMLINK:
						if(oh)
						{
							in->variant.symLinkVariant.alias = yaffs_CloneString(oh->alias);
	                                                if(!in->variant.symLinkVariant.alias)
						   		alloc_failed = 1;
						}
						break;
					}
				}
			}
		} /* End of scanning for each chunk */

		if (state == YAFFS_BLOCK_STATE_NEEDS_SCANNING) 
			state = YAFFS_BLOCK_STATE_FULL;

		bi->blockState = state;

		/* Now let's see if it was dirty */
		if (bi->pagesInUse == 0 && !bi->hasShrinkHeader && bi->blockState == YAFFS_BLOCK_STATE_FULL) ;
			//yaffs_BlockBecameDirty(dev, blk);

	}

	if (altBlockIndex) 
		free(blockIndex);
	else
		free(blockIndex);
	
	/* Ok, we've done all the scanning.
	 * Fix up the hard link chains.
	 * We should now have scanned all the objects, now it's time to add these 
	 * hardlinks.
	 */
	yaffs_HardlinkFixup(dev,hardList);
	
	
	/*
        *  Sort out state of unlinked and deleted objects.
        
        {
                struct ylist_head *i;
                struct ylist_head *n;

                yaffs_Object *l;

                // Soft delete all the unlinked files 
                ylist_for_each_safe(i, n, &dev->unlinkedDir->variant.directoryVariant.children) 
		{
                        if (i) 
			{
                                l = ylist_entry(i, yaffs_Object, siblings);
                                yaffs_DestroyObject(l);
                        }
                }

                //Soft delete all the deletedDir files 
                ylist_for_each_safe(i, n,
                                   &dev->deletedDir->variant.directoryVariant.
                                   children) {
                        if (i) {
                                l = ylist_entry(i, yaffs_Object, siblings);
                                yaffs_DestroyObject(l);

                        }
		}
	}
	*/
	yaffs_ReleaseTempBuffer(dev, chunkData, __LINE__);
	
	if(alloc_failed)
		return YAFFS_FAIL;

	printf("yaffs_ScanBackwards ends\n" );

	return YAFFS_OK;
}

/*------------------------------  Directory Functions ----------------------------- */
static void yaffs_AddObjectToDirectory(yaffs_Object * directory, yaffs_Object * obj)
{

	if (!directory) 
	{
		printf("yaffs_AddObjectToDirectory:Null pointer!\n");
		return;
	}
	if (directory->variantType != YAFFS_OBJECT_TYPE_DIRECTORY) 
	{
		printf("yaffs_AddObjectToDirectory:need a directory!\n");
		return;
	}

        if (obj->siblings.prev == NULL)
                /* Not initialised */
                YINIT_LIST_HEAD(&obj->siblings);
	else if (!ylist_empty(&obj->siblings)) 
	{
                /* If it is holed up somewhere else, un hook it */
               // yaffs_RemoveObjectFromDirectory(obj);
        }
        /* Now add it */
        ylist_add(&obj->siblings, &directory->variant.directoryVariant.children);
        obj->parent = directory;

        if (directory == obj->myDev->unlinkedDir || directory == obj->myDev->deletedDir) 
	{
		obj->unlinked = 1;
		obj->myDev->nUnlinkedFiles++;
		obj->renameAllowed = 0;
	}
}

yaffs_Object *yaffs_FindObjectByName(yaffs_Object * directory, const char * name)
{
        int sum;
        struct ylist_head *i;
        char buffer[YAFFS_MAX_NAME_LENGTH + 1];

        yaffs_Object *l;

	if (!name)
		return NULL;

	if (directory->variantType != YAFFS_OBJECT_TYPE_DIRECTORY) 
	{
		printf("yaffs_FindObjectByName:need directory!\n");
		return NULL;
	}
        sum = yaffs_CalcNameSum(name);

        ylist_for_each(i, &directory->variant.directoryVariant.children) 
	{
                if (i)
		{
                        l = ylist_entry(i, yaffs_Object, siblings);
                        
                        yaffs_CheckObjectDetailsLoaded(l);
			
			/* Special case for lost-n-found */
			if (l->objectId == YAFFS_OBJECTID_LOSTNFOUND) 
			{
				if (strcmp(name, YAFFS_LOSTNFOUND_NAME) == 0)
					return l;
			} 
			else if (l->sum == sum || l->hdrChunk <= 0)
			{
				/* LostnFound chunk called Objxxx
				 * Do a real check
				 */
				yaffs_GetObjectName(l, buffer, YAFFS_MAX_NAME_LENGTH);
				if (strncmp(name, buffer,YAFFS_MAX_NAME_LENGTH) == 0) 
					return l;
			}
		}
	}

	return NULL;
}

/* GetEquivalentObject dereferences any hard links to get to the
 * actual object.
 */
yaffs_Object *yaffs_GetEquivalentObject(yaffs_Object * obj)
{
	if (obj && obj->variantType == YAFFS_OBJECT_TYPE_HARDLINK) 
	{
		/* We want the object id of the equivalent object, not this one */
		obj = obj->variant.hardLinkVariant.equivalentObject;
		yaffs_CheckObjectDetailsLoaded(obj);
	}
	return obj;

}


int yaffs_GetObjectName(yaffs_Object * obj, char * name, int buffSize)
{
	memset(name, 0, buffSize * sizeof(char));
	
	yaffs_CheckObjectDetailsLoaded(obj);

	if (obj->objectId == YAFFS_OBJECTID_LOSTNFOUND)
		strncpy(name, YAFFS_LOSTNFOUND_NAME, buffSize - 1);
	else if (obj->hdrChunk <= 0) 
	{
		char locName[20];
		/* make up a name */
		sprintf(locName, "%s%d", YAFFS_LOSTNFOUND_PREFIX, obj->objectId);
		strncpy(name, locName, buffSize - 1);
	}
	else 
	{
		int result;
		__u8 *buffer = yaffs_GetTempBuffer(obj->myDev, __LINE__);
		yaffs_ObjectHeader *oh = (yaffs_ObjectHeader *) buffer;

		memset(buffer, 0, obj->myDev->nDataBytesPerChunk);

		if (obj->hdrChunk > 0)
			result = yaffs_ReadChunkWithTagsFromNAND(obj->myDev, obj->hdrChunk, buffer, NULL);

		strncpy(name, oh->name, buffSize - 1);

		yaffs_ReleaseTempBuffer(obj->myDev, buffer, __LINE__);
	}

	return strlen(name);
}

int yaffs_GetObjectFileLength(yaffs_Object * obj)
{

	/* Dereference any hard linking */
	obj = yaffs_GetEquivalentObject(obj);
 
	if (obj->variantType == YAFFS_OBJECT_TYPE_FILE)
		return obj->variant.fileVariant.fileSize;
	
	if (obj->variantType == YAFFS_OBJECT_TYPE_SYMLINK)
		return strlen(obj->variant.symLinkVariant.alias);
	else
		/* Only a directory should drop through to here */
		return obj->myDev->nDataBytesPerChunk;
}

unsigned yaffs_GetObjectType(yaffs_Object * obj)
{
	obj = yaffs_GetEquivalentObject(obj);

	switch (obj->variantType) {
	case YAFFS_OBJECT_TYPE_FILE:
		return DT_REG;
		break;
	case YAFFS_OBJECT_TYPE_DIRECTORY:
		return DT_DIR;
		break;
	case YAFFS_OBJECT_TYPE_SYMLINK:
		return DT_LNK;
		break;
	case YAFFS_OBJECT_TYPE_HARDLINK:
		return DT_REG;
		break;
	case YAFFS_OBJECT_TYPE_SPECIAL:
		if (S_ISFIFO(obj->yst_mode))
			return DT_FIFO;
		if (S_ISCHR(obj->yst_mode))
			return DT_CHR;
		if (S_ISBLK(obj->yst_mode))
			return DT_BLK;
		if (S_ISSOCK(obj->yst_mode))
			return DT_SOCK;
	default:
		return DT_REG;
		break;
	}
}


/*---------------------------- Initialisation code -------------------------------------- */
static int yaffs_CreateInitialDirectories(yaffs_Device *dev)
{
	/* Initialise the unlinked, deleted, root and lost and found directories */
	
	dev->lostNFoundDir = dev->rootDir =  NULL;
	dev->unlinkedDir = dev->deletedDir = NULL;

	dev->unlinkedDir = yaffs_CreateFakeDirectory(dev, YAFFS_OBJECTID_UNLINKED, S_IFDIR);
	
	dev->deletedDir = yaffs_CreateFakeDirectory(dev, YAFFS_OBJECTID_DELETED, S_IFDIR);

	dev->rootDir = yaffs_CreateFakeDirectory(dev, YAFFS_OBJECTID_ROOT, YAFFS_ROOT_MODE | S_IFDIR);
	dev->lostNFoundDir = yaffs_CreateFakeDirectory(dev, YAFFS_OBJECTID_LOSTNFOUND, YAFFS_LOSTNFOUND_MODE | S_IFDIR);
	
	if(dev->lostNFoundDir && dev->rootDir && dev->unlinkedDir && dev->deletedDir)
	{
		yaffs_AddObjectToDirectory(dev->rootDir, dev->lostNFoundDir);
		return YAFFS_OK;
	}
	
	return YAFFS_FAIL;
}

void yaffs_Deinitialise(yaffs_Device * dev);
int yaffs_GutsInitialise(yaffs_Device * dev)
{
	int init_failed = 0;
	unsigned x;
	int bits;

	/* Check stuff that must be set */
	if (!dev) 
	{
		printf("yaffs_GutsInitialise:need a device!\n");
		return YAFFS_FAIL;
	}

	dev->internalStartBlock = dev->startBlock;
	dev->internalEndBlock = dev->endBlock;
	dev->blockOffset = 0;
	dev->chunkOffset = 0;
	dev->nFreeChunks = 0;

	if (dev->startBlock == 0) 
	{
		dev->internalStartBlock = dev->startBlock + 1;
		dev->internalEndBlock = dev->endBlock + 1;
		dev->blockOffset = 1;
		dev->chunkOffset = dev->nChunksPerBlock;
	}

	/* Check geometry parameters. */
	if ((!dev->inbandTags && dev->isYaffs2 && dev->totalBytesPerChunk < 1024) || 
	    (!dev->isYaffs2 && dev->totalBytesPerChunk != 512) || 
	    (dev->inbandTags && !dev->isYaffs2 ) ||
	     dev->nChunksPerBlock < 2 || 
	     dev->nReservedBlocks < 2 || 
	     dev->internalStartBlock <= 0 || 
	     dev->internalEndBlock <= 0 || 
	     dev->internalEndBlock <= (dev->internalStartBlock + dev->nReservedBlocks + 2)	// otherwise it is too small
	    )
	{

	printf("inbandTags %x,yffs2 %x,bpc %x,cpb %x,rb %x,isb %x,ieb %x,and %x\n ",dev->inbandTags, dev->isYaffs2,dev->totalBytesPerChunk, dev->nChunksPerBlock,dev->nReservedBlocks,dev->internalStartBlock,dev->internalEndBlock,dev->internalStartBlock + dev->nReservedBlocks + 2);
	    	printf("some device data may error!\n");
		return YAFFS_FAIL;
	}
	
	dev->nDataBytesPerChunk = dev->totalBytesPerChunk;

	/* OK now calculate a few things for the device */	
	/*
	 *  Calculate all the chunk size manipulation numbers: 	 
	 */
 	 {
		__u32 x = dev->nDataBytesPerChunk;
		 /* We always use dev->chunkShift and dev->chunkDiv */
		 dev->chunkShift = Shifts(x);
		 x >>= dev->chunkShift;
		 dev->chunkDiv = x;
		 /* We only use chunk mask if chunkDiv is 1 */
		 dev->chunkMask = (1<<dev->chunkShift) - 1;
	}
	 

	/*
	 * Calculate chunkGroupBits.
	 * We need to find the next power of 2 > than internalEndBlock
	 */

	x = dev->nChunksPerBlock * (dev->internalEndBlock + 1);
	
	bits = ShiftsGE(x);
	
	/* Set up tnode width if wide tnodes are enabled. */
	if(!dev->wideTnodesDisabled)
	{
		/* bits must be even so that we end up with 32-bit words */
		if(bits & 1)
			bits++;
		if(bits < 16)
			dev->tnodeWidth = 16;
		else
			dev->tnodeWidth = bits;
	}
	else
		dev->tnodeWidth = 16;
 
	dev->tnodeMask = (1<<dev->tnodeWidth)-1;
		
	/* Level0 Tnodes are 16 bits or wider (if wide tnodes are enabled),
	 * so if the bitwidth of the
	 * chunk range we're using is greater than 16 we need
	 * to figure out chunk shift and chunkGroupSize
	 */
		 
	if (bits <= dev->tnodeWidth)
		dev->chunkGroupBits = 0;
	else
		dev->chunkGroupBits = bits - dev->tnodeWidth;	

	dev->chunkGroupSize = 1 << dev->chunkGroupBits;

	if (dev->nChunksPerBlock < dev->chunkGroupSize)
	{
		/* We have a problem because the soft delete won't work if
		 * the chunk group size > chunks per block.
		 * This can be remedied by using larger "virtual blocks".
		 */
		printf("yaffs: chunk group too large\n");
		return YAFFS_FAIL;
	}

	/* OK, we've finished verifying the device, lets continue with initialisation */
	/* More device initialisation */
	dev->garbageCollections = 0;
	dev->passiveGarbageCollections = 0;
	dev->currentDirtyChecker = 0;
	dev->bufferedBlock = -1;
	dev->doingBufferedBlockRewrite = 0;
	dev->nDeletedFiles = 0;
	dev->nBackgroundDeletions = 0;
	dev->nUnlinkedFiles = 0;
	dev->eccFixed = 0;
	dev->eccUnfixed = 0;
	dev->tagsEccFixed = 0;
	dev->tagsEccUnfixed = 0;
	dev->nErasureFailures = 0;
	dev->nErasedBlocks = 0;
	dev->isDoingGC = 0;
	dev->hasPendingPrioritisedGCs = 1; /* Assume the worst for now, will get fixed on first GC */

	/* Initialise temporary buffers and caches. */
	if(!yaffs_InitialiseTempBuffers(dev))
		init_failed = 1;
	
	dev->gcCleanupList = NULL;
	dev->cacheHits = 0;
	
	if(!init_failed)
	{
		dev->gcCleanupList = malloc(dev->nChunksPerBlock * sizeof(__u32));
		if(!dev->gcCleanupList)
			init_failed = 1;
	}

	if (dev->isYaffs2)
		dev->useHeaderFileSize = 1;
	if(!init_failed && !yaffs_InitialiseBlocks(dev))
		init_failed = 1;
		
	yaffs_InitialiseTnodes(dev);
	yaffs_InitialiseObjects(dev);

	if(!init_failed && !yaffs_CreateInitialDirectories(dev))
		init_failed = 1;


	if(!init_failed)
	{
		/* Now scan the flash. */
		if (dev->isYaffs2) 
		{
			printf("Read check point data......\n");
			if(yaffs_CheckpointRestore(dev))
			{
				yaffs_CheckObjectDetailsLoaded(dev->rootDir);
				printf("Read check point data OK!\n");
			} 
			else 
			{

				/* Clean up the mess caused by an aborted checkpoint load 
				 * and scan backwards. 
				 */
				printf("Read check point data Failed!\nNow scaning NAND Flash......\n");
				yaffs_DeinitialiseBlocks(dev);
				yaffs_DeinitialiseTnodes(dev);
				yaffs_DeinitialiseObjects(dev);
				
				dev->nErasedBlocks = 0;
				dev->nFreeChunks = 0;
				dev->allocationBlock = -1;
				dev->allocationPage = -1;
				dev->nDeletedFiles = 0;
				dev->nUnlinkedFiles = 0;
				dev->nBackgroundDeletions = 0;
				dev->oldestDirtySequence = 0;

				if(!init_failed && !yaffs_InitialiseBlocks(dev))
					init_failed = 1;
					
				yaffs_InitialiseTnodes(dev);
				yaffs_InitialiseObjects(dev);

				if(!init_failed && !yaffs_CreateInitialDirectories(dev))
					init_failed = 1;

				if(!init_failed && !yaffs_ScanBackwards(dev))
					init_failed = 1;
			}
		}
		else
			//if yaffs1
			init_failed = 1;
	}
		
	if(init_failed)
	{
		/* Clean up the mess */
		yaffs_Deinitialise(dev);
		return YAFFS_FAIL;
	}

	/* Zero out stats */
	dev->nPageReads = 0;
	dev->nPageWrites = 0;
	dev->nBlockErasures = 0;
	dev->nGCCopies = 0;
	dev->nRetriedWrites = 0;

	dev->nRetiredBlocks = 0;

	//yaffs_VerifyFreeChunks(dev);
	//yaffs_VerifyBlocks(dev);

	return YAFFS_OK;

}

void yaffs_Deinitialise(yaffs_Device * dev)
{
	if (dev->isMounted) 
	{
		int i;

		yaffs_DeinitialiseBlocks(dev);
		yaffs_DeinitialiseTnodes(dev);
		yaffs_DeinitialiseObjects(dev);
		/*if (dev->nShortOpCaches > 0 && dev->srCache) 
		{

			for (i = 0; i < dev->nShortOpCaches; i++) 
			{
				if(dev->srCache[i].data)
					free(dev->srCache[i].data);
				
				dev->srCache[i].data = NULL;
			}

			free(dev->srCache);
			dev->srCache = NULL;
		}
		*/
		free(dev->gcCleanupList);

		for (i = 0; i < YAFFS_N_TEMP_BUFFERS; i++)
			free(dev->tempBuffer[i].buffer);
		
		dev->isMounted = 0;
	}
}

static int yaffs_FindNiceObjectBucket(yaffs_Device * dev)
{
	static int x = 0;
	int i;
	int l = 999;
	int lowest = 999999;

	/* First let's see if we can find one that's empty. */
	for (i = 0; i < 10 && lowest > 0; i++) 
	{
		x++;
		x %= YAFFS_NOBJECT_BUCKETS;
		if (dev->objectBucket[x].count < lowest) 
		{
			lowest = dev->objectBucket[x].count;
			l = x;
		}

	}

	/* If we didn't find an empty list, then try
	 * looking a bit further for a short one
	 */

	for (i = 0; i < 10 && lowest > 3; i++) 
	{
		x++;
		x %= YAFFS_NOBJECT_BUCKETS;
		if (dev->objectBucket[x].count < lowest) 
		{
			lowest = dev->objectBucket[x].count;
			l = x;
		}

	}

	return l;
}

int yaffsfs_Match(char a, char b)
{
	// case sensitive
	return (a == b);
}

int yaffsfs_IsPathDivider(char ch)
{
	char *str = "/";

	while(*str){
		if(*str == ch)
			return 1;
		str++;
	}

	return 0;
}


// yaffsfs_FindDirectory
// Parse a path to determine the directory and the name within the directory.
//
// eg. "/data/xx/ff" --> puts name="ff" and returns the directory "/data/xx"
static yaffs_Object *yaffsfs_DoFindDirectory(yaffs_Object *startDir, const char *path, char **name, int symDepth)
{
	yaffs_Object *dir;
	char *restOfPath;
	char str[YAFFS_MAX_NAME_LENGTH+1];
	int i;

	if(startDir)
	{
		dir = startDir;
		restOfPath = (char *)path;
	}
	else
	{
		//dir = yaffsfs_FindRoot(path,&restOfPath);
		dir=yaffs_FindObjectByNumber(&gdev,YAFFS_OBJECTID_ROOT);
		restOfPath = path+1;
	}

	while(dir)
	{
		// parse off /.
		// curve ball: also throw away surplus '/'
		// eg. "/ram/x////ff" gets treated the same as "/ram/x/ff"
		while(yaffsfs_IsPathDivider(*restOfPath)) restOfPath++; // get rid of '/'

		*name = restOfPath;
		i = 0;
		
		while(*restOfPath && !yaffsfs_IsPathDivider(*restOfPath))
		{
			if (i < YAFFS_MAX_NAME_LENGTH)
			{
				str[i] = *restOfPath;
				str[i+1] = '\0';
				i++;
			}
			restOfPath++;
		}

		if(!*restOfPath)
			// got to the end of the string
			return dir;
		else
		{
			if(strcmp(str,".") == 0); 	//do nothing
			else if(strcmp(str,"..") == 0)
				dir = dir->parent;
			else
			{
				dir = yaffs_FindObjectByName(dir,str);

				if(dir && dir->variantType != YAFFS_OBJECT_TYPE_DIRECTORY)
					dir = NULL;
			}
		}
	}
	// directory did not exist.
	return NULL;
}

static yaffs_Object *yaffsfs_FindDirectory(yaffs_Object *relativeDirectory,const char *path,char **name,int symDepth)
{
	return yaffsfs_DoFindDirectory(relativeDirectory,path,name,symDepth);
}

// yaffsfs_FindObject turns a path for an existing object into the object
//
static yaffs_Object *yaffsfs_FindObject(yaffs_Object *relativeDirectory, const char *path,int symDepth)
{
	yaffs_Object *dir;
	yaffs_Object *l;
	char *name;
	char buffer[YAFFS_MAX_NAME_LENGTH+1];
	struct ylist_head *i;

	dir = yaffsfs_FindDirectory(relativeDirectory,path,&name,symDepth);

	if(dir && *name)
		return yaffs_FindObjectByName(dir,name);
	else if(dir)
	{
		ylist_for_each(i, &dir->variant.directoryVariant.children) 
		{
                	if (i)
			{
                        	l = ylist_entry(i, yaffs_Object, siblings);
                        
                      		yaffs_CheckObjectDetailsLoaded(l);
			
				yaffs_GetObjectName(l, buffer, YAFFS_MAX_NAME_LENGTH);
				if( l->variantType == YAFFS_OBJECT_TYPE_DIRECTORY )
					printf("%s/  ",buffer);
				else 
					printf("%s  ",buffer);
			}
		}
		printf("\n");
	}
	return dir;
}



int yaffs_read(int fd, void *buf, unsigned int nbyte)
{
	yaffsfs_Handle *h = NULL;
	yaffs_Object *obj = NULL;
	int pos = 0;
	int nRead = -1;
	unsigned int maxRead;
	int length;

	if(h = (struct yaffsfs_Handle *)_file[fd].data)
		obj = h->obj;
	else
	{
		printf("yaffs_read:error fd:%d!\n",fd);
		return -1;
	}	

	if(!h || !obj)
		return -1;
	else
	{
		length = yaffs_GetObjectFileLength(obj);
		pos =  h->position;
		if(yaffs_GetObjectFileLength(obj) > pos)
			maxRead = yaffs_GetObjectFileLength(obj) - pos;
		else
			maxRead = 0;

		if(nbyte > maxRead) 
			nbyte = maxRead;

		if(nbyte > 0)
		{
			nRead = yaffs_ReadDataFromFile(obj,buf,pos,nbyte);
			if(nRead >= 0)
				h->position = pos + nRead;
			else
				printf("read file byte:%d\n",nRead);
				
		}
		else
			nRead = 0;

	}
	_file[fd].posn += nRead;
	return (nRead >= 0) ? nRead : -1;
}


//######################################## mtdnand function #############################################
int nandmtd2_WriteChunkWithTagsToNAND(yaffs_Device *dev, int chunkInNAND, __u8 *buffer, yaffs_ExtendedTags *tags)
{
	return YAFFS_OK;
}

int nandmtd2_MarkNANDBlockBad(yaffs_Device *dev, int blockNo)
{
	return YAFFS_OK;
}

int nandmtd2_ReadChunkWithTagsFromNAND(yaffs_Device *dev, int chunkInNAND, __u8 *data, yaffs_ExtendedTags *tags)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	struct mtd_oob_ops ops;
	size_t dummy;
	int retval = 0;
	loff_t addr = ((loff_t) chunkInNAND) * dev->nDataBytesPerChunk;
	yaffs_PackedTags2 pt;

	if(data)
		retval = mtd->read(mtd, addr, dev->totalBytesPerChunk, &dummy, data);		
		
	if (tags)
	{
		ops.mode = MTD_OOB_AUTO;
		ops.ooblen = sizeof(pt);
		ops.len = data ? dev->nDataBytesPerChunk : sizeof(pt);
		ops.ooboffs = 0;
		ops.datbuf = data;
		ops.oobbuf = dev->spareBuffer;
		retval = mtd->read_oob(mtd, addr, &ops);
	}
	
	if (tags)
	{
		memcpy(&pt, dev->spareBuffer, sizeof(pt));
		yaffs_UnpackTags2(tags, &pt);
	}

	if(tags && retval == -EBADMSG && tags->eccResult == YAFFS_ECC_RESULT_NO_ERROR)
		tags->eccResult = YAFFS_ECC_RESULT_UNFIXED;
	
	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int nandmtd2_QueryNANDBlock(yaffs_Device *dev, int blockNo, yaffs_BlockState *state, __u32 *sequenceNumber)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	int retval;

	retval = mtd->block_isbad(mtd, blockNo * dev->nChunksPerBlock * dev->nDataBytesPerChunk);
     	if (retval)
	{
		*state = YAFFS_BLOCK_STATE_DEAD;
		*sequenceNumber = 0;
	} 
	else
	{
		yaffs_ExtendedTags t;
		nandmtd2_ReadChunkWithTagsFromNAND(dev, blockNo * dev->nChunksPerBlock, NULL, &t);
   		if (t.chunkUsed)
		{
			*sequenceNumber = t.sequenceNumber;
			*state = YAFFS_BLOCK_STATE_NEEDS_SCANNING;
		}
		else
		{	
			*sequenceNumber = 0;
			*state = YAFFS_BLOCK_STATE_EMPTY;
		}
	}

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int nandmtd2_EraseBlockInNAND(yaffs_Device *dev, int blockNumber)
{
	return YAFFS_OK;
}

int nandmtd2_InitialiseNAND(yaffs_Device *dev)
{
	return YAFFS_OK;
}


int yaffs_initialise()
{
	int ok;

	
	yaffsfs_InitHandles();
		
	ok = yaffs_GutsInitialise(&gdev);		
	if(!ok)
		printf("GutsInitialise failed!\n");
	return ok?0:-1;
}

int yaffs_write(int fd, const void *buf, unsigned int nbyte){}
int yaffs_lseek(int fd, off_t offset, int where)
{
	yaffsfs_Handle *h = NULL;
	yaffs_Object *obj = NULL;
	int pos = -1;
	int fSize = -1;

	h = (struct yaffsfs_Handle *)_file[fd].data;
	obj = h->obj;

	if(!h || !obj)
	{
		// bad handle
		return -1;
	}
	else if(where == SEEK_SET)
	{
		if(offset >= 0)
		{
			pos = offset;
		}
	}
	else if(where == SEEK_CUR)
	{
		if( (h->position + offset) >= 0)
		{
			pos = (h->position + offset);
		}
	}
	else if(where == SEEK_END)
	{
		fSize = yaffs_GetObjectFileLength(obj);
		if(fSize >= 0 && (fSize + offset) >= 0)
		{
			pos = fSize + offset;
		}
	}

	if(pos >= 0)
	{
		h->position = pos;
	}

	return pos;
}

int yaffs_open(int fd, const char *path, int oflag, int mode)
{
	yaffs_Object *obj = NULL;
	int handle = -1;
	yaffsfs_Handle *h = NULL;
	int i;
	int fdmtd;
	int nBlocks;
	char mtdpath[200];
	struct mtdpriv *priv;
	sprintf(mtdpath,"/dev/%s\n",path+11);
	fdmtd=open(mtdpath,O_RDWR);
	priv=(void *)_file[fdmtd].data;

	{
	memset(&gdev,0,sizeof(yaffs_Device));

	//Now initialise soc' Nand Flash: mtd,nand_chip
	
	nBlocks = priv->open_size/priv->file->mtd->erasesize;
	gdev.genericDevice = priv->file->mtd;
	gdev.startBlock = (priv->open_offset+priv->file->part_offset)/priv->file->mtd->erasesize;
	gdev.endBlock = gdev.startBlock+nBlocks - 1;
	gdev.nChunksPerBlock = priv->file->mtd->erasesize/priv->file->mtd->writesize;
	gdev.totalBytesPerChunk = priv->file->mtd->writesize;
	gdev.nReservedBlocks = 5;
	gdev.nShortOpCaches = 0;
	gdev.name = priv->file->mtd->name;
	
	gdev.writeChunkWithTagsToNAND = nandmtd2_WriteChunkWithTagsToNAND;
	gdev.readChunkWithTagsFromNAND = nandmtd2_ReadChunkWithTagsFromNAND;
	gdev.markNANDBlockBad = nandmtd2_MarkNANDBlockBad;
	gdev.queryNANDBlock = nandmtd2_QueryNANDBlock;
	gdev.spareBuffer = malloc(priv->file->mtd->oobsize);
	gdev.isYaffs2 = 1;

	gdev.eraseBlockInNAND = nandmtd2_EraseBlockInNAND;
	gdev.initialiseNAND = nandmtd2_InitialiseNAND;
	
	//use checkpoint to mount yaffs
	gdev.skipCheckpointRead = 0;
	gdev.skipCheckpointWrite = 0;
	}
	close(fdmtd);



	path += 11;

	//get rid of /dev/yaffs@mtdblock0/vmlinx to /vmlinux 
	while(*path && *path!='/') path++;
	if(!*path) 
	{
		printf("error path!\n");
		return -1;
	}

	//if(gdev.endBlock <= 0)
	{
		if(yaffs_initialise()==-1)
		{
			printf("Yaffs2 initialise failed!\n");
			return -1;
		}
	}
	
	handle = yaffsfs_GetHandle();
	if(handle >= 0)
	{
		h = yaffsfs_GetHandlePointer(handle);

		// try to find the exisiting object
		obj = yaffsfs_FindObject(NULL,path,0);
		if(obj && obj->variantType != YAFFS_OBJECT_TYPE_FILE)
			obj = NULL;
		if(obj)
		{
			h->obj = obj;
			h->inUse = 1;
	    		h->readOnly = (oflag & (O_WRONLY | O_RDWR)) ? 0 : 1;
			h->append =  (oflag & O_APPEND) ? 1 : 0;
			h->exclusive = (oflag & O_EXCL) ? 1 : 0;
			h->position = 0;

                        obj->inUse++;
		}
		else
		{
			yaffsfs_PutHandle(handle);
			handle = -1;
			return 0;
		}
	}
	
	if(handle >= 0)
	{
		_file[fd].valid = 1;
		_file[fd].data = (void *)h;
	}
	return fd;
}



int yaffs_close(int fd)
{
	//yaffsfs_PutHandle(fd);
	yaffsfs_Handle *h;

	h = (struct yaffs_Handle *)_file[fd].data;
	if(h)
	{
		h->inUse=0;
		h->obj=NULL;
	}
	return 0;
}

