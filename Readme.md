# Michael Carlucci (mjc548) & Daniel Bezden (dvb27)

## Functions
**interrupt()**

This function prints a new line if there are no child processes. Without this function, the shell would be terminated which is incorrect.

**split()**

This function splits the passed in string by the delimiter using strtok(). It then uses strcpy() to avoid corrupting the passed in string.

## Code Overview
Our program works by first setting the signal handler, then mallocing and initializing an array of process ids. Once these two things are done, the program enters an infinite loop. On each iteration of the loop, a prompt is printed and the program waits for user input. Once input is obtained, the program checks if it is “exit”. If it is, then the program returns 0, and if it is not it continues on. The program then splits the input into a list of commands separated by ‘;’. We then loop through each command. In this second loop, we use strcpy() to copy the command to a new string and split this string using strtok() along any ‘>’ signs and ‘|’ signs. Any string containing ‘|’ is stored in the array sub_cmds. The program then finds the length of sub_cmds. It then makes a pipe array if pipes are found in  the commands. Next, a new loop is entered where each command in the pipe chain is run. We use dup2 to run the pipe and redirect functions within this portion of the program. The program first pipes in from the previous command if there was one, then pipes to the next command if there is one. Finally, if a redirect is present, a redirect is used on the final command of the string. Once this loop is finished, another loop starts where the program waits on each sub-command and closes pipes.

## Difficulties Encountered
The most difficult part of this project was trying to keep track of many pipes and knowing when and how to close them. This caused many issues and made up the bulk of our debugging time. However, we eventually figured out how everything should work and have now passed all the test cases we have tried involving piping.
