#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#define CHILD 4

int pipes[CHILD][2];

//Handle Ctrl+C interruption
void handle_signal(int sig)
{
	signal(SIGINT, SIG_IGN);
	printf("\nCtrl+C pressed! Program interrupted.\n");
	fflush(stdout);
	exit(0);
}

int main()
{
	pid_t pid;
	char message[256];

	signal(SIGINT, handle_signal);

	//create pipes for communication
	for (int i = 0; i < CHILD; i++)
	{
		if(pipe(pipes[i]) == -1)
		{
			perror("Pipe error");
			return 1;
		}
	}

	//create child processes
	for(int i = 0; i < CHILD; i++)
	{
		pid = fork();

		if(pid == 0)
		{
			//children ignore Ctrl+ c
			signal(SIGINT, SIG_IGN);

			//close unused pipe ends
			for (int j = 0; j < CHILD; j++)
			{
				close(pipes[j][1]);
		
				if(j != i)
					close(pipes[j][0]);
			}

			//receive message from manager
			while (read(pipes[i][0], message, sizeof(message)) > 0)
			{
				printf("[EMPLOYEE %d | PID %d] Message: %s\n", i + 1, getpid(), message);
				fflush(stdout);
			}

			close(pipes[i][0]);
			exit(0);
		}
	}
	
	//close unused read ends in the manager
	for(int i = 0; i < CHILD; i++)
	{
		close(pipes[i][0]);
	}
	
	printf("==============================\n");
	printf("   OFFICE MESSAGE SYSTEM\n");
	printf("==============================\n");
	printf("[MANAGER | PID %d] Created %d employees.\n", getpid(), CHILD);

	printf("Type 'exit' to close the system.\n");
	printf("Press Ctrl+C to interrupt.\n\n");

	while (1)
	{
		printf(" Manager, enter a message: ");
		fflush(stdout);

		if (fgets(message, sizeof(message), stdin) == NULL)
			break;

		message[strcspn(message, "\n")] = '\0';

		if (strcmp(message, "exit") == 0)
			break;

		if(message[0] == '\0')
			continue;

		//send message to all employees
		for(int i = 0; i < CHILD; i++)
		{
			write(pipes[i][1], message, strlen(message) + 1);
		}

		sleep(1);
		printf("[MANAGER] Message sent to all employees.\n\n");
	}

	printf("\n[MANAGER] Closing the system...\n");

	//close pipes so the employees can exit
	for (int i = 0; i < CHILD; i++)
	{
		close(pipes[i][1]);
	}

	//wait for all child processes to finish
	while (wait(NULL) > 0)
	{
	}

	printf("[MANAGER] All employees have exited. System closed.\n");

	return 0;
}
