
void sys_exit(int);
int sys_open(char*);
char* sys_sbrk(int);
int sys_exec(char *, char **);
int sys_console_read(char*);
int sys_wait(int*);
void sys_sigreturn(void);
