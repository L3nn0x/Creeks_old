#include "libc/libc.h"
#include "libc/syscalls.h"
#include "libc/malloc.h"

#define SIZE		512

int main(int argc, char **argv)
{
	char out[SIZE];
	int signal, pid;

	if (argc < 3) {
		sprintf(out, "usage: %s <signal> <pid>\n", argv[0]);
		console_write(out, GREEN);
		exit(1);
	}

	signal = atoi(argv[1]);
	pid = atoi(argv[2]);

	kill(pid, signal);

	exit(0);

	/* never goes there */
	return 0;
}

