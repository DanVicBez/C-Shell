#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
	while(1) {
		printf("%s $ ", getcwd(NULL, 0));
		char *string = malloc(1024);
		fgets(string, 1024, stdin);
		string[strcspn(string, "\n")] = '\0';
		
		if(!strcmp(string, "exit")) {
			return 0;
		}
		
		if(fork() == 0) {
			char *args[2];
			args[0] = string;
			args[1] = NULL;
			if(execvp(string, args) == -1) {
				perror(string);
			}
		} else {
			wait(NULL);
		}
	}

	return 0;
}
