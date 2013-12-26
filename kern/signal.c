#include "types.h"
#include "lib.h"
#include "process.h"
#include "signal.h"
#include "syscalls.h"

int dequeue_signal(int mask) 
{
	int sig;

	if (mask) {
		sig = 1;
		while (!(mask & 1)) {
			mask = mask >> 1;
			sig++;
		}
	}
	else
		sig = 0;

	return sig;
}

int handle_signal(int sig)
{
	u32 *esp;

	//printk("DEBUG: handle_signal(): signal %d for process %d\n", sig, current->pid);

	if (current->sigfn[sig] == (void*) SIG_IGN) {
		clear_signal(&current->signal, sig);
	}
	else if (current->sigfn[sig] == (void*) SIG_DFL) {
		switch(sig) {
			case SIGHUP : case SIGINT : case SIGQUIT : 
				asm("mov %0, %%eax; mov %%eax, %%cr3"::"m"(current->regs.cr3));
				sys_exit(1);
				break;
			case SIGCHLD : 
				break;
			default :
				clear_signal(&current->signal, sig);
		}
	}
	else {
		/* 
		 * Sauvegarde des registres sur la pile utilisateur 
		 */
		esp = (u32*) current->regs.esp - 20;

		asm("mov %0, %%eax; mov %%eax, %%cr3"::"m"(current->regs.cr3));

		/* Code assembleur qui appelle sys_sigreturn() */
		esp[19] = 0x0030CD00;
		esp[18] = 0x00000EB8;

		/* Sauvegarde des registres */
		esp[17] = current->kstack.esp0;
		esp[16] = current->regs.ss;
		esp[15] = current->regs.esp;
		esp[14] = current->regs.eflags;
		esp[13] = current->regs.cs;
		esp[12] = current->regs.eip;
		esp[11] = current->regs.eax;
		esp[10] = current->regs.ecx;
		esp[9] = current->regs.edx;
		esp[8] = current->regs.ebx;
		esp[7] = current->regs.ebp;
		esp[6] = current->regs.esi;
		esp[5] = current->regs.edi;
		esp[4] = current->regs.ds;
		esp[3] = current->regs.es;
		esp[2] = current->regs.fs;
		esp[1] = current->regs.gs;

		/* Adresse de retour pour %eip */
		esp[0] = (u32) &esp[18];

		/* 
		 * Remplace les registres %esp et %eip pour executer la routine
		 * de service definie par l'utilisateur.
		 */
		current->regs.esp = (u32) esp;
		current->regs.eip = (u32) current->sigfn[sig];

		/* Efface le signal et retablit le handler par defaut */
		current->sigfn[sig] = (void*) SIG_DFL;
		if (sig != SIGCHLD)
			clear_signal(&current->signal, sig);
	}

	return 0;
}

