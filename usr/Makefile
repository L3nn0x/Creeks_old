OBJ = shell cat kill task1 task2

CC = gcc

FLAGS = -m32 -Wall -fno-stack-protector -fno-builtin

all: $(OBJ)

shell: shell.o libc/libc.a
	ld -Ttext=40000000 --entry=main -o $@ $^

cat: cat.o libc/libc.a
	ld -Ttext=40000000 --entry=main -o $@ $^

kill: kill.o libc/libc.a
	ld -Ttext=40000000 --entry=main -o $@ $^

task1: task1.o libc/libc.a
	ld -Ttext=40000000 --entry=main -o $@ $^

task2: task2.o libc/libc.a
	ld -Ttext=40000000 --entry=main -o $@ $^

libc/libc.a:
	make -C libc

%.o: %.c
	$(CC) $(FLAGS) -o $@ -c $^ -I./libc

clean:
	rm -f $(OBJ) *.o *.a
	make -C libc clean

re: clean all
