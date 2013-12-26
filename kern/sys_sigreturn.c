#include "types.h"
#include "io.h"
#include "process.h"
#include "schedule.h"
#include "syscalls.h"

void sys_sigreturn(void)
{
	u32 *esp;

	cli;

	asm("mov (%%ebp), %%eax; mov %%eax, %0": "=m"(esp):);
	esp += 17;

	current->kstack.esp0 = esp[17];
	current->regs.ss = esp[16];
	current->regs.esp = esp[15];
	current->regs.eflags = esp[14];
	current->regs.cs = esp[13];
	current->regs.eip = esp[12];
	current->regs.eax = esp[11];
	current->regs.ecx = esp[10];
	current->regs.edx = esp[9];
	current->regs.ebx = esp[8];
	current->regs.ebp = esp[7];
	current->regs.esi = esp[6];
	current->regs.edi = esp[5];
	current->regs.ds = esp[4];
	current->regs.es = esp[3];
	current->regs.fs = esp[2];
	current->regs.gs = esp[1];

	switch_to_task(0, KERNELMODE);
}
