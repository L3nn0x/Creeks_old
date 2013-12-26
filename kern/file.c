#include "lib.h"
#include "list.h"
#include "kmalloc.h"
#include "file.h"
#include "process.h"

//add_file : ajoute un fichier/un repertoire
void add_file(struct disk *disk, char* path, int type)
{
	char name[512], parent[512];
	int taille=strlen(path),i=taille,j;
	while(i!=0&&path[i]!='/')
		i--;
	i++;
	memcpy(name,&path[i],taille-i);
	printk("name : %s\n",name);
	memcpy(parent,path,i);
	printk("parent : %s\n",parent);

	struct file *fp;
	fp=(struct file*) kmalloc(sizeof(struct file));
	fp->name=(char*)kmalloc((taille-i)*sizeof(char));
	strcpy(fp->name,name);
	fp->disk=disk;
	if(type==REP)
		fp->inum=ext2_create_inode(disk,EXT2_S_IFREG);
	else
		fp->inum=ext2_create_inode(disk,EXT2_S_IFDIR);
	printk("inum : %d\n",fp->inum);
	if(fp->inum==-1)
	{
		kfree(fp->name);
		kfree(fp);
		printk("Error creating the file\n");
		return;
	}
	fp->inode=ext2_read_inode(disk, fp->inum);
	fp->mmap=0;
	fp->parent=path_to_file(parent);
	printk("parent : %s\n",fp->parent->name);

	list_add(&fp->sibling, &fp->parent->leaf);
	if(type==REP)
	{
		INIT_LIST_HEAD(&fp->leaf);
		INIT_LIST_HEAD(&fp->sibling);
	}
	get_dir_entries(fp->parent);
}


/*
 * init_root(): initialise la structure de fichier decrivant la racine.
 */
struct file *init_root(struct disk *disk)
{
	struct file *fp;

	fp = (struct file *) kmalloc(sizeof(struct file));

	fp->name = (char *) kmalloc(sizeof("/"));
	strcpy(fp->name, "/");

	fp->disk = disk;
	fp->inum = EXT2_INUM_ROOT;
	fp->inode = ext2_read_inode(disk, fp->inum);
	fp->mmap = 0;
	fp->parent = fp;

	INIT_LIST_HEAD(&fp->leaf);
	get_dir_entries(fp);

	INIT_LIST_HEAD(&fp->sibling);

	return fp;
}

/* is_directory(): renvoit 1 si le fichier en argument est un repertoire */
int is_directory(struct file *fp)
{
	if (!fp->inode)
		fp->inode = ext2_read_inode(fp->disk, fp->inum);

	return (fp->inode->i_mode & EXT2_S_IFDIR) ? 1 : 0;
}

/*
 * is_cached_leaf(): dans un repertoire passe en argument, recherche
 * le fichier en parametre parmi les fichiers feuilles presents dans le cache.
 */
struct file *is_cached_leaf(struct file *dir, char *filename)
{
	struct file *leaf;

	//// printk("DEBUG: is_cached_leaf(): looking for %s in %s\n", filename, dir->name);

	list_for_each_entry(leaf, &dir->leaf, sibling){
		if (strcmp(leaf->name, filename) == 0)
			return leaf;
	}

	return (struct file *) 0;
}

/*
 * get_dir_entries(): met a jour la liste des sous-repertoires du repertoire
 * passe en argument.
 */
