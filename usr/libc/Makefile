EXE=libc.a

FLAGS = -Wall -fno-stack-protector -fno-builtin

CC = gcc

SRC = libc.c syscalls.c malloc.c

OBJ = $(SRC:.c=.o)

all: $(EXE)

$(EXE): $(OBJ)
	ar rcs $@ $^
	ranlib $@

%.o: %.c
	$(CC) -c $^ -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(EXE)

re: fclean all
