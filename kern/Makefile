CC = gcc -g -m32 -Wall -fno-stack-protector -fno-builtin -I. -I./syscalls

SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)

SRC_SYSCALLS= $(wildcard syscalls/*.c)
OBJ_SYSCALLS= $(SRC_SYSCALLS:.c=.o)


all: kernel

kernel: $(OBJ) $(OBJ_SYSCALLS)
	$(MAKE) -C asm
	ld --oformat=elf32-i386 -Ttext=100000 --entry=_start asm/*.o $^ -o $@

.o: .c
	$(CC) -c $^

clean:
	$(MAKE) clean -C asm
	rm -f $(OBJ) $(OBJ_SYSCALLS)

fclean: clean
	rm -f kernel

indent:
	indent -kr -i8 -ts8 *.c *.h */*.c */*.h

re: fclean all
