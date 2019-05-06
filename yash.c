#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>

#define MAX_LINE_SIZE 2000 // used to add restriction to user input

// *** main() ***
// continuously handles user commands until the shell is exited
int main(int argc, char const *argv[]) {

	char *command;
	int status = 1;
	// process user commands until the shell is exited
	do {
		printf("# ");               		// print the prompt
        command = malloc(sizeof(char)*MAX_LINE_SIZE);
		command = fgets(command, MAX_LINE_SIZE, STDIN_FILENO);
		free(command);					
	} while (status);

	return EXIT_SUCCESS;
}
