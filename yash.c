#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "parse.h"
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>

#define MAX_LINE_SIZE 2000 // used to add restriction to user input

// returns the user command as a single line of input
char *get_user_input(void)
{
	char *command = malloc(sizeof(char) * (MAX_LINE_SIZE + 1)); // allocate memory for command
	// int i = 0;											  // initialize index to 0
	// int next;											  // will hold next character of user input
	
	if (!command){                                        // check to make sure memory was succesfully allocated
		fprintf(stderr, "Exit: allocation error\n");
		exit(EXIT_FAILURE);
	}
    char* ptr = fgets(command, MAX_LINE_SIZE, stdin);
    return command;

	// while (true){
	// 	next = getchar(); // get next character of input
	// 	// if next character is end of the command, null terminate the command and return it
	// 	if (next == '\n' || next == EOF){
	// 		command[i] = '\0';
	// 		return command;
	// 	}
	// 	// next is not end of command so append it to the command array and increment i
	// 	command[i] = next;
	// 	i++;
	// }
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
        printf("tokeize");
        num_tokens = tokenize_command(command, tokens);
        
        for(int i=0; i<num_tokens; i++){
            printf("%d: %s/n", i, tokens[i]);
        }
        printf("free!");
		free(command);
        free(tokens);				
	} while (status);

	return EXIT_SUCCESS;
}


int main2(int argc, char const *argv[]) {

    char buffer[MAX_LINE_SIZE+1];
    char *ptr = fgets(buffer, MAX_LINE_SIZE, stdin);
    if (ptr == NULL) exit(1); // error
    int len = strlen(buffer);
    if (buffer[len-1] == '\n') buffer[len-1] = '\0'; // this should always be true


}