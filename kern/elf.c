#include "elf.h"
#include "ext2.h"
#include "mm.h"
#include "lib.h"
#include "kmalloc.h"

/* 
 * Teste si le fichier dont l'adresse est passee en argument
 * est au format ELF
 */
int is_elf(char *file)
{
	Elf32_Ehdr *hdr;

	hdr = (Elf32_Ehdr *) file;
	if (hdr->e_ident[0] == 0x7f && hdr->e_ident[1] == 'E'
	    && hdr->e_ident[2] == 'L' && hdr->e_ident[3] == 'F')
		return 1;
	else
		return 0;
}

u32 load_elf(char *file, struct process *proc)
{
	char *p;
	u32 v_begin, v_end;
	Elf32_Ehdr *hdr;
	Elf32_Phdr *p_entry;
	int i, pe;

	hdr = (Elf32_Ehdr *) file;
	p_entry = (Elf32_Phdr *) (file + hdr->e_phoff);	/* Program header table offset */

	if (!is_elf(file)) {
		printk("INFO: load_elf(): file not in ELF format !\n");
		return 0;
	}

	for (pe = 0; pe < hdr->e_phnum; pe++, p_entry++) {	/* Read each entry */

		if (p_entry->p_type == PT_LOAD) {
			v_begin = p_entry->p_vaddr;
			v_end = p_entry->p_vaddr + p_entry->p_memsz;

			if (v_begin < USER_OFFSET) {
				printk ("INFO: load_elf(): can't load executable below %p\n", USER_OFFSET);
				return 0;
			}

			if (v_end > USER_STACK) {
				printk ("INFO: load_elf(): can't load executable above %p\n", USER_STACK);
				return 0;
			}


			/* Description de la zone exec + rodata */
			if (p_entry->p_flags == PF_X + PF_R) {	
				proc->b_exec = (char*) v_begin;
				proc->e_exec = (char*) v_end;
			}

			/* Description de la zone bss */
			if (p_entry->p_flags == PF_W + PF_R) {	
				proc->b_bss = (char*) v_begin;
				proc->e_bss = (char*) v_end;
			}

			memcpy((char *) v_begin, (char *) (file + p_entry->p_offset), p_entry->p_filesz);

			if (p_entry->p_memsz > p_entry->p_filesz)
				for (i = p_entry->p_filesz, p = (char *) p_entry->p_vaddr; i < p_entry->p_memsz; i++)
					p[i] = 0;
		}
	}

	/* Return program entry point */
	return hdr->e_entry;
}
