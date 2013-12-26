#include "process.h"

char* sys_sbrk(int size)
{
	char *ret;
	ret = current->e_heap;

	current->e_heap += size;
	return ret;
}
