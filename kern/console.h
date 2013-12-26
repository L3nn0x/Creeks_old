
#define INBUFSIZE 4096

#ifndef __CONS__
#define __CONS__
struct terminal {
	struct process *pread;	/* pointe sur le processus qui lit a partir de la console */
	struct process *pwrite;	/* pointe sur le processus qui ecrit sur la console */
};

struct vconsole {
	struct terminal *term;	/* terminal */
	char inb[INBUFSIZE];		/* input buffer */
	int inlock;		/* wait for input (1) */
	int keypos;		/* read pointer */
	int mode;		/* not buffered (0), buffered (1), fully buffered (2) */
};
#endif

struct terminal *current_term;

void putc_console(char);
