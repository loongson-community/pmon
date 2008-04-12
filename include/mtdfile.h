#include <linux/mtd/mtd.h>
#include <sys/queue.h>

typedef struct mtdfile {
	struct mtd_info *mtd;
	char name[26];
	int refs;
	int fd;
	int flags;
	int index;
	int part_size;
	int part_offset;
#define MTDFILE_STATIC  0x0000
#define MTDFILE_DYNAMIC 0x0001
	LIST_ENTRY(mtdfile)	i_next;
} mtdfile;

typedef struct mtdpriv {
struct mtdfile *file;
	int open_offset;
	int open_size;
} mtdpriv;

