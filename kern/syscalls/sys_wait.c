#include "list.h"
#include "lib.h"
#include "io.h"
#include "process.h"
#include "signal.h"


int sys_wait(int* status)
{
	int pid;
	struct list_head *p, *n;
	struct process *children;

	//// printk("DEBUG: sys_wait(): [%d] wait for children death\n", current->pid);

	while (0 == is_signal(current->signal, SIGCHLD))
		;

	//printk("DEBUG: sys_wait(): [%d] has a dead children\n", current->pid);

	cli;

	/* Recherche du fils mort */
	list_for_each_safe(p, n, &current->child) {
		children = list_entry(p, struct process, sibling);
		if (children->state == -1) {
			pid = children->pid;		/* recupere le pid */
			*status = children->status;	/* recupere le status */
			children->state = 0;		/* libere le fils */
			list_del(p);			/* enleve le fils de sa liste */
			clear_signal(&current->signal, SIGCHLD);	/* efface le signal SIGCHLD */
			break;
		}
	}

	sti;

	return pid;
}

