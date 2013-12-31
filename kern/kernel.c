#include "types.h"
#include "gdt.h"
#include "screen.h"
#include "io.h"
#include "idt.h"
#include "lib.h"
#include "mm.h"
#include "process.h"
#include "disk.h"
#include "kmalloc.h"
#include "ext2.h"
#include "elf.h"
#include "file.h"
#include "vgaM.h"

struct mb_partial_info {
	unsigned long flags;
	unsigned long low_mem;
	unsigned long high_mem;
	unsigned long boot_device;
	unsigned long cmdline;
};

void ok_msg(void)
{
	kX = 40;
	kattr = 0x0A;
	printk("OK\n");
	kattr = 0x07;
}

void kmain(struct mb_partial_info *mbi)
{
	printk("Pepin is booting...\n");
	printk("RAM detected : %uk (lower), %uk (upper)\n", mbi->low_mem, mbi->high_mem);

	cli;

	/* Initialisation de la GDT et des segments */
	printk("Loading GDT");
	init_gdt();
	asm("	movw $0x18, %%ax \n \
		movw %%ax, %%ss \n \
		movl %0, %%esp"::"i" (KERN_STACK));
	ok_msg();

	printk("Loading IDT");
	init_idt();
	ok_msg();

	printk("Configure PIC");
	init_pic();
	ok_msg();

	printk("Loading Task Register");
	asm("	movw $0x38, %ax; ltr %ax");
	ok_msg();

	printk("Enabling paging");
	init_mm(mbi->high_mem);
	ok_msg();
	hide_hw_cursor();

	{
		struct partition *p1;
		struct file *fp;
		struct terminal tty1;

		/*
		 * Lecture des informations relatives a la premiere partition
		 */
		p1 = (struct partition *) kmalloc(sizeof(struct partition));
		disk_read(0, 0x01BE, (char *) p1, 16);
		printk("Partition found: bootable: %x, block start: %d, block size: %d\n",
		     p1->bootable, p1->s_lba, p1->size);

		/*
		 * On alloue une structure pour stocker les informations
		 * relative au filesystem
		 */
		hd = ext2_get_disk_info(0, p1);
		f_root=init_root(hd);
		printk("Filesystem found ! \n");

		/* Initialise le terminal */
		tty1.pread = tty1.pwrite = 0;
		current_term = &tty1;
		printk("Terminal initialized\n");

		/* Initialise le thread kernel */
		current = &p_list[0];
		current->pid = 0;
		current->state = 1;
		current->regs.cr3 = (u32) pd0;
		current->pwd=f_root;
		printk("Thread kernel initialized\n");

		//lancement du shell
		fp=path_to_file("/bin/shell");
		fp->inode = ext2_read_inode(hd, fp->inum);
		load_task(hd, fp->inode, 0, 0);

		kattr=0x47;
		printk("Interruptions activees\n");
		kattr=0x02;

		sti;

		while (1) {
			if(n_proc==0){
				cli;
				load_task(hd, fp->inode, 0, 0);
				sti;
			}
		}
	}

}
