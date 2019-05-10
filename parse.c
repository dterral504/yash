#include "parse.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"

// *** tokenize_command() ***
// tokenizes the command string and stores the tokens in the tokens array
// returns the total number of tokens
int tokenize_command(const char *command, char **tokens)
{
    // initialize token array
    int max_tokens = MAX_TOKENS; // initial size of token array

    tokens = calloc(1, sizeof(char *) * max_tokens); // allocate memory for token array
    if (!tokens)
    { // check to make sure memory was succesfully allocated
        fprintf(stderr, "Exit: allocation error\n");
        exit(EXIT_FAILURE);
    }

    // create copy of user input for strtok_r
    char *cmd_copy = calloc(1, sizeof(char) * (strlen(command) + 1));
    strcpy(cmd_copy, command);

    // initialize variables for strtok_r
    int i = 0;                     // index into token array
    char *next;                    // holds each token - used for strtok_r
    char *ptr = NULL;              // save ptr for strtok_r
    const char *delimeter = " \n"; // delimeters for strtok_r

    // parse user input
    for (next = strtok_r(cmd_copy, delimeter, &ptr);
         next != NULL;
         next = strtok_r(NULL, delimeter, &ptr))
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
    free(cmd_copy);
    tokens[i] = NULL;
    return i;
}