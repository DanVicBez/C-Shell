#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

#define NUM_PIDS 128
int *pids;

// print a new line if there are no child processes
void interrupt(int signum) {
	int running = 0;

	int i;
	for(i = 0; i < NUM_PIDS; i++) {
		if(pids[i]) {
			running = 1;
		}
	}

	if(!running) {
		printf("\n%s $ ", getcwd(NULL, 0));
		fflush(stdout);
	}
}

// split cmd_orig by delimiter using strtok()
// uses strcpy() to avoid corrupting cmd_orig
char **split(char *cmd_orig, char *delimiter) {
	char *cmd = malloc(sizeof(char) * 1024);
	strcpy(cmd, cmd_orig);
	
	char *token = strtok(cmd, delimiter);
	char **tokens = malloc(sizeof(char *) * 1024);
	
	int i;
	for(i = 0; token != NULL; i++) {
		tokens[i] = token;
		token = strtok(NULL, delimiter);
	}
	
	tokens[i] = NULL;
	
	return realloc(tokens, sizeof(char *) * (i + 1));
}

int main(int argc, char **argv) {
	signal(SIGINT, interrupt);
	
	pids = malloc(sizeof(int) * NUM_PIDS);
	
	int i;
	for(i = 0; i < NUM_PIDS; i++) {
		pids[i] = 0;	
	}

	while(1) {
		// print prompt and receive user input
		printf("%s $ ", getcwd(NULL, 0));
		char *string = malloc(1024);
		fgets(string, 1024, stdin);
		string[strcspn(string, "\n")] = '\0';
		
		if(!strcmp(string, "exit")) {
			return 0;
		}
		
		// split input into list of commands separated by ;
		char **cmds = split(string, ";");
		
		// loop through each command
		char *cmd = malloc(1024);
		for(i = 0; cmds[i] != NULL; i++) {
			strcpy(cmd, cmds[i]);
			
			char *half1 = strtok(cmd, ">");
			char *half2 = strtok(NULL, ">");
			
			char **sub_cmds = split(half1, "|");
			
			// find length of sub_cmds
			int len = 0;
			while(sub_cmds[len++]);
			len--;
			
			int **pipes;
			if(len > 1) {	
				pipes = malloc(sizeof(int *) * (len - 1));
			}
			
			// run each command in the pipe chain
			int j;
			for(j = 0; j < len; j++) {
				char *sub_cmd = sub_cmds[j];
				char **args = split(sub_cmd, " ");
				
				if(j < len - 1) {
					pipes[j] = malloc(sizeof(int) * 2);
					pipe2(pipes[j], O_CLOEXEC);
				}
				
				pids[j] = fork();
				if(pids[j] == 0) {
					// pipe in from previous command (for all but first command)
					if(j > 0) {
						dup2(pipes[j - 1][0], STDIN_FILENO);
						close(pipes[j - 1][0]);
						close(pipes[j - 1][1]);
					}
					
					// pipe to next command (for all but last command)
					if(j < len - 1) {
						dup2(pipes[j][1], STDOUT_FILENO);
						close(pipes[j][0]);
						close(pipes[j][1]);
					}
					
					// redirect last command if > or >> is present
					if(j == len - 1 && half2 != NULL) {
						// remove leading spaces
						while(isspace((unsigned char) *half2)) half2++;
					
						int fd = 0;
						if(strstr(cmds[i], ">>") == NULL) {
							fd = open(half2, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
						} else {
							fd = open(half2, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
						}
						
						dup2(fd, STDOUT_FILENO);
					}

					if(execvp(args[0], args) == -1) {
						perror(sub_cmd);
						
						if(j > 0) {
							close(pipes[j - 1][0]);
						}
						
						if(j < len - 1) {
							close(pipes[j][1]);
						}
						
						return 0;
					}
				}
			}
			
			// wait on each sub-command and close pipes
			for(j = 0; j < len; j++) {
				waitpid(pids[j], NULL, 0);
				pids[j] = 0;
				
				if(j < len - 1) {
					close(pipes[j][0]);
					close(pipes[j][1]);
				}
			}
		}
	}

	return 0;
}
