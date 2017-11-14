#include <stdio.h>

#include <signal.h>
#include <bits/sigaction.h> // error occurred unknown storage size
#include <sys/wait.h>

#include "commands.h"
#include "signal_handlers.h"
void catch_sigint(int signalNo)
{
    printf("\n");
}

void catch_sigtstp(int signalNo)
{
  // TODO: File this!
  printf("\n");
}

void catch_sigchld(int signalNo)
{
	int status, pid;
	while((pid = wait(&status)) != -1)
	{
		if(WIFEXITED(status) && WEXITSTATUS(status) != (unsigned char)-1)
		{
			if(pid == bg_pid)
			{
				fflush(stdout);
				printf("%d done ", bg_pid);
				for(int i = 0; i < bg_argc; i++)
				{
					printf("%s ", bg_argv[i]);
				}
				printf("\n");
			}
		}
	}
}

void init_sig()
{
    struct sigaction sigint_action;
	sigint_action.sa_handler = catch_sigint;
	sigint_action.sa_flags = 0;
    sigemptyset(&sigint_action.sa_mask);
    sigaction(SIGINT, &sigint_action, 0);

    struct sigaction sigtstp_action;
	sigtstp_action.sa_handler = catch_sigtstp;
	sigtstp_action.sa_flags = 0;
    sigemptyset(&sigtstp_action.sa_mask);
    sigaction(SIGTSTP, &sigtstp_action, 0);

	struct sigaction sigchld_action;
	sigchld_action.sa_handler = catch_sigchld;
	sigchld_action.sa_flags = 0;
    sigemptyset(&sigchld_action.sa_mask);
    sigaction(SIGCHLD, &sigchld_action, 0);

	//signal(SIGTSTP, catch_sigtstp);
	//signal(SIGINT, catch_sigint);
}
