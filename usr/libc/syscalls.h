#define GREEN 0x02
#define BLUE 0x01
#define RED 0x04
#define PURPLE 0x05
#define ORANGE 0x06
#define WHITE 0x07
void console_write(char*,int);
void exit(int) __attribute__ ((noreturn));
int open(char*);
int read(int, char*, int);
void close(int);
int console_read(char*);
int chdir(char*);
void pwd(char*);
int exec(char*, char**);
void* sbrk(int);
int wait(int*);
int kill(int, int);
int sigaction(int, void*);
void create_dir(char* path);
