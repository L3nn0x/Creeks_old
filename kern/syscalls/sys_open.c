#include "types.h"
#include "list.h"
#include "lib.h"
#include "io.h"
#include "process.h"
#include "kmalloc.h"
#include "mm.h"
#include "schedule.h"
#include "file.h"
#include "console.h"
#include "signal.h"
#include "syscalls.h"


int sys_open(char *path)
{
	u32 fd;
	struct file *fp;
	struct open_file *of;

	if (!(fp = path_to_file(path))) {
		//// printk("DEBUG: sys_open(): can't open %s\n", path);
		return -1;
	}

	//// printk("DEBUG: sys_open(): process[%d] opening file %s\n", current->pid, fp->name);
	
	fp->opened++;

	if (!fp->inode)
		fp->inode = ext2_read_inode(fp->disk, fp->inum);

	/* Lecture du fichier */
	fp->mmap = ext2_read_file(fp->disk, fp->inode);

	/* 
	 * Recherche d'un descripteur libre.
	 */
	fd = 0;
	if (current->fd == 0) {
		current->fd = (struct open_file *) kmalloc(sizeof(struct open_file));
		current->fd->file = fp;
		current->fd->ptr = 0;
		current->fd->next = 0;
	} else {
		of = current->fd;
		while (of->file && of->next) {
			of = of->next;
			fd++;
		}

		if (of->file == 0) {	/* reutilisation d'un descripteur precedement ouvert */
			of->file = fp;
			of->ptr = 0;
		} else {		/* nouveau descripteur */
			of->next = (struct open_file *) kmalloc(sizeof(struct open_file));
			of->next->file = fp;
			of->next->ptr = 0;
			of->next->next = 0;
			fd++;
		}
	}

	return fd;
}
