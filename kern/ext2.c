#include "ext2.h"
#include "disk.h"
#include "kmalloc.h"
#include "lib.h"

/*
 * Initialise la structure decrivant le disque logique.
 * Offset correspond au debut de la partition.
 */
struct disk *ext2_get_disk_info(int device, struct partition *part)
{
	int i, j;
	struct disk *hd;

	hd = (struct disk *) kmalloc(sizeof(struct disk));

	hd->device = device;
	hd->part = part;
	hd->sb = ext2_read_sb(hd, part->s_lba * 512);
	hd->blocksize = 1024 << hd->sb->s_log_block_size;

	i = (hd->sb->s_blocks_count / hd->sb->s_blocks_per_group) +
	    ((hd->sb->s_blocks_count % hd->sb->s_blocks_per_group) ? 1 : 0);
	j = (hd->sb->s_inodes_count / hd->sb->s_inodes_per_group) +
	    ((hd->sb->s_inodes_count % hd->sb->s_inodes_per_group) ? 1 : 0);
	hd->groups = (i > j) ? i : j;

	hd->gd = ext2_read_gd(hd, part->s_lba * 512);

	return hd;
}

struct ext2_super_block *ext2_read_sb(struct disk *hd, int s_part)
{
	struct ext2_super_block *sb;

	sb = (struct ext2_super_block *) kmalloc(sizeof(struct ext2_super_block));
	disk_read(hd->device, s_part + 1024, (char *) sb, sizeof(struct ext2_super_block));

	return sb;
}

struct ext2_group_desc *ext2_read_gd(struct disk *hd, int s_part)
{
	struct ext2_group_desc *gd;
	int offset, gd_size;

	/* localisation du bloc */
	offset = (hd->blocksize == 1024) ? 2048 : hd->blocksize;

	/* taille occupee par les descripteurs */
	gd_size = hd->groups * sizeof(struct ext2_group_desc);

	/* creation du tableau de descripteurs */
	gd = (struct ext2_group_desc *) kmalloc(gd_size);

	disk_read(hd->device, s_part + offset, (char *) gd, gd_size);

	return gd;
}

void ext2_write_gd(struct disk *hd, struct ext2_group_desc* gd, int s_part)
{
	int offset=(hd->blocksize==1024)?2048 : hd->blocksize;
	int gd_size=hd->groups*sizeof(struct ext2_group_desc);

	disk_write(hd->device, s_part+offset, (char *) gd, gd_size);
}

//cree une inode
int ext2_create_inode(struct disk *disk, int mode)
{
	int i,j,group=-1,index;
	u32 *idInode;
	printk("DEBUG : groups : %d\n",disk->groups);

	for(i=0;i<=disk->groups;i++)
	{
		idInode=(u32*)disk->gd[i].bg_inode_bitmap;
		for(j=0;j<disk->sb->s_inodes_per_group;j++)
			if(idInode[j]==1&&group==-1)
			{group=i; index=j;}
	}
	if(group==-1)
		return -1;
	printk("DEBUG : group %d has %d free inode(s)\n",group,disk->gd[group].bg_free_inodes_count);
	printk("DEBUG : Free inode found on block %d, inode number %d\n",group,index);
	return (-1);
	idInode=(u32*)disk->gd[group].bg_inode_bitmap;
	//idInode[index]=0;
	//ecriture de la gd sur le disque
	//ext2_write_gd(disk,disk->gd,disk->part->s_lba * 512);

	int offset = disk->gd[group].bg_inode_table * disk->blocksize + index * disk->sb->s_inode_size;
	//printk("offset : %d\n",offset);

	//creation de l'inode
	struct ext2_inode * in=(struct ext2_inode*)kmalloc(sizeof(struct ext2_inode));
	in->i_mode=mode;
	in->i_size=1;

	//ecriture de l'inode sur le disque
	disk_write(disk->device, disk->part->s_lba*512+offset, (char *) in, disk->sb->s_inode_size);

	kfree(in);

	int inum=group*disk->sb->s_inodes_per_group+1;
	return inum;

}
/* retourne la structure d'inode a partir de son numero */
struct ext2_inode *ext2_read_inode(struct disk *hd, int i_num)
{
	int gr_num, index, offset;
	struct ext2_inode *inode;

