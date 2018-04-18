#include <linux/mtd/mtd.h>
#include <sys/queue.h>


#define MTD_FLAGS_CHAR      0x1 /*main+oob,all data */
#define MTD_FLAGS_BLOCK     0x2 /*main;but no OOB */
#define MTD_FLAGS_RAW       0x4 /*raw mtd,no check bad part*/
#define MTD_FLAGS_CHAR_MARK 0x8 /*main+oob,but bad_mark auto*/
#define MTD_FLAGS_GOOD      0x10 /*use good part info*/

typedef struct mtdfile {
	struct mtd_info *mtd;
	int refs;
	int fd;
	int flags;
	int index;
	unsigned long long part_size;
	unsigned long long part_offset;
#define MTDFILE_STATIC  0x0000
#define MTDFILE_DYNAMIC 0x0001
	LIST_ENTRY(mtdfile)	i_next;
        void *trans_table;
        unsigned long long part_size_real;
	char name[1];
} mtdfile;

typedef struct mtdpriv {
struct mtdfile *file;
	unsigned long long open_offset;
	unsigned long long open_size;
	unsigned long long open_size_real;
        int flags;
} mtdpriv;

