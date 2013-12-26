
int bl_read(int, int, int, char *);	/* drive, block, count, buf */
int bl_write(int, int, int, char *);

int disk_read(int, int, char *, int);	/* drive, offset, buf, count */ //A utiliser
int disk_write(int, int, char*, int); //A utiliser
