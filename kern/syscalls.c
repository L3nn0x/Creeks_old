#include "types.h"
#include "list.h"
#include "lib.h"
#include "io.h"
#include "process.h"
#include "kmalloc.h"
#include "mm.h"
#include "schedule.h"
#include "file.h"
#include "console.h"
#include "signal.h"
#include "syscalls.h"
#include "screen.h"


void do_syscalls(int sys_num)
{
	u32 *stack_ptr;

	/* 
	 * Stocke dans stack_ptr le pointeur vers les registres sauvegardes.
	 * Les arguments sont transmis dans : ebx, ecx, edx, esi edi
	 * Le code de retour sera transmis dans %eax : stack_ptr[14]
	 */
	asm("mov %%ebp, %0": "=m"(stack_ptr):);

	/* sys_console_write */
	if (sys_num == 1) {
		char *u_str;
		int color=0x02;

		asm("mov %%ebx, %0\n mov %%ecx, %1": "=m"(u_str), "=m"(color):);

		cli;
		kattr=color;
		printk(u_str);
		kattr=0x02;
		sti;
	}

	else if (sys_num == 2) {
		int status;

		cli;
		asm("mov %%ebx, %0": "=m"(status):);
		sys_exit(status);
	}

	else if (sys_num == 3) {
		char *path;

		cli;
		asm("mov %%ebx, %0": "=m"(path):);
		stack_ptr[14] = sys_open(path);
	}

	/* sys_read */
	else if (sys_num == 4) {
		char *u_buf;	/* buffer d'entree utilisateur */
		u32 fd;
		u32 bufsize;
		struct open_file *of;

		cli;
		asm("	mov %%ebx, %0;	\
			mov %%ecx, %1;	\
			mov %%edx, %2": "=m"(fd), "=m"(u_buf), "=m"(bufsize):);

		//// printk("DEBUG: sys_read(): reading %d bytes on fd %d\n", bufsize, fd);

		of = current->fd;
		while (fd) {
			of = of->next;
			if (of == 0) {
				printk ("ERROR: sys_read(): invalid file descriptor\n");
				stack_ptr[14] = -1;
				return;
			}
			fd--;
		}

		if ((of->ptr + bufsize) > of->file->inode->i_size)
			bufsize = of->file->inode->i_size - of->ptr;

		memcpy(u_buf, (char *) (of->file->mmap + of->ptr), bufsize);
		of->ptr += bufsize;

		stack_ptr[14] = bufsize;
	}

	/* sys_close */
	else if (sys_num == 5) {
		u32 fd;
		struct open_file *of;

		cli;
		asm("mov %%ebx, %0": "=m"(fd):);

		//// printk("DEBUG: sys_close(): process[%d] closing fd %d\n", current->pid, fd);

		of = current->fd;
		while (fd) {
			of = of->next;
			if (of == 0) {
				printk("ERROR: sys_close(): invalid file descriptor\n");
				return;
			}
			fd--;
		}

		kfree(of->file->mmap);
		of->file->mmap = 0;
		of->file = 0;
		of->ptr = 0;
	}

	else if (sys_num == 6) {
		char *u_buf;

		asm("mov %%ebx, %0": "=m"(u_buf):);

		stack_ptr[14] = sys_console_read(u_buf);
	}

	/* sys_chdir */
	else if (sys_num == 7) {
		char *path;
		struct file *fp;
		int ok=0;

		cli;
		asm("mov %%ebx, %0": "=m"(path):);

		if (!(fp = path_to_file(path))) {
			ok=-1;
			stack_ptr[14]=ok;
			return;
		}

		if (!fp->inode)
			fp->inode = ext2_read_inode(fp->disk, fp->inum);

		if (!is_directory(fp)) {
			ok=-2;
			stack_ptr[14]=ok;
			return;
		}

		current->pwd = fp;
		stack_ptr[14]=0;
		//// printk("DEBUG: sys_chdir() to %s\n", current->pwd->name);
	}

	/* sys_pwd */
	else if (sys_num == 8) {
		char *u_buf;
		int sz;
		struct file *fp;

		cli;
		asm("mov %%ebx, %0": "=m"(u_buf):);

		if (current->pwd == f_root) {
			u_buf[0] = '/';
			u_buf[1] = 0;
			return;
		}

		fp = current->pwd;
		sz = strlen(fp->name) + 1;
		while (fp->parent != f_root) {
			fp = fp->parent;
			sz += (strlen(fp->name) + 1);
		}

		fp = current->pwd;
		u_buf[sz] = 0;

		while (sz > 0) {
			memcpy(u_buf + sz - strlen(fp->name), fp->name,
			       strlen(fp->name));
			sz -= (strlen(fp->name) + 1);
			u_buf[sz] = '/';
			fp = fp->parent;
		}

		//// printk("DEBUG: sys_pwd(): %s\n", current->pwd->name);
	}

	else if (sys_num == 9) {
		char *path;
		char **argv;

		asm("	mov %%ebx, %0	\n \
			mov %%ecx, %1"
			: "=m"(path), "=m"(argv) :);

		stack_ptr[14] = sys_exec(path, argv);
	}

	else if (sys_num == 10) {
		int  size;

		asm("mov %%ebx, %0": "=m"(size):);
		stack_ptr[14] = (u32) sys_sbrk(size);
	}

	else if (sys_num == 11) {
		int *status;

		asm("mov %%ebx, %0": "=m"(status):);
		stack_ptr[14] = sys_wait(status);
	}

	/* sys_kill */
	else if (sys_num == 12) {
		int pid, sig;

		cli;
		asm("	mov %%ebx, %0	\n \
			mov %%ecx, %1"
			: "=m"(pid), "=m"(sig) :);

		//printk("DEBUG: sys_kill(): sending signal %d to process %d\n", sig, pid);

		if (p_list[pid].state > 0) {
			set_signal(&p_list[pid].signal, sig);
			stack_ptr[14] = 0;
		}
		else 
			stack_ptr[14] = -1;
	}

	/* sys_sigaction */
	else if (sys_num == 13) {
		char *fn;
		int sig;

		cli;
		asm("	mov %%ebx, %0	\n \
			mov %%ecx, %1"
			: "=m"(sig), "=m"(fn) :);

		//printk("DEBUG: sys_sigaction(): signal %d trapped at %p by process %d\n", 
			//sig, fn, current->pid);
		
		if (sig < 32)
			current->sigfn[sig] = fn;

		stack_ptr[14] = 0;
	}

	else if (sys_num == 14) {
		cli;
		sys_sigreturn();
	}
	else if(sys_num==15){
		cli;
		char *path;
		asm("mov %%ebx, %0":"=m"(path):);
		add_file(hd,path,REP);
	}

	/* debug */
	else if (sys_num == 100) {
		u32 *pa;
		asm("mov %%ebp, %0": "=m"(pa):);
		printk("eax: %p ecx: %p edx: %p ebx: %p\n", pa[12], pa[11], pa[10], pa[9]);
		printk("ds: %p esi: %p edi: %p\n", pa[4], pa[6], pa[5]);
		printk("ss: %p ebp: %p esp: %p\n", pa[17], pa[7], pa[16]);
		printk("cs: %p eip: %p\n", pa[14], pa[13]);
	}

	else
		printk("unknown syscall %d\n", sys_num);

	sti;

	return;
}
