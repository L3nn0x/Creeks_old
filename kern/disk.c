#include "types.h"
#include "kmalloc.h"
#include "io.h"
#include "lib.h"

int bl_common(int drive, int numblock, int count)
{
	outb(0x1F1, 0x00);	/* NULL byte to port 0x1F1 */
	outb(0x1F2, count);	/* Sector count */
	outb(0x1F3, (unsigned char) numblock);	/* Low 8 bits of the block address */
	outb(0x1F4, (unsigned char) (numblock >> 8));	/* Next 8 bits of the block address */
	outb(0x1F5, (unsigned char) (numblock >> 16));	/* Next 8 bits of the block address */

	/* Drive indicator, magic bits, and highest 4 bits of the block address */
	outb(0x1F6, 0xE0 | (drive << 4) | ((numblock >> 24) & 0x0F));

	return 0;
}

int bl_read(int drive, int numblock, int count, char *buf)
{
	u16 tmpword;
	int idx;

	bl_common(drive, numblock, count);
	outb(0x1F7, 0x20);

	/* Wait for the drive to signal that it's ready: */
	while (!(inb(0x1F7) & 0x08));

	for (idx = 0; idx < 256 * count; idx++) {
		tmpword = inw(0x1F0);
		buf[idx * 2] = (unsigned char) tmpword;
		buf[idx * 2 + 1] = (unsigned char) (tmpword >> 8);
	}

	return count;
}

int bl_write(int drive, int numblock, int count, char *buf)
{
	u16 tmpword;
	int idx;

	bl_common(drive, numblock, count);
	outb(0x1F7, 0x30);

	/* Wait for the drive to signal that it's ready: */
	while (!(inb(0x1F7) & 0x08));

	for (idx = 0; idx < 256 * count; idx++) {
		tmpword = (buf[idx * 2 + 1] << 8) | buf[idx * 2];
		outw(0x1F0, tmpword);
	}

	return count;
}

/* 
 * Read count bytes on disk
 */
int disk_write(int drive, int offset, char *buf, int count)
{
	int bl_begin, bl_end, blocks;

	bl_begin = offset / 512;
	bl_end = (offset + count) / 512;
	blocks = bl_end - bl_begin + 1;

	bl_write(drive, bl_begin, blocks, buf);

	return count;
}

int disk_read(int drive, int offset, char *buf, int count)
{
	char *bl_buffer;
	int bl_begin, bl_end, blocks;

	bl_begin = offset / 512;
	bl_end = (offset + count) / 512;
	blocks = bl_end - bl_begin + 1;

	bl_buffer = (char *) kmalloc(blocks * 512);

	bl_read(drive, bl_begin, blocks, bl_buffer);
	memcpy(buf, (char *) (bl_buffer + offset % 512), count);

	kfree(bl_buffer);

	return count;
}
