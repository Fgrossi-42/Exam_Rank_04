#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

//semplice funzione di errore o messaggio, stampa sempre s, e s2 solo se esiste, segue con un \n
int err(char *s, char *s2)
{
	while(*s)
	write(2, s++, 1);
	if(s2)
		while(*s2)
		write(2, s2++, 1);
	write(2, "\n", 1);
	return (1);
}

//funzione di esecuzione dei comandi
int exec(char **argv, int i, int tmp, char **env)
{
	argv[i] = NULL;
	dup2(tmp, 0);
	close(tmp);
	execve(argv[0], argv, env);
	return(err("error: cannot execute ", argv[0]));
}

int main (int argc, char **argv, char **env)
{
	int i = 0;
	int pid = 0;
	int tmp = dup(0);
	int fd[2];
	(void)argc;

	while(argv[i] && argv[i + 1])
	{
		//avanza comandi fino al prossimo | o ;, utile solo dopo la prima iterazione della while
		argv = &argv[i + 1];
		i = 0;

		//porta avanti fino al prossimo | o ;
		while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
		i++;

		//caso speciale per il comando cd
		if(!strcmp(argv[0], "cd"))
		{
			if(i != 2)
			err("error: cd: bad arguments", NULL);
			else if (chdir(argv[1]) != 0)
				err("error: cd: cannot change directory to ", argv[1]);
		}
		//caso ;
		else if(i != 0 && (argv[i] == NULL || !strcmp(argv[i], ";")))
		{
			pid = fork();
			if(!pid)
			{
				if (exec(argv, i, tmp, env))
				return (1);
			}
			else
			{
				close(tmp);
				while(waitpid(-1, NULL, WUNTRACED) != -1)
				;
				tmp = dup(0);
			}
		}
		//caso |
		else if(i != 0 && !strcmp(argv[i], "|"))
		{
			pipe(fd);
			pid = fork();
			if(!pid)
			{
				dup2(fd[1], 1);
				close(fd[1]);
				close(fd[0]);
				if (exec(argv, i, tmp, env))
					return (1);
			}
			else
			{
				close(tmp);
				close(fd[1]);
				tmp = fd[0];
			}
		}
	}
	return (0);
}