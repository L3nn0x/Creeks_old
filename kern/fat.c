#include "fat.h"

struct fat_BS *fat_get_disk_info(int device, struct partition *part)
{
	int i, j;
	struct fat_BS *hd;

	hd = (struct fat_BS *) kmalloc(sizeof(struct fat_BS));

	disk_read(device,0,(char*)hd,sizeof(struct fat_BS));

	return hd;
}

