#include "types.h"
#include "list.h"
#include "screen.h"
#include "io.h"
#include "kbd.h"
#include "lib.h"
#include "process.h"
#include "schedule.h"
#include "mm.h"
#include "kmalloc.h"
#include "syscalls.h"
#include "console.h"

void isr_default_int(void)
{
	printk("interrupt\n");
}

void isr_GP_exc(void)
{
	printk("#GP\n");
	asm("hlt");
}

void isr_PF_exc(void)
{
	u32 faulting_addr, code;
	u32 eip;
	struct page *pg;

 	asm(" 	movl 60(%%ebp), %%eax	\n \
    		mov %%eax, %0		\n \
		mov %%cr2, %%eax	\n \
		mov %%eax, %1		\n \
 		movl 56(%%ebp), %%eax	\n \
    		mov %%eax, %2"
		: "=m"(eip), "=m"(faulting_addr), "=m"(code));

	//// printk("DEBUG: isr_PF_exc(): #PF on eip: %p. cr2: %p code: %p\n", eip, faulting_addr, code);

	if (faulting_addr >= USER_OFFSET && faulting_addr < USER_STACK) {
		pg = (struct page *) kmalloc(sizeof(struct page));
		pg->p_addr = get_page_frame(); 
		pg->v_addr = (char *) (faulting_addr & 0xFFFFF000);
		list_add(&pg->list, &current->pglist);
		pd_add_page(pg->v_addr, pg->p_addr, PG_USER, current->pd);
	} 
	else {		
		printk("Segmentation fault on eip: %p. cr2: %p code: %p\n", eip, faulting_addr, code);
		sys_exit(1);
	}
}

void isr_clock_int(void)
{
	static int tic = 0;
	static int sec = 0;
	tic++;
	if (tic % 100 == 0) {
		sec++;
		tic = 0;
	}
	schedule();
}

void isr_kbd_int(void)
{
	uchar i;
	static int lshift_enable;
	static int rshift_enable;
	static int alt_enable;
	static int ctrl_enable;

	do {
		i = inb(0x64);
	} while ((i & 0x01) == 0);

	i = inb(0x60);
	i--;

	if (i < 0x80) {		/* touche enfoncee */
		switch (i) {
		case 0x29:
			lshift_enable = 1;
			break;
		case 0x35:
			rshift_enable = 1;
			break;
		case 0x1C:
			ctrl_enable = 1;
			break;
		case 0x37:
			alt_enable = 1;
			break;
		default:
			putc_console(kbdmap
			       [i * 4 + (lshift_enable || rshift_enable)]);
		}
	} else {		/* touche relachee */
		i -= 0x80;
		switch (i) {
		case 0x29:
			lshift_enable = 0;
			break;
		case 0x35:
			rshift_enable = 0;
			break;
		case 0x1C:
			ctrl_enable = 0;
			break;
		case 0x37:
			alt_enable = 0;
			break;
		}
	}

}