	inode = (struct ext2_inode *) kmalloc(sizeof(struct ext2_inode));

	/* groupe qui contient l'inode */
	gr_num = (i_num - 1) / hd->sb->s_inodes_per_group;

	/* index de l'inode dans le groupe */
	index = (i_num - 1) % hd->sb->s_inodes_per_group;

	/* offset de l'inode sur le disk */
	offset = hd->gd[gr_num].bg_inode_table * hd->blocksize + index * hd->sb->s_inode_size;

	/* lecture */
	disk_read(hd->device, (hd->part->s_lba * 512) + offset, (char *) inode, hd->sb->s_inode_size);

	return inode;
}

char *ext2_read_file(struct disk *hd, struct ext2_inode *inode)
{
	char *mmap_base, *mmap_head, *buf;

	int *p, *pp, *ppp;
	int i, j, k;
	int n, size;

	buf = (char *) kmalloc(hd->blocksize);
	p = (int *) kmalloc(hd->blocksize);
	pp = (int *) kmalloc(hd->blocksize);
	ppp = (int *) kmalloc(hd->blocksize);

	/* taille totale du fichier */
	size = inode->i_size;
	mmap_head = mmap_base = kmalloc(size);

	/* direct block number */
	for (i = 0; i < 12 && inode->i_block[i]; i++) {
		disk_read(hd->device, (hd->part->s_lba * 512) + inode->i_block[i] * hd->blocksize, buf, hd->blocksize);

		n = ((size > hd->blocksize) ? hd->blocksize : size);
		memcpy(mmap_head, buf, n);
		mmap_head += n;
		size -= n;
	}

	/* indirect block number */
	if (inode->i_block[12]) {
		disk_read(hd->device, (hd->part->s_lba * 512) + inode->i_block[12] * hd->blocksize, (char *) p, hd->blocksize);

		for (i = 0; i < hd->blocksize / 4 && p[i]; i++) {
			disk_read(hd->device, (hd->part->s_lba * 512) + p[i] * hd->blocksize, buf, hd->blocksize);

			n = ((size > hd->blocksize) ? hd->blocksize : size);
			memcpy(mmap_head, buf, n);
			mmap_head += n;
			size -= n;
		}
	}

	/* bi-indirect block number */
	if (inode->i_block[13]) {
		disk_read(hd->device, (hd->part->s_lba * 512) + inode->i_block[13] * hd->blocksize, (char *) p, hd->blocksize);

		for (i = 0; i < hd->blocksize / 4 && p[i]; i++) {
			disk_read(hd->device, (hd->part->s_lba * 512) + p[i] * hd->blocksize, (char *) pp, hd->blocksize);

			for (j = 0; j < hd->blocksize / 4 && pp[j]; j++) {
				disk_read(hd->device, (hd->part->s_lba * 512) + pp[j] * hd->blocksize, buf, hd->blocksize);

				n = ((size > hd-> blocksize) ? hd->blocksize : size);
				memcpy(mmap_head, buf, n);
				mmap_head += n;
				size -= n;
			}
		}
	}

	/* tri-indirect block number */
	if (inode->i_block[14]) {
		disk_read(hd->device, (hd->part->s_lba * 512) + inode->i_block[14] * hd->blocksize, (char *) p, hd->blocksize);

		for (i = 0; i < hd->blocksize / 4 && p[i]; i++) {
			disk_read(hd->device, (hd->part->s_lba * 512) + p[i] * hd->blocksize, (char *) pp, hd->blocksize);

			for (j = 0; j < hd->blocksize / 4 && pp[j]; j++) {
				disk_read(hd->device, (hd->part->s_lba * 512) + pp[j] * hd->blocksize, (char *) ppp, hd->blocksize);

				for (k = 0; k < hd->blocksize / 4 && ppp[k]; k++) {
					disk_read(hd->device, (hd->part->s_lba * 512) + ppp[k] * hd->blocksize, buf, hd->blocksize);

					n = ((size > hd->blocksize) ? hd->blocksize : size);
					memcpy(mmap_head, buf, n);
					mmap_head += n;
					size -= n;
				}
			}
		}
	}

	kfree(buf);
	kfree(p);
	kfree(pp);
	kfree(ppp);

	return mmap_base;
}
