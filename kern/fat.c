#include "fat.h"
#include "ext2.h"
#include "kmalloc.h"
#include "disk.h"

struct fat_BS *fat_get_disk_info(int device, struct partition *part)
{
	struct fat_BS *hd;

	hd = (struct fat_BS *) kmalloc(sizeof(struct fat_BS));

	disk_read(device,0,(char*)hd,sizeof(struct fat_BS));

	return hd;
}

