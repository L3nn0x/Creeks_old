#include "types.h"
#include "gdt.h"
#include "process.h"
#include "lib.h"
#include "mm.h"
#include "signal.h"

/* 
 * switch_to_task(): Prepare la commutation de tache effectuee par do_switch().
 * Le premier parametre indique le pid du processus a charger.
 * Le mode indique si ce processus etait en mode utilisateur ou en mode kernel
 * quand il a ete precedement interrompu par le scheduler.
 * L'empilement des registres sur la pile change selon le cas.
 */
void switch_to_task(int n, int mode)
{
	u32 kesp, eflags;
	u16 kss, ss, cs;
	int sig;

	/* Commence le changement de processus */
	current = &p_list[n];

	/* Traite les signaux */
	if ((sig = dequeue_signal(current->signal))) 
		handle_signal(sig);

	/* Charge le TSS du nouveau processus */
	default_tss.ss0 = current->kstack.ss0;
	default_tss.esp0 = current->kstack.esp0;

	/* 
	 * Empile les registres ss, esp, eflags, cs et eip necessaires a la
	 * commutation. Ensuite, la fonction do_switch() restaure les
	 * registres, la table de page du nouveau processus courant et commute
	 * avec l'instruction iret.
	 */
	ss = current->regs.ss;
	cs = current->regs.cs;
	eflags = (current->regs.eflags | 0x200) & 0xFFFFBFFF;

	/* Prepare le changement de pile noyau */
	if (mode == USERMODE) {
		kss = current->kstack.ss0;
		kesp = current->kstack.esp0;
	} else {			/* KERNELMODE */
		kss = current->regs.ss;
		kesp = current->regs.esp;
	}

	asm("	mov %0, %%ss; \
		mov %1, %%esp; \
		cmp %[KMODE], %[mode]; \
		je next; \
		push %2; \
		push %3; \
		next: \
		push %4; \
		push %5; \
		push %6; \
		push %7; \
		ljmp $0x08, $do_switch" 
		:: \
		"m"(kss), \
		"m"(kesp), \
		"m"(ss), \
		"m"(current->regs.esp), \
		"m"(eflags), \
		"m"(cs), \
		"m"(current->regs.eip), \
		"m"(current), \
		[KMODE] "i"(KERNELMODE), \
		[mode] "g"(mode)
	    );
}

void schedule(void)
{
	struct process *p;
	u32 *stack_ptr;
	int i, newpid;

	/* Stocke dans stack_ptr le pointeur vers les registres sauvegardes */
	asm("mov (%%ebp), %%eax; mov %%eax, %0": "=m"(stack_ptr):);

	/* Si il n'y a aucun processus, on retourne directement */
	if (!n_proc) {
		return;
	}

	/* Si il y a un seul processus, on le laisse tourner */
	else if (n_proc == 1 && current->pid != 0) {
		return;
	}

	/* Si il y a au moins deux processus, on commute vers le suivant */
	else {
		/* Sauver les registres du processus courant */
		current->regs.eflags = stack_ptr[16];
		current->regs.cs = stack_ptr[15];
		current->regs.eip = stack_ptr[14];
		current->regs.eax = stack_ptr[13];
		current->regs.ecx = stack_ptr[12];
		current->regs.edx = stack_ptr[11];
		current->regs.ebx = stack_ptr[10];
		current->regs.ebp = stack_ptr[8];
		current->regs.esi = stack_ptr[7];
		current->regs.edi = stack_ptr[6];
		current->regs.ds = stack_ptr[5];
		current->regs.es = stack_ptr[4];
		current->regs.fs = stack_ptr[3];
		current->regs.gs = stack_ptr[2];

		/* 
		 * Sauvegarde le contenu des registres de pile (ss, esp)
		 * au moment de l'interruption. Necessaire car le processeur
		 * empile ou non ces valeurs selon le contexte de l'interruption.
		 */
		if (current->regs.cs != 0x08) {	/* mode utilisateur */
			current->regs.esp = stack_ptr[17];
			current->regs.ss = stack_ptr[18];
		} else {	/* pendant un appel systeme */
			current->regs.esp = stack_ptr[9] + 12;	/* vaut : &stack_ptr[17] */
			current->regs.ss = default_tss.ss0;
		}

		/* Sauver le TSS de l'ancien processus */
		current->kstack.ss0 = default_tss.ss0;
		current->kstack.esp0 = default_tss.esp0;
	}

	/* Choix du nouveau processus (un simple roundrobin) */
	newpid = 0;
	for (i = current->pid + 1; i < MAXPID && newpid == 0; i++) {
		if (p_list[i].state == 1)
			newpid = i;
	}

	if (!newpid) {
		for (i = 1; i < current->pid && newpid == 0; i++) {
			if (p_list[i].state == 1)
				newpid = i;
		}
	}

	p = &p_list[newpid];

	//// printk("DEBUG: prepare to switch to process: %d\n", p->pid); /*DEBUG*/
	//// printk("DEBUG: ss: %p, esp: %p, eip: %p\n", p->regs.ss, p->regs.esp, p->regs.eip); /*DEBUG*/

	/* Commutation */
	if (p->regs.cs != 0x08)
		switch_to_task(p->pid, USERMODE);
	else
		switch_to_task(p->pid, KERNELMODE);


}
