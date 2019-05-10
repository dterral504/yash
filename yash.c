#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "parse.h"
#include "debug.h"
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>

#define MAX_LINE_SIZE 2000 // used to add restriction to user input
#define MAX_TOKENS 50	  // defines initial size of allocated memory for token array

int main(int argc, char const *argv[])
{

	while (1)
	{
		char *buffer = calloc(1, sizeof(char) * (MAX_LINE_SIZE + 1)); // holds user input

		printf("# ");									 // print prompt
		char *ptr = fgets(buffer, MAX_LINE_SIZE, stdin); // get user input in buffer
		if (ptr == NULL)
			exit(1); // user input error

		// null terminate user input
		int len = strlen(buffer);
		if (buffer[len - 1] == '\n')
			buffer[len - 1] = '\0'; // this should always be true

		// tokenize user input
		// int num_tokens = tokenize_command((const char *)buffer, tokens);
		// *************
		// initialize token array
		int max_tokens = MAX_TOKENS;							// initial size of token array
		char **tokens = calloc(1, sizeof(char *) * max_tokens); // allocate memory for token array
		if (!tokens)
		{ // check to make sure memory was succesfully allocated
			fprintf(stderr, "Exit: allocation error\n");
			exit(EXIT_FAILURE);
		}

		// create copy of user input for strtok_r
		char *cmd_copy = calloc(1, sizeof(char) * (strlen(buffer) + 1));
		strcpy(cmd_copy, buffer);

		// initialize variables for strtok_r
		int i = 0;					   // arg count
		char *next;					   // holds each token - used for strtok_r
		char *save_ptr = NULL;		   // save ptr for strtok_r
		const char *delimeter = " \n"; // delimeters for strtok_r

		// tokenize user input
		for (next = strtok_r(cmd_copy, delimeter, &save_ptr);
			 next != NULL;
			 next = strtok_r(NULL, delimeter, &save_ptr))
		{
			tokens[i] = next;
			i++;

			// allocate more space if neccessary
			if (i >= max_tokens)
			{
				max_tokens += MAX_TOKENS;
				tokens = realloc(tokens, sizeof(char *) * max_tokens);
				if (!tokens)
				{ // check to make sure memory was succesfully allocated
					fprintf(stderr, "Exit: allocation error\n");
					exit(EXIT_FAILURE);
				}
			}
			if (DEBUG)
			{
				printf("next: %s\n", next);
			}
		}
		tokens[i] = NULL; // null terminate tokens array
		// *************
		free(cmd_copy);
		free(buffer);
		free(tokens);
	}
}
