#include "screen.h"
#include "console.h"
#include "process.h"

/* 
 * Pour copier un caractere dans le buffer utilisateur, il faut etre dans le
 * contexte du processus en question ! 
 */
void putc_console(char c)
{
	struct process *p;

	/* En l'absence de console, on ecrit directement dans la memoire video */
	if (!current_term || !(p = current_term->pread) || p->state<1) {
		putcar(c);
		return;
	}

	if (p->console->mode == 0) {	/* Not buffered mode */
		putcar(c);
		p->console->inb[0] = c;
		p->console->term->pread = 0;
		p->console->inlock = 0;
	} else {			/* Buffered modes */
		if (c == 8) {		/* backspace */
			if (p->console->keypos) {
				p->console->inb[p->console->keypos--] = 0;
				if (p->console->mode == 1)
					putcar(c);
			}
		}
		else if (c == 10) {	/* newline */
			if (p->console->mode == 1)
				putcar(c);
			p->console->inb[p->console->keypos++] = c;
			p->console->inb[p->console->keypos] = 0; 
			p->console->term->pread = 0;
			p->console->inlock = 0;
			p->console->keypos = 0;
		}
		else {
			if (p->console->mode == 1)
				putcar(c);
			p->console->inb[p->console->keypos++] = c; 
		}

	} 
}
