#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#define CHILD 4

void interrupt(int sig)
{
	(void)sig;
	const char msg[]= "\nCtrl+C pressed! Program interrupted.\n";
	write(STDOUT_FILENO, msg, sizeof(msg) - 1);
	_exit(0);
}

int main()
{
	int pipes[CHILD][2];
	pid_t pid;
	char message[256];

	signal(SIGINT, interrupt);

	for(int i = 0; i < CHILD; i++){
		if(pipe(pipes[i]) == -1){
			perror("Pipe error");
			return 1;
		}
	}

	for (int i = 0; i < CHILD; i++){
		pid = fork();

		if ( pid == -1){
			perror("Fork error");
			return 1;
		}

		if (pid == 0){
			signal(SIGINT, SIG_IGN);
			
			for(int j = 0; j < CHILD; j++){
				close(pipes[j][1]);
				
				if(j != i)
					close(pipes[j][0]);
			}

			char received[256] = {0};
			ssize_t n = read(pipes[i][0], received, sizeof(received) - 1);

			if(n > 0){
				received[n] = '\0';
				printf("[Child %d] Received: %s", i + 1, received);
				fflush(stdout);
			}

			close(pipes[i][0]);
			exit(0);
	
		}
	}
	
	for (int i = 0; i < CHILD; i++)
		close(pipes[i][0]);

	printf("==== MESSAGE SYSTEM ====\n");
	printf("Enter a Message:");
	fflush(stdout);

	if(fgets(message, sizeof(message), stdin) != NULL){
		for(int i = 0; i < CHILD; i++){
			write(pipes[i][1], message, strlen(message) + 1);
			close(pipes[i][1]);
		}
	}else{
		for(int i = 0; i < CHILD; i++)
			close(pipes[i][1]);
	}

	for (int i = 0; i< CHILD; i++)
		wait(NULL);

	printf("[Parent] All messages has been sent successfully !.\n");

	return 0;
}
