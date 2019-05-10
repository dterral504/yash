#ifndef PARSE_H
#define PARSE_H

#include <stdbool.h>

#define MAX_TOKENS 50 // defines size of allocated memory for token array
#define STDIN 0
#define STDOUT 1
#define STDERR 2

// typedef struct
// {
//     char *command;
//     int input;
//     int output;
//     int error;
//     bool is_bg;
//     int num_args;
//     char **args;
// } job_t;

// job_t *new_job();

// void free_job(job_t *job);

// char *alloc_and_copy_str(char *str);

int tokenize_command(const char *command, char **tokens);

// bool is_valid_command(char **tokens, int num);

// job_t *parse_job(char *tokens[], int num);

// bool parse_jobs(char *tokens[], int num_tokens, job_t *out[2], int *num_jobs_out);

#endif