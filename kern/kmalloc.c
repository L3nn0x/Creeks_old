#include "types.h"
#include "lib.h"
#include "mm.h"
#include "kmalloc.h"

void *ksbrk(int n)
{
	struct kmalloc_header *chunk;
	char *p_addr;
	int i;

	if ((kern_heap + (n * PAGESIZE)) > (char *) KERN_HEAP_LIM) {
		printk
		    ("PANIC: ksbrk(): no virtual memory left for kernel heap !\n");
		return (char *) -1;
	}

	chunk = (struct kmalloc_header *) kern_heap;

	/* Allocation d'une page libre */
	for (i = 0; i < n; i++) {
		p_addr = get_page_frame();
		if (p_addr < 0) {
			printk
			    ("PANIC: ksbrk(): no free page frame available !\n");
			return (char *) -1;
		}

		/* Ajout dans le repertoire de pages */
		pd0_add_page(kern_heap, p_addr, 0);

		kern_heap += PAGESIZE;
	}

	/* Marquage pour kmalloc */
	chunk->size = PAGESIZE * n;
	chunk->used = 0;

	return chunk;
}

void *kmalloc(unsigned long size)
{
	unsigned long realsize;	/* taille totale de l'enregistrement */
	struct kmalloc_header *chunk, *other;

	if ((realsize =
	     sizeof(struct kmalloc_header) + size) < KMALLOC_MINSIZE)
		realsize = KMALLOC_MINSIZE;

	/* 
	 * On recherche un bloc libre de 'size' octets en parcourant le HEAP
	 * kernel a partir du debut
	 */
	chunk = (struct kmalloc_header *) KERN_HEAP;
	while (chunk->used || chunk->size < realsize) {
		if (chunk->size == 0) {
			printk
			    ("PANIC: kmalloc(): corrupted chunk on %x with null size (heap %x) !\nSystem halted\n",
			     chunk, kern_heap);
			asm("hlt");
		}

		chunk =
		    (struct kmalloc_header *) ((char *) chunk +
					       chunk->size);

		if (chunk == (struct kmalloc_header *) kern_heap) {
			if (ksbrk((realsize / PAGESIZE) + 1) < 0) {
				printk
				    ("PANIC: kmalloc(): no memory left for kernel !\nSystem halted\n");
				asm("hlt");
			}
		} else if (chunk > (struct kmalloc_header *) kern_heap) {
			printk
			    ("PANIC: kmalloc(): chunk on %x while heap limit is on %x !\nSystem halted\n",
			     chunk, kern_heap);
			asm("hlt");
		}
	}

	/* 
	 * On a trouve un bloc libre dont la taille est >= 'size'
	 * On fait de sorte que chaque bloc est une taille minimale
	 */
	if (chunk->size - realsize < KMALLOC_MINSIZE)
		chunk->used = 1;
	else {
		other =
		    (struct kmalloc_header *) ((char *) chunk + realsize);
		other->size = chunk->size - realsize;
		other->used = 0;

		chunk->size = realsize;
		chunk->used = 1;
	}

	kmalloc_used += realsize;

	/* retourne un pointeur sur la zone de donnees */
	return (char *) chunk + sizeof(struct kmalloc_header);
}

void kfree(void *v_addr)
{
	struct kmalloc_header *chunk, *other;

	/* On libere le bloc alloue */
	chunk =
	    (struct kmalloc_header *) (v_addr -
				       sizeof(struct kmalloc_header));
	chunk->used = 0;

	kmalloc_used -= chunk->size;

	/* 
	 * On merge le bloc nouvellement libere avec le bloc suivant ci celui-ci
	 * est aussi libre
	 */
	while ((other =
		(struct kmalloc_header *) ((char *) chunk + chunk->size))
	       && other < (struct kmalloc_header *) kern_heap
	       && other->used == 0)
		chunk->size += other->size;
}
