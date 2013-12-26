
#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGBUS		 7
#define SIGKILL		 9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19

#define SIG_DFL		0	/* default signal handling */
#define SIG_IGN		1	/* ignore signal */

#define set_signal(mask, sig)	*(mask) |= ((u32) 1 << (sig - 1))
#define clear_signal(mask, sig)	*(mask) &= ~((u32) 1 << (sig - 1))
#define is_signal(mask, sig)	(mask & ((u32) 1 << (sig - 1)))

int dequeue_signal(int);
int handle_signal(int);

