
struct directory_entry {
	unsigned long inode;		/* inode number or 0 (unused) */
	short rec_len;			/* offset to the next dir. entry */
	unsigned char name_len;		/* name length */
	unsigned char type;
	char name;
} __attribute__ ((packed));


#define DT_UNKNOWN     0
#define DT_FIFO        1
#define DT_CHR         2
#define DT_DIR         4	/* directory */
#define DT_BLK         6
#define DT_REG         8	/* regular file */
#define DT_LNK         10
#define DT_SOCK        12
#define DT_WHT         14

