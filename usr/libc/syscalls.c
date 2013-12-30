#include "libc.h"


void console_write(char *msg, char color)
{
	int sys_num = 1;

	asm volatile ("		   \
		mov %0, %%eax	\n \
		mov %1, %%ebx	\n \
		mov %2, %%ecx   \n \
		int $0x30"
		:: "g" (sys_num), "g" (msg), "g"(color));
}

void exit(int status)
{
	int sys_num = 2;

	asm volatile ("		   \
		mov %0, %%eax	\n \
		mov %1, %%ebx	\n \
		int $0x30"
		:: "g" (sys_num), "g" (status)
	);
	while (1);
}

int open(char *file)
{
	int fd;
	int sys_num = 3;

	asm volatile("			   \
		mov %1, %%eax		\n \
		mov %2, %%ebx		\n \
		int $0x30		\n \
		mov %%eax, %0"
		: "=g" (fd)
		: "g" (sys_num), "m" (file)
	);

	return fd;
}

int read(int fd, char *buf, int size)
{
	int count;
	int sys_num = 4;

	asm volatile ("			   \
		mov %1, %%eax		\n \
		mov %2, %%ebx		\n \
		mov %3, %%ecx		\n \
		mov %4, %%edx		\n \
		int $0x30		\n \
		mov %%eax, %0"
		: "=g" (count)
		: "g" (sys_num), "g" (fd), "g" (buf), "g" (size)
	);

	return count;
}

void close(int fd)
{
	int sys_num = 5;

	asm volatile ("		   \
		mov %0, %%eax	\n \
		mov %1, %%ebx	\n \
		int $0x30"
		:: "g" (sys_num), "g" (fd)
	);
}

int console_read(char *buf)
{
	int count;
	int sys_num = 6;

	asm volatile ("		   \
		mov %1, %%eax	\n \
		mov %2, %%ebx	\n \
		int $0x30	\n \
		mov %%eax, %0"
		: "=g" (count)
		: "g" (sys_num), "g" (buf)
	);

	return count;
}

int chdir(char *path)
{
	int sys_num = 7,ok=0;

	asm volatile ("		   \
		mov %1, %%eax	\n \
		mov %2, %%ebx	\n \
		int $0x30 \n \
		mov %%eax, %0"
		:"=g"(ok): "g" (sys_num), "g" (path)
	);
	return ok;
}

void pwd(char *buf)
{
	int sys_num = 8;

	asm volatile ("		   \
		mov %0, %%eax	\n \
		mov %1, %%ebx	\n \
		int $0x30"
		:: "g" (sys_num), "g" (buf)
	);
}

int exec(char *path, char *argv[])
{
	int ret;
	int sys_num = 9;

	asm volatile ("		   \
		mov %1, %%eax	\n \
		mov %2, %%ebx	\n \
		mov %3, %%ecx	\n \
		int $0x30	\n \
		mov %%eax, %0"
		: "=g" (ret)
		: "g" (sys_num), "g" (path), "g" (argv)
	);

	return ret;
}

void* sbrk(int n)
{
	char *addr;
	int sys_num = 10;

	asm volatile("			   \
		mov %1, %%eax		\n \
		mov %2, %%ebx		\n \
		int $0x30		\n \
		mov %%eax, %0"
		: "=g" (addr)
		: "g" (sys_num), "m" (n)
	);

	return addr;
}

int wait(int *status)
{
	int pid;
	int sys_num = 11;

	asm volatile("			   \
		mov %1, %%eax		\n \
		mov %2, %%ebx		\n \
		int $0x30		\n \
		mov %%eax, %0"
		: "=g" (pid)
		: "g" (sys_num), "m" (status)
	);

	return pid;
}

int kill(int pid, int sig)
{
	int ret;
	int sys_num = 12;

	asm volatile("			   \
		mov %1, %%eax		\n \
		mov %2, %%ebx		\n \
		mov %3, %%ecx		\n \
		int $0x30		\n \
		mov %%eax, %0"
		: "=g" (ret)
		: "g" (sys_num), "g" (pid), "g" (sig)
	);

	return ret;
}

int sigaction(int sig, void *fn)
{
	int ret;
	int sys_num = 13;

	asm volatile("			   \
		mov %1, %%eax		\n \
		mov %2, %%ebx		\n \
		mov %3, %%ecx		\n \
		int $0x30		\n \
		mov %%eax, %0"
		: "=g" (ret)
		: "g" (sys_num), "g" (sig), "g" (fn)
	);

	return ret;
}

void create_dir(char* path)
{
	int sys_num=15;
	asm volatile("mov %0, %%eax\n \
			mov %1 ,%%ebx \n \
			int $0x30"::"g"(sys_num), "g"(path));
}
