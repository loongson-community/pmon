#include <linux/mtd/mtd.h>
#include <sys/queue.h>

typedef struct mtdfile {
	struct mtd_info *mtd;
	char name[26];
	int refs;
	int fd;
	int flags;
	LIST_ENTRY(mtdfile)	i_next;
} mtdfile;
