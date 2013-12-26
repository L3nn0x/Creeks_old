#include "types.h"
#include "list.h"
#include "ext2.h"
#include "console.h"

#define KERNELMODE	0
#define USERMODE	1

#define MAXPID	32

#ifndef __PROCESS_STRUCT__
#define __PROCESS_STRUCT__
struct process {
	int pid;

	struct {
		u32 eax, ecx, edx, ebx;
		u32 esp, ebp, esi, edi;
		u32 eip, eflags;
		u32 cs:16, ss:16, ds:16, es:16, fs:16, gs:16;
		u32 cr3;
	} regs __attribute__ ((packed));

	struct {
		u32 esp0;
		u16 ss0;
	} kstack __attribute__ ((packed));

	/* 
	 * ATTENTION ! On ne peut ajouter des elements au dessus de cette
	 * limite sans modifier la fonction assembleur do_switch() qui effectue
	 * la commutation de tache.
	 */

	/* NOTE: redondance entre regs.cr3 et pd->base->p_addr */
	struct page_directory *pd;	

	struct list_head pglist;	/* Pages utilisees par le processus (exec, data, stack) */

	char *b_exec;
	char *e_exec;
	char *b_bss;
	char *e_bss;
	char *b_heap;
	char *e_heap;			/* Pointe sur le sommet du "heap" */

	struct file *pwd;		/* Pointe sur le repertoire courant */
	struct open_file *fd;		/* Liste chainee des fichiers ouverts */

	struct process *parent;		/* Processus parent */
	struct list_head child; 	/* Processus enfants */
	struct list_head sibling; 	/* Processus conjoints */

	struct vconsole *console;	/* Console */

	u32 signal;
	void* sigfn[32];

	int status;	/* exit status */

	int state;	/* -1 zombie, 0 not runnable, 1 ready/running, 2 sleep */

} __attribute__ ((packed));
#endif


#ifdef __PLIST__

struct process p_list[MAXPID + 1];
struct process *current = 0;
int n_proc = 0;

#else

extern struct process p_list[];
extern struct process *current;
extern int n_proc;

#endif

int load_task(struct disk *, struct ext2_inode *, int, char **);

