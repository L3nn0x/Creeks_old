#include "libc/libc.h"
#include "libc/syscalls.h"
#include "libc/malloc.h"
#include "dir.h"

//#define PROMPT	"myShell> "

void sigtrap(void)
{
	console_write("signal trapped\n",GREEN);
}

int main(void)
{
	struct directory_entry *dir;
	char buf[512], out[512]; 
	char *beg_p, *end_p, **av;
	int fd, count, ac;
	int status, pid;
	int i, num;

	sigaction(10, &sigtrap);

	num = 0;
	while(1) {
		num++;
		pwd(buf);
		sprintf(out,"MyShell %s >", buf);
		console_write(out,GREEN);
		// console_write(PROMPT);

		for(i=0;i<512;i++)
			buf[i]=0;

		console_read(buf);

		beg_p = buf;
		while (*beg_p == ' ' || *beg_p == '\t')		/* enleve les blancs */
			beg_p++;

		/* exit */
		if (strncmp("exit", beg_p, 4) == 0) {
			exit(0);
		}

		/* ls */
		else if (strncmp("ls", beg_p, 2) == 0) {
			fd = open(".");
			count = read(fd, buf, sizeof(buf));
			dir = (struct directory_entry*) buf;

			while(count>0 && dir->inode) {
				memcpy(out, &dir->name, dir->name_len);
				out[dir->name_len] = 0;
				
				if(strncmp(".",out,1)&&strncmp("..",out,2))
				{
					if(dir->type==DT_FIFO)
						console_write(out,PURPLE);
					else
						console_write(out,ORANGE);
					console_write("    ",GREEN);
				}

				count -= dir->rec_len;
				dir = (struct directory_entry*) ((char*) dir + dir->rec_len);
			}
			close(fd);
			console_write("\n",GREEN);
		}

		/* cd */
		else if (strncmp("cd", beg_p, 2) == 0) {
			beg_p += 2;
			while (*beg_p == ' ' || *beg_p == '\t') 
				beg_p++;

			end_p = beg_p;
	       		while (*end_p && *end_p != '\n' && *end_p != ' ' && *end_p != '\t') 
				end_p++;
			*end_p = 0;

			int ok=chdir(beg_p);
			for(i=0;i<512;i++)
				out[i]=0;
			if(ok==-1)
				sprintf(out,"Can't chdir to : %s\n",beg_p);
			else if(ok==-2)
				sprintf(out,"%s isn't a directory\n",beg_p);
			console_write(out,GREEN);
		}

		/* pwd */
		else if (strncmp("pwd", beg_p, 3) == 0) {
			pwd(buf);
			console_write(buf,GREEN);
			console_write("\n",GREEN);
		}
		else if(!strncmp("mkdir",beg_p,3)){//mkdir
			beg_p += 5;
			while (*beg_p == ' ' || *beg_p == '\t') 
				beg_p++;

			end_p = beg_p;
	       		while (*end_p && *end_p != '\n' && *end_p != ' ' && *end_p != '\t') 
				end_p++;
			*end_p = 0;

			pwd(buf);

			int ok=chdir(beg_p);
			for(i=0;i<512;i++)
				out[i]=0;

			if(!ok)
			{
				sprintf(out,"%s already exist\n",beg_p);
				console_write(out,GREEN);
				chdir(buf);
			}
			else if(ok==-2)
			{
				sprintf(out,"%s already exist and isn't a directory\n",beg_p);
				console_write(out,GREEN);
			}else
				create_dir(beg_p);
		}

		/* help */
		else if (strncmp("help", beg_p, 4) == 0) {
			console_write("usage:\n\tcd\n\texit\n\tls\n\tpwd\n\tmkdir\n",GREEN);
		}

		/* exec command */
		else {
			/* Comptage du nombre de parametres */
			ac = 1;
			end_p = beg_p;
       			while (*end_p && *end_p != '\n') {
		       		while (*end_p && *end_p != '\n' && *end_p != ' ' && *end_p != '\t') 
					end_p++;
				ac++;
				while (*end_p == ' ' || *end_p == '\t') 
					end_p++;
			}

			if (ac>1) {
				av = (char**) malloc(sizeof(char*) * ac);

				beg_p = end_p = beg_p;
				for(i=0 ; i<(ac-1) ; i++) {
					while (*end_p == ' ' || *end_p == '\t')
						end_p++;
					beg_p = end_p;
	
       					while (*end_p && *end_p != '\n' && *end_p != ' ' && *end_p != '\t')
       						end_p++;
		
       					av[i] = (char*) malloc(end_p - beg_p + 1);
       					strncpy(av[i], beg_p, end_p - beg_p);
			       		av[i][end_p - beg_p] = 0;
				}
				av[i] = (char*) 0;

				if(!strncmp("cat", av[0], 3))
					av[0]="/bin/cat";
				else if(!strncmp("kill",av[0],4))
					av[0]="/bin/kill";
	
				pid = exec(av[0], av);

				if (pid > 0) {
					//sprintf(out, "shell: create child [%d]\n", pid);
					//console_write(out);
					pid = wait(&status);
					// sprintf(out, "shell: child [%d] exited with status (%d)\n", pid, status);
					// console_write(out);
				}

				for(i=0 ; i<(ac-1) ; i++) 
					free(av[i]);
				free((char*) av);
			}
		}
	}

	exit(0);

	return 0;
}

