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

int main(int argc, char const *argv[])
{

	while (1)
	{
		char *buffer = calloc(1, sizeof(char) * (MAX_LINE_SIZE + 1)); // holds user input
		char **tokens;
		int argc;
		printf("# ");									 // print prompt
		char *ptr = fgets(buffer, MAX_LINE_SIZE, stdin); // get user input in buffer
		if (ptr == NULL)
			exit(1); // user input error

		// null terminate user input
		int len = strlen(buffer);
		if (buffer[len - 1] == '\n')
			buffer[len - 1] = '\0'; // this should always be true
		// tokenize user input
		int num_tokens = tokenize_command((const char *)buffer, tokens);
		for (int i = 0; i < num_tokens; i++)
		{
			if (DEBUG)
			{
				printf("%d: %s/n", i, tokens[i]);
			}
		}
		free(buffer);
		free(tokens);
	}
}
