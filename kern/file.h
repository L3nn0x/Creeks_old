#include "ext2.h"

struct file {
	struct disk *disk;
	u32 inum;		/* inode number */
	char *name;		/* file name */
	struct ext2_inode *inode;
	char *mmap;		/* buffer (if opened) */
	int opened;		/* number of process that have opened the file */

	struct file *parent;	/* parent directory */
	struct list_head leaf;	/* child files linked list */
	struct list_head sibling;	/* siblings */
};

struct open_file {
	struct file *file;	/* descripteur de fichier */
	u32 ptr;		/* pointeur de lecture dans le fichier */
	struct open_file *next;
};

struct file *f_root;		/* root file : "/" */
struct disk *hd;//disque

#define REP 0
#define FILE 1

struct file *init_root(struct disk *);
int is_directory(struct file *);
struct file *is_cached_leaf(struct file *, char *);
int get_dir_entries(struct file *);
struct file *path_to_file(char *);
void add_file(struct disk *,char*,int);
