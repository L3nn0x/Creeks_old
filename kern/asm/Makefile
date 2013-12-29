SRC = boot.asm int.asm sched.asm

CC = nasm

FLAGS = -f elf32

OBJ = $(SRC:.asm=.o)

all: $(OBJ)

%.o: %.asm
	$(CC) $(FLAGS) -o $@ $^

clean:
	rm -f $(OBJ)

fclean: clean

re: fclean all
