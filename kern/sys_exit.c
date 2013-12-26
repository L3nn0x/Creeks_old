#include "types.h"
#include "list.h"
#include "lib.h"
#include "io.h"
#include "process.h"
#include "kmalloc.h"
#include "mm.h"
#include "schedule.h"
#include "file.h"
#include "signal.h"

void sys_exit(int status) 
{
	u16 kss;
	u32 kesp;
	struct list_head *p, *n;
	struct page *pg;
	struct open_file *fd, *fdn;
	struct process *proc;

	//printk("DEBUG: sys_exit(): process[%d] exit\n", current->pid);     /* DEBUG */

	n_proc--;
	current->state = -1;
	current->status = status;

	/* 
	 * Liberation des ressources memoire allouees au processus :
	 *   - les pages utilisees par le code executable
	 *   - la pile utilisateur
	 *   - la pile noyau
	 *   - le repertoire et les tables de pages
	 */

	/* Libere la memoire occupee par le processus */
	list_for_each_safe(p, n, &current->pglist) {
		pg = list_entry(p, struct page, list);
		release_page_frame(pg->p_addr);
		list_del(p);
		kfree(pg);
	}

	/* Libere la liste des fichiers ouverts */
	fd = current->fd;
	while (fd) {
		/* Libere le fichier si plus aucun processus ne pointe dessus */
		fd->file->opened--;
		if (fd->file->opened == 0) {
			kfree(fd->file->mmap);
			fd->file->mmap = 0;
		}

		fdn = fd->next;
		kfree(fd);
		fd = fdn;
	}

	/* Met a jour la liste des processus du pere */
	if (current->parent->state > 0) 
		set_signal(&current->parent->signal, SIGCHLD);
	else
		printk("WARNING: sys_exit(): process %d without valid parent\n", current->pid);
	
	/* Donne un nouveau pere aux enfants */
	list_for_each_safe(p, n, &current->child) {
		proc = list_entry(p, struct process, sibling);
		proc->parent = &p_list[0];
		list_del(p);
		list_add(p, &p_list[0].child);
	}

	/* Libere la console */
	kfree(current->console);
	current->console = 0;

	/* Libere la pile noyau */
	kss = p_list[0].regs.ss;
	kesp = p_list[0].regs.esp;
	asm("mov %0, %%ss; mov %1, %%esp;"::"m"(kss), "m"(kesp));
	release_page_from_heap((char *) ((u32) current->kstack.esp0 & 0xFFFFF000));

	/* Libere le repertoire de pages et les tables associees */
	asm("mov %0, %%eax; mov %%eax, %%cr3"::"m"(pd0));
	pd_destroy(current->pd);

	switch_to_task(0, KERNELMODE);
}
