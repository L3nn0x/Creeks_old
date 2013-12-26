
#define MALLOC_MINSIZE	16
#define PAGESIZE	4096


struct malloc_header {
	unsigned long size:31;	/* taille totale de l'enregistrement */
	unsigned long used:1;
} __attribute__ ((packed));

#ifdef __MALLOC__
	char *b_heap = 0;	/* pointe sur le debut du heap */
	char *e_heap = 0;		/* pointe sur le sommet du heap */
#else
	extern char *b_heap;
	extern char *e_heap;
#endif


void* malloc(unsigned long);
void free(char *);
