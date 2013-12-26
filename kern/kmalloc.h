#include "types.h"

#define KMALLOC_MINSIZE		16

struct kmalloc_header {
	unsigned long size:31;	/* taille totale de l'enregistrement */
	unsigned long used:1;
} __attribute__ ((packed));

void *ksbrk(int);
void *kmalloc(unsigned long);
void kfree(void *);
