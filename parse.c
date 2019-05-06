#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
#include "parse.h"

job_t *new_job()
{
    job_t *job = (job_t *)calloc(1, sizeof(job));
    job->input = -1;
    job->output = -1;
    job->error = -1;
    job->is_bg = false;
    return job;
}

void free_job(job_t *job)
{
    if (!job)
        return;
    if (job->input != STDIN)
        close(job->input);
    if (job->output != STDOUT)
        close(job->output);
    free(job->command);
    if (job->num_args > 0)
    {
        for (int i = 0; i < job->num_args; ++i)
            free(job->args[i]);
        free(job->args);
    }
    free(job);
}

char *alloc_and_copy_str(char *str)
{
    size_t len = strlen(str);
    char *new_str = (char *)calloc(len + 1, sizeof(char));
    return strcpy(new_str, str);
}

// Restrictions on the input
// *************************
// 1. Everything that can be a token(<, >, 2>, etc.) will have a space before
// and after it. Also, any redirections will follow the command after all its args.
// 2. If a command has a &symbol(indicating that it will be run in the background),
// then the &symbol will always be the last token in the line.
// 3. Each line contains one command or two commands in one pipeline
// 4. Lines will not exceed 2000 characters
// 5. All characters will be ASCII
// 6. Ctrl - d will exit the shell

// *** tokenize_command() ***
// tokenizes the command string and stores the tokens in the tokens array
// returns the total number of tokens
int tokenize_command(char *command, char **tokens)
{
    // initialize token array
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

// checks if last token is invalid (<, >, 2>, |)
bool is_valid_command(char **tokens, int num)
{
    char *last = tokens[num - 1];
    if ((strcmp(last, ">") == 0) ||
        (strcmp(last, "2>") == 0) ||
        (strcmp(last, "<") == 0) ||
        (strcmp(last, "|") == 0))
    {
        return false;
    }
    return true;
}
