#include "types.h"
#include "io.h"
#include "lib.h"
#include "mm.h"
#include "kmalloc.h"
#include "ext2.h"
#include "elf.h"
#include "file.h"
#include "list.h"
#include "signal.h"

#define __PLIST__
#include "process.h"


/*
 * Charge une tache en memoire
 */
int load_task(struct disk *hd, struct ext2_inode *inode, int argc, char **argv)
{
	struct page *kstack;
	struct process *previous;
	struct vconsole *console;

	char **param, **uparam;
	char *file;
	u32 stackp;
	u32 e_entry; 

	int pid;
	int i;

	/* 
	 * Calcul du pid du nouveau processus. Assume qu'on n'atteindra jamais
	 * la limite imposee par la taille de p_list[]
	 * FIXME: reutiliser slots libres
	 */
	pid = 1;
	while (p_list[pid].state != 0 && pid++ < MAXPID);
	if (p_list[pid].state != 0) {
		printk("PANIC: not enough slot for processes\n");
		return 0;
	}

	n_proc++;
	p_list[pid].pid = pid;

	/* On copie les arguments a passer au programme */
	if (argc) {
		param = (char**) kmalloc(sizeof(char*) * (argc+1));
		for (i=0 ; i<argc ; i++) {
			param[i] = (char*) kmalloc(strlen(argv[i]) + 1);
			strcpy(param[i], argv[i]);
		}
		param[i] = 0;
	}

	/* Cree un repertoire de pages */
	p_list[pid].pd = pd_create();

	/* Initialise la liste des pages frame utilisees */
	INIT_LIST_HEAD(&p_list[pid].pglist);

	/* 
	 * On change d'espace d'adressage pour passer sur celui du nouveau
	 * processus.
	 * Il faut faire pointer 'current' sur le nouveau processus afin que
	 * les defauts de pages mettent correctement a jour les informations 
	 * de la struct process.
	 */
	previous = current;
	current = &p_list[pid];
	asm("mov %0, %%eax; mov %%eax, %%cr3"::"m"(p_list[pid].pd->base->p_addr));

	/* 
	 * Charge l'executable en memoire. En cas d'echec, on libere les
	 * ressources precedement allouees.
	 */
	file = ext2_read_file(hd, inode);
	e_entry = (u32) load_elf(file, &p_list[pid]);
	kfree(file);
	if (e_entry == 0) {	/* echec */
		for (i=0 ; i<argc ; i++) 
			kfree(param[i]);
		kfree(param);
		current = previous;
		asm("mov %0, %%eax ;mov %%eax, %%cr3"::"m" (current->regs.cr3));
		pd_destroy(p_list[pid].pd);
		return 0;
	}

	/* 
	 * Cree la pile utilisateur. La memoire est allouee dynamiquement a
	 * chaque defaut de page.
	 */
	stackp = USER_STACK - 16;

	/* Copie des parametres sur la pile utilisateur */
	if (argc) {
		uparam = (char**) kmalloc(sizeof(char*) * argc);

		for (i=0 ; i<argc ; i++) {
			stackp -= (strlen(param[i]) + 1);
			strcpy((char*) stackp, param[i]);
			uparam[i] = (char*) stackp;
		}

		stackp &= 0xFFFFFFF0;		/* alignement */

		/* Creation des arguments de main() : argc, argv[]... */
		stackp -= sizeof(char*);
		*((char**) stackp) = 0;

		for (i=argc-1 ; i>=0 ; i--) {		/* argv[0] a argv[n] */
			stackp -= sizeof(char*);
			*((char**) stackp) = uparam[i]; 
		}

		stackp -= sizeof(char*);	/* argv */
		*((char**) stackp) = (char*) (stackp + 4); 

		stackp -= sizeof(char*);	/* argc */
		*((int*) stackp) = argc; 

		stackp -= sizeof(char*);

		for (i=0 ; i<argc ; i++) 
			kfree(param[i]);

		kfree(param);
		kfree(uparam);
	}

	/* Cree la pile noyau */
	kstack = get_page_from_heap();

	/* On associe une console au processus */
	console = (struct vconsole *) kmalloc(sizeof(struct vconsole));
	console->term = current_term;
	console->inlock = 0;
	console->keypos = 0;
	console->mode = 1;	/* buffered mode */

	/* Initialise le reste des registres et des attributs */
	p_list[pid].regs.ss = 0x33;
	p_list[pid].regs.esp = stackp;
	p_list[pid].regs.eflags = 0x0;
	p_list[pid].regs.cs = 0x23;
	p_list[pid].regs.eip = e_entry;
	p_list[pid].regs.ds = 0x2B;
	p_list[pid].regs.es = 0x2B;
	p_list[pid].regs.fs = 0x2B;
	p_list[pid].regs.gs = 0x2B;
	p_list[pid].regs.cr3 = (u32) p_list[pid].pd->base->p_addr;

	p_list[pid].kstack.ss0 = 0x18;
	p_list[pid].kstack.esp0 = (u32) kstack->v_addr + PAGESIZE - 16;

	p_list[pid].regs.eax = 0;
	p_list[pid].regs.ecx = 0;
	p_list[pid].regs.edx = 0;
	p_list[pid].regs.ebx = 0;

	p_list[pid].regs.ebp = 0;
	p_list[pid].regs.esi = 0;
	p_list[pid].regs.edi = 0;

	/* p_list[pid].pd; */
	/* p_list[pid].pglist; */

	/* 
	 * Attributs deja initialises par load_elf():
	 *   p_list[pid].b_exec;
	 *   p_list[pid].e_exec;
	 *   p_list[pid].b_bss;
	 *   p_list[pid].e_bss;
	 */
	p_list[pid].b_heap = (char*) ((u32) p_list[pid].e_bss & 0xFFFFF000) + PAGESIZE;
	p_list[pid].e_heap = p_list[pid].b_heap;

	p_list[pid].pwd = previous->pwd;

	p_list[pid].fd = 0;	/* Aucun fichier ouvert */

	if (previous->state != 0) 
		p_list[pid].parent = previous;
	else 
		p_list[pid].parent = &p_list[0];

	INIT_LIST_HEAD(&p_list[pid].child);

	if (previous->state != 0) 
		list_add(&p_list[pid].sibling, &previous->child);
	else 
		list_add(&p_list[pid].sibling, &p_list[0].child);

	p_list[pid].console = console;

	p_list[pid].signal = 0;
	for(i=0 ; i<32 ; i++)
		p_list[pid].sigfn[i] = (char*) SIG_DFL;

	p_list[pid].status = 0;

	p_list[pid].state = 1;

	current = previous;
	asm("mov %0, %%eax ;mov %%eax, %%cr3":: "m"(current->regs.cr3));

	return pid;
}
