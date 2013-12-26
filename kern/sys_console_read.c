#include "types.h"
#include "lib.h"
#include "process.h"
#include "console.h"

int sys_console_read(char *u_buf)
{
		if (!current->console->term) {
			printk("DEBUG: sys_console_read(): process without term\n");
			return -1;
		}

		/* Bloque si la console est deja utilisee */
		while (current_term->pread);

		current->console->term->pread = current;
		current->console->inlock = 1;

		/* Bloque jusqu'a ce que le buffer utilisateur soit rempli */
		while (current->console->inlock == 1);

		strcpy(u_buf, current->console->inb);

		return strlen(u_buf);
}

