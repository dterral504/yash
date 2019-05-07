#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "parse.h"
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>

#define MAX_LINE_SIZE 2000 // used to add restriction to user input

// returns the user command as a single line of input
char *get_user_input(void)
{
	char *command = malloc(sizeof(char) * MAX_LINE_SIZE); // allocate memory for command
	int i = 0;											  // initialize index to 0
	int next;											  // will hold next character of user input
	
	if (!command){                                        // check to make sure memory was succesfully allocated
		fprintf(stderr, "Exit: allocation error\n");
		exit(EXIT_FAILURE);
	}

	while (true){
		next = getchar(); // get next character of input
		// if next character is end of the command, null terminate the command and return it
		if (next == '\n' || next == EOF){
			command[i] = '\0';
			return command;
		}
		// next is not end of command so append it to the command array and increment i
		command[i] = next;
		i++;
	}
}

// *** main() ***
// continuously handles user commands until the shell is exited
int main(int argc, char const *argv[]) {

	char *command;  // holds user input
    char **tokens;  // tokenizes user input
	int num_tokens; // num of tokens in user input
	int status = 1;
	// process user commands until the shell is exited
	do {
		printf("# ");               		// print the prompt
        command = get_user_input();
        num_tokens = tokenize_command(command, tokens);
		free(command);					
	} while (status);

	return EXIT_SUCCESS;
}
