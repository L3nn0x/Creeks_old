#include "io.h"
#include "types.h"

#ifdef __SCREEN__

#define RAMSCREEN 0xB8000	/* debut de la memoire video */
#define SIZESCREEN 0xFA0	/* 4000, nombres d'octets d'une page texte */
#define SCREENLIM 0xB8FA0

char kX = 0;			/* position courante du curseur a l'ecran */
char kY = 8;
char kattr = 0x07;		/* attributs video des caracteres a afficher */

char cursorX = 0;		/* position du curseur */
char cursorY = 8;
char cursorA = 0x40;
char cursorC = ' ';

#else

extern char kX;
extern char kY;
extern char kattr;
extern char cursorX;
extern char cursorY;
extern char cursorA;
extern char cursorC;

#endif				/* __SCREEN__ */

void scrollup(unsigned int);
void putcar(uchar);
void move_hw_cursor(u8, u8);
void hide_hw_cursor(void);
void show_cursor(void);
void hide_cursor(void);
