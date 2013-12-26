#include <stdarg.h>

/*
 * Appels systeme
 */
void write_console(char *msg)
{
	int sys_num = 1;
	asm volatile ("		   \
		mov %0, %%ebx	\n \
		mov %1, %%eax	\n \
		int $0x30" 
		:: "g" (msg), "g" (sys_num)
	);
}

void exit(void)
{
	int sys_num = 2;
	asm volatile ("		   \
		mov %0, %%eax	\n \
		int $0x30" 
		:: "g" (sys_num)
	);
}

int open(char *file)
{
	int fd;
	int sys_num = 3;

	asm volatile("			   \
		mov %1, %%ebx		\n \
		mov %2, %%eax		\n \
		int $0x30		\n \
		mov %%eax, %0"	
		: "=g" (fd) 
		: "m" (file), "g" (sys_num) );
	
	return fd;
}

int read(int fd, char *buf, int size)
{
	int count;
	int sys_num = 4;

	asm volatile ("			   \
		mov %1, %%ebx		\n \
		mov %2, %%ecx		\n \
		mov %3, %%edx		\n \
		mov %4, %%eax		\n \
		int $0x30		\n \
		mov %%eax, %0" 
		: "=g" (count) 
		: "g" (fd), "g" (buf), "g" (size), "g" (sys_num) );
	
	return count;
}

void close(int fd)
{
	int sys_num = 5;
	asm volatile ("		   \
		mov %0, %%ebx	\n \
		mov %1, %%eax	\n \
		int $0x30" 
		:: "g" (fd), "g" (sys_num));
}

