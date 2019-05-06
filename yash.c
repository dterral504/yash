#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
#include "parse.h"
#include "yash.h"

#define MAX_LINE_SIZE 2000 // used to add restriction to user input

int execute(job_t *jobs[], int *num_jobs)
{

    int fd[2];
    pid_t cpid;

    if (*num_jobs > 1)
    {
        // Setup pipe
        fd[0] = jobs[0]->output;
        fd[1] = jobs[1]->input;

        if (pipe(fd) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        cpid = fork();

        if (cpid < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (cpid > 0)
        {

            execvp(jobs[0]->args[0], jobs[0]->args);
        }

        if (cpid == 0)
        {                 /* Child reads from pipe */
            close(fd[1]); /* Closes unused write end */
        }
    }
    else
    {
        execvp(jobs[0]->args[0], jobs[0]->args);
    }
    return 1;
}

char *get_user_input(void)
{
    char *command = malloc(sizeof(char) * MAX_LINE_SIZE); // allocate memory for user input

    if (!command)
    { // check to make sure memory was succesfully allocated
        fprintf(stderr, "Exit: allocation error\n");
        exit(EXIT_FAILURE);
    }
    command = fgets(command, MAX_LINE_SIZE, STDIN_FILENO);
    return command;
}

// *** main() ***
// continuously handles user commands until the shell is exited
int main(int argc, char const *argv[])
{
    char *command;  // initialize variable to hold user input
    char **tokens;  // initialize array to hold tokens
    int num_tokens; // initialize variable to count # of tokens
    job_t *jobs[2];

    int status = 1; // holds the return status of execute() - used in do-while loop
    do
    {
        printf("# ");                                   // print the prompt
        command = get_user_input();                     // get input from the user
        num_tokens = tokenize_command(command, tokens); // tokenize the user input & get total # of tokens
        printf("%s: argc=%d", command, num_tokens);

    } while (status);

    return EXIT_SUCCESS;
}