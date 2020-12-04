#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

int pid1 = 0;
int pid2 = 0;

void interrupt(int signum) {
	if(pid1) {
		kill(pid1, SIGTERM);
		printf("\n");
	} else if(pid2) {
		kill(pid2, SIGTERM);
		printf("\n");
	} else {
		printf("\n%s $ ", getcwd(NULL, 0));
		fflush(stdout);
	}
}

int main(int argc, char **argv) {
	signal(SIGINT, interrupt);

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
		char *cmd = strtok(string, ";");
		char **cmds = malloc(sizeof(char *) * 1024);
		
		int i;
		for(i = 0; cmd != NULL; i++) {
			cmds[i] = cmd;
			cmd = strtok(NULL, ";");
		}
		
		// loop through each command
		cmd = malloc(1024);
		for(i = 0; cmds[i] != NULL; i++) {
			strcpy(cmd, cmds[i]);
			
			if(strstr(cmd, "|") != NULL) {
				char *half1 = strtok(cmd, "|");
				char *half2 = strtok(NULL, ">");
				char *file = strtok(NULL, ">");

				int fd[2];
				pipe(fd);
				
				char *token = strtok(half1, " ");
				if(token == NULL) continue;
				
				char **args1 = malloc(sizeof(char *) * 1024);
				
				int j;
				for(j = 0; token != NULL; j++) {
					args1[j] = token;
					token = strtok(NULL, " ");
				}
				
				token = strtok(half2, " ");
				if(token == NULL) continue;
				
				char **args2 = malloc(sizeof(char *) * 1024);
				
				for(j = 0; token != NULL; j++) {
					args2[j] = token;
					token = strtok(NULL, " ");
				}
				
				pid1 = fork();
				if(pid1 == 0) {
					dup2(fd[1], STDOUT_FILENO);
					// pipe output and run
					close(fd[0]);
					
					if(execvp(args1[0], args1) == -1) {
						perror(string);
						return 0;
					}
				} else {
					pid2 = fork();
					if(pid2 == 0) {
						// pipe input and run
						dup2(fd[0], STDIN_FILENO);
						close(fd[1]);
						
						int fd1 = 0;
						if(file != NULL){
							while(isspace((unsigned char)*file)) file++;
							if(strstr(cmds[i], ">>") == NULL){
								fd1 = open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
							}else{
								fd1 = open(file, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
							}
							dup2(fd1, STDOUT_FILENO);
						}
						if(execvp(args2[0], args2) == -1) {
							perror(string);
							return 0;
						}
					} else {				
						waitpid(pid1, NULL, 0);
						pid1 = 0;
						
						close(fd[1]);
					
						waitpid(pid2, NULL, 0);
						pid2 = 0;
					}
				}
			} else {
				char *half1 = strtok(cmd, ">");
				char *file = strtok(NULL, ">");
				char *token = strtok(half1, " ");
				if(token == NULL) continue;
				
				char **args = malloc(sizeof(char *) * 1024);
				
				int j;
				for(j = 0; token != NULL; j++) {
					args[j] = token;
					token = strtok(NULL, " ");
				}
				
				if(!strcmp(args[0], "cd")) {
					if(args[1] == NULL) {
						chdir(getenv("HOME"));
					} else if(chdir(args[1]) == -1) {
							perror(args[1]);
					}
				} else {
					pid1 = fork();
					if(pid1 == 0) {
						int fd1 = 0;
						if(file != NULL){
							while(isspace((unsigned char)*file)) file++;
							if(strstr(cmds[i], ">>") == NULL){
								fd1 = open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
							}else{
								fd1 = open(file, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
							}
							dup2(fd1, STDOUT_FILENO);
						}
						if(execvp(args[0], args) == -1) {
							perror(string);
							return 0;
						}
					} else {
						wait(NULL);
						pid1 = 0;
					}
				}
			}
		}
	}

	return 0;
}
