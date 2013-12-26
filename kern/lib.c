#include <stdarg.h>
#include "screen.h"

/* La fonction memcpy permet de copier n octets de src vers dest.
 * Les adresses sont lineaires.
 */
void *memcpy(char *dst, char *src, int n)
{
	char *p = dst;
	while (n--)
		*dst++ = *src++;
	return p;
}

int strcpy(char *dst, char *src)
{
	int i = 0;
	while ((dst[i] = src[i++]));

	return i;
}

int strcmp(char *dst, char *src)
{
	int i = 0;

	while ((dst[i] == src[i])) {
		if (src[i++] == 0)
			return 0;
	}

	return 1;
}

int strlen(char *s)
{
	int i = 0;
	while (*s++)
		i++;
	return i;
}

void itoa(char *buf, unsigned long int n, int base)
{
	unsigned long int tmp;
	int i, j;

	tmp = n;
	i = 0;

	do {
		tmp = n % base;
		buf[i++] = (tmp < 10) ? (tmp + '0') : (tmp + 'a' - 10);
	} while (n /= base);
	buf[i--] = 0;

	for (j = 0; j < i; j++, i--) {
		tmp = buf[j];
		buf[j] = buf[i];
		buf[i] = tmp;
	}
}

