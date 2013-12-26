#include "libc/libc.h"
#include "libc/syscalls.h"
#include "libc/malloc.h"

#define SIZE		512

int main(int argc, char **argv)
{
	char *buf, out[SIZE];
	int fd, n, i;

	if (-1 == (int) (buf = (char*) malloc(SIZE))) {
		console_write("error: malloc() failed\n",GREEN);
		exit(1);
	}

	for (i=1 ; i<argc ; i++) {
		if (-1 == (fd = open(argv[i]))) {
			sprintf(out, "error: open() %s failed\n", argv[i]);
			console_write(out,GREEN);
			free(buf);
		}

		else {
			do {
				if (-1 == (n = read(fd, buf, SIZE-1))) {
					console_write("error: read() failed\n",GREEN);
					free(buf);
					break;
				}
				buf[n] = 0;
				console_write(buf,GREEN);
			} while (n);

			close(fd);
			console_write("\n",GREEN);
		}
	}

	free(buf);

	exit(0);

	/* never goes there */
	return 0;
}