int get_dir_entries(struct file *dir)
{
	struct directory_entry *dentry;
	struct file *leaf;
	u32 dsize;
	char *filename;
	int f_toclose;

	//// printk("DEBUG: get_dir_entries(): look for %s leafs\n", dir->name);

	/* Le repertoire est-il dans le cache ? */
	if (!dir->inode)
		dir->inode = ext2_read_inode(dir->disk, dir->inum);

	/*
	 * Est-ce que le fichier est bien un repertoire ?
	 * FIXME: on ne gere pas les liens symboliques
	 */
	if (!is_directory(dir)) {
		printk("get_dir_entries() error: %s is not a directory\n", dir->name);
		return -1;
	}

	/*
	 * Si il n'est pas deja ouvert, on ouvre le repertoire pour le lire. Le
	 * flag f_toclose indique si il faut fermer le fichier apres lecture.
	 */
	if (!dir->mmap) {
		dir->mmap = ext2_read_file(dir->disk, dir->inode);
		f_toclose = 1;
	} else {
		f_toclose = 0;
	}

	/*
	 * Lecture de chaque entree du repertoire et creation de la structure
	 * correspondante si elle n'est pas dans le cache
	 */
	dsize = dir->inode->i_size;			/* taille du repertoire */
	dentry = (struct directory_entry *) dir->mmap;

	while (dentry->inode && dsize) {
		filename = (char *) kmalloc(dentry->name_len + 1);
		memcpy(filename, &dentry->name, dentry->name_len);
		filename[dentry->name_len] = 0;

		//// printk("DEBUG: get_dir_entries(): reading inode %d: %s\n", dentry->inode, filename);

		/*
		 * La feuille est ajoutee dans le cache de fichier (a moins qu'elle y soit deja).
		 * Note : les entrees '..' et '.' ne sont pas prises en compte.
		 */
		if (strcmp(".", filename) && strcmp("..", filename)) {

			if (!(leaf = is_cached_leaf(dir, filename))) {

				//// printk("DEBUG: get_dir_entries(): create file struct for %s in cache\n", filename);

				leaf = (struct file *) kmalloc(sizeof(struct file));
				leaf->name = (char *) kmalloc(dentry->name_len + 1);
				strcpy(leaf->name, filename);

				leaf->disk = dir->disk;
				leaf->inum = dentry->inode;
				leaf->inode = 0;
				leaf->mmap = 0;
				leaf->parent = dir;
				INIT_LIST_HEAD(&leaf->leaf);
				list_add(&leaf->sibling, &dir->leaf);
			}
		}

		kfree(filename);

		/* Lecture de l'entree suivante */
		dsize -= dentry->rec_len;
		dentry = (struct directory_entry *) ((char *) dentry + dentry->rec_len);
	}

	if (f_toclose == 1) {
		kfree(dir->mmap);
		dir->mmap = 0;
	}

	return 0;
}

/*
 * path_to_file(): renvoit la structure de fichier, dans le cache, associee au
 * chemin en argument.
 */
struct file *path_to_file(char *path)
{
	char *name, *beg_p, *end_p;
	struct file *fp;

	//// printk("DEBUG: path_to_file(): searching file struct for %s\n", path);

	/* Chemin relatif ou absolu */
	if (path[0] != '/')
		fp = current->pwd;
	 else
		fp = f_root;

	/*
	 * Analyse du chemin.
	 * Note :
	 * -  beg_p pointe sur le debut de mot
	 * -  end_p pointe sur la fin
	 */
	beg_p = path;
	while (*beg_p == '/')
		beg_p++;
	end_p = beg_p + 1;

	while (*beg_p != 0) {
		/* Est-ce que le fichier courant est bien un repertoire */
		if (!fp->inode)
			fp->inode = ext2_read_inode(fp->disk, fp->inum);

		if (!is_directory(fp))
			return (struct file *) 0;

		/* Extraction du nom du sous-repertoire */
		while (*end_p != 0 && *end_p != '/')
			end_p++;
		name = (char *) kmalloc(end_p - beg_p + 1);
		memcpy(name, beg_p, end_p - beg_p);
		name[end_p - beg_p] = 0;

		if (strcmp("..", name) == 0) {		/* '..' */
			fp = fp->parent;
		} else if (strcmp(".", name) == 0) {	/* '.' */
			/* nop */
		} else {
			get_dir_entries(fp);
			if (!(fp = is_cached_leaf(fp, name))) {
				kfree(name);
				return (struct file *) 0;
			}
		}

		beg_p = end_p;
		while (*beg_p == '/')
			beg_p++;
		end_p = beg_p + 1;

		kfree(name);
	}

	return fp;
}
