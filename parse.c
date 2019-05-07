#include "parse.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// *** tokenize_command() ***
// tokenizes the command string and stores the tokens in the tokens array
// returns the total number of tokens
int tokenize_command(char *command, char **tokens)
{
    // initialize token array
    printf("tokenize");
    int array_size = MAX_TOKENS;                  // initial size of token array
    
    tokens = malloc(sizeof(char *) * array_size); // allocate memory for token array
    int i = 0;                                    // index into token array

    if (!tokens)
    { // check to make sure memory was succesfully allocated
        fprintf(stderr, "Exit: allocation error\n");
        exit(EXIT_FAILURE);
    }
    // initialize variables for strtok
    char *next;                    // used for strtok
    const char *delimeter = " \n"; // delimeters for strtok

    next = strtok(command, delimeter);
    while (next != NULL)
    { // parse tokens with strtok
        tokens[i] = next;
        i++;

        if (i >= array_size)
        { // allocate more space if neccessary
            array_size += MAX_TOKENS;
            tokens = realloc(tokens, sizeof(char *) * array_size);

            if (!tokens)
            { // check to make sure memory was succesfully allocated
                fprintf(stderr, "Exit: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        next = strtok(NULL, delimeter);
    }
    tokens[i] = NULL;
    return i;
}