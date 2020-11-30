all: cshell.c
	gcc -g cshell.c -o cshell

clean:
	rm cshell
