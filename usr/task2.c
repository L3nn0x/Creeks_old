#include "libc/libc.h"
#include "libc/syscalls.h"
#include "libc/malloc.h"

void foo(void)
{
	console_write("signal trapped !\n", GREEN);
}

int main(void)
{
	sigaction(1, foo);
	while (1);

	exit(0);

	return 0;
}

