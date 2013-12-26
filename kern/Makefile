
CC = gcc -g -Wall -fno-stack-protector -fno-builtin -I.

SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)

SRC_SYSCALLS= $(wildcard syscalls/*.c)
OBJ_SYSCALLS= $(SRC_SYSCALLS:.c=.o)


all: kernel

kernel: boot.o sched.o int.o $(OBJ) $(OBJ_SYSCALLS)
	ld -Ttext=100000 --entry=_start $^ -o $@

boot.o: boot.asm
	nasm -f elf -o $@ $^

sched.o: sched.asm
	nasm -f elf -o $@ $^

int.o: int.asm
	nasm -f elf -o $@ $^

.o: .c
	$(CC) -c $^

clean:
	rm -f kernel *.o syscalls/*.o

indent:
	indent -kr -i8 -ts8 *.c *.h */*.c */*.h

re: clean all
