#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h> 
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#include <malloc/malloc.h> // malloc/malloc.h on macs
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

//#define DEBUG 1  // uncomment to see print statements

#define MAX_LINE_LENGTH 2001
#define MAX_ARGS 128

#define PIPE "|"
#define BACKGROUND "&"

#define JOBS "jobs"
#define RUNNING "Running"
#define STOPPED "Stopped"
#define DONE "Done"

typedef struct process {
 pid_t pid;                   // ids
 pid_t pgid;

 char* status;                  // pointer to status
 struct process *next;      // pointer to next process
 struct process *parent;   // pointer to parent process

 int std_in;                    // process i/o 
 int std_out;
 int std_err;

 bool background;         // if background = true, job is in background       
 int sig;

 char* input;                // command line input
 char **argv;                // arguments
} ProcessType;

extern char** path;         // global
pid_t process_group; 

ProcessType root;
ProcessType* foreground; 

/**
 * ========================================
 * Process Functions
 * ========================================
 */

/*
 * add process to end of list
 */
void addProcess(ProcessType *rootProcess, ProcessType *thisProcess) {
    ProcessType *currentProcess = rootProcess;

    while (currentProcess->next) {
        currentProcess = currentProcess->next;
        if (currentProcess->pid == thisProcess->pid) {
            return;
        }
    }
    
    currentProcess->next = thisProcess;
    thisProcess->parent = currentProcess;
    thisProcess->next = NULL;
}

/**
 * free up this specific process 
 */
void freeProcess(ProcessType **thisProcess) {
    if (*thisProcess == NULL) { return; }   // check if process is already free

    if ((*thisProcess)->input) {
        free((*thisProcess)->input);
    }
    
    free(*thisProcess);
    *thisProcess = NULL;
}

/**
 * terminates all processes
 */
void killProcess() {
    ProcessType *thisProcess = root.next;
    while (thisProcess) {
        if (kill(thisProcess->pid, SIGKILL) < 0) {
            perror("kill");
            return;
        }

        ProcessType *currentProcess = thisProcess->next;
        freeProcess(&thisProcess);
        thisProcess = currentProcess;
    }
        
    if (foreground) {
        freeProcess(&foreground);
    }
}

/**
 * searches jobs list and removes process
 */
void removeProcess(ProcessType *parentProcess, ProcessType *thisProcess) {
    if (!thisProcess) { return; }

    ProcessType *currentProcess = parentProcess->next;
    while (currentProcess) {
        if (currentProcess->pid == thisProcess->pid) {
            if (currentProcess->next) {
                currentProcess->next->parent = currentProcess->parent;
            }

            if (currentProcess->parent) {
                currentProcess->parent->next = currentProcess->next;
            }

            return;
        }
    currentProcess = currentProcess->next;
    }
}

/**
 * wait until process is done or background it
 */
void processSchedule(ProcessType *thisProcess) {
    siginfo_t signal;
    
    if (thisProcess->background) {
        thisProcess->status = RUNNING;
        addProcess(&root, thisProcess);
    } else {
        signal.si_pid = 0;
        foreground = thisProcess;
        waitid(P_PID, thisProcess->pid, &signal, WEXITED | WSTOPPED);
        tcsetpgrp(0, process_group);
        
        if (signal.si_pid != 0) {
            if(signal.si_code == CLD_EXITED) {
                removeProcess(&root, foreground);
                freeProcess(&foreground);
            }
            if(signal.si_code == CLD_STOPPED) {
                thisProcess->status = "Stopped";
                addProcess(&root, foreground);
                foreground = NULL;
            }
        }

    }
}   

/*
 * Execute processes
 */
pid_t executeProcess(ProcessType *process) {
    ProcessType *thisProcess;
    
    pid_t pid;
    int input, output, error;
    int fd[2];

    input = process->std_in;

    for (thisProcess = process; thisProcess != NULL; thisProcess = thisProcess->next) {
        if (thisProcess->next) {
            if (pipe(fd) < 0) {
                perror("pipe");
                return -1;
            }

            output = fd[1]; 
            
            if (thisProcess->std_out == thisProcess->std_err) { error = output; }
        } else {
            output = thisProcess->std_out;
            error = thisProcess->std_err;
            #ifdef DEBUG
                printf("Last one to execute...\n");
            #endif
        }
        
        pid = fork();   // executue single process
        if (pid == 0) { // child process
            #ifdef DEBUG
                printf("Inside child process\n");
            #endif
            if (thisProcess->pid <= 0) {
                thisProcess->pid = getpid();
            }
            
            setpgid(0, thisProcess->pid);
           
            if (thisProcess->background == false) {
                tcsetpgrp(0, thisProcess->pid);
            }

            signal(SIGINT, SIG_DFL);        // default signal handlers
            signal(SIGCHLD, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);

            if (input != 0) {
                dup2(input, 0); 
            }

            if (output != 1) {
                dup2(output, 1);
            }

            if (error != 2) {
                dup2(error, 2);
            }

            if (input > 2) {
                close(input);
            }
                
            if (output > 2) {
                close(output);
            }
            
            if (error > 2) {
                close(error);
            }

            execvp(thisProcess->argv[0], thisProcess->argv);

            // if execvp succeeds, none of the following will happen
            fprintf(stderr, "yash: ");
            perror(thisProcess->argv[0]);
            _exit(1);
            return 0;
        } else {    // parent process
            thisProcess->pid = pid;
            if (process->pid <= 0) {
                process->pid = pid;
             }
            setpgid(pid, process->pid);
        }

        if (input != thisProcess->std_in ) close(input);
        if (output != thisProcess->std_out) close(output);
        if (error != thisProcess->std_err) close(error);
                
        if (thisProcess->std_in  > 2) close(thisProcess->std_in );
        if (thisProcess->std_out > 2) close(thisProcess->std_out);
        if (thisProcess->std_err > 2) close(thisProcess->std_err);

        input = fd[0];
    }
    return pid;
}

/**
 * searches for process in job list by id
 */
ProcessType* searchForProcess(ProcessType *thisProcess, pid_t pid) {
    ProcessType *currentProcess = thisProcess->next;
    
    while (currentProcess) {
        if (currentProcess->pid == pid) { 
            return currentProcess; // return current process if id matches
        }
    currentProcess = currentProcess->next;
    }

    #ifdef DEBUG
        printf("Warning: cannot find process matching id\n");
    #endif

    return NULL;    // if process isn't found, return NULL
}

/**
 * free up process pipeline
 */
void freeProcessPipe(ProcessType *process) {
    ProcessType *currentProcess;
    
    while (process) {   // free all processes in queue
        currentProcess = process->next;
        free(process);
        process = currentProcess;
    }
}

/**
 * gets most recent process
 */
ProcessType* getNewestProcess() {
    ProcessType *currentProcess = root.next;
    ProcessType *recentProcess = NULL;
    
    while (currentProcess) {
        if (strcmp(currentProcess->status, STOPPED) == 0 ) {
            recentProcess = currentProcess;
        } else if (strcmp(currentProcess->status, RUNNING) == 0 ) {
            if (recentProcess == NULL || recentProcess->status == RUNNING) {
                recentProcess = currentProcess;
            }
        }
        currentProcess = currentProcess->next;
    }
    return recentProcess;
}

/**
 * parses input for (number of) arguments
 */
void processInput(char* cmd_line) {
    // Initalize process arguments
    char *argv[MAX_ARGS];                       
    memset(argv, 0, sizeof(argv));
    int argc = 0;
    
    // Check if input is built-in shell command
    int builtin = isShellCommand(cmd_line); 
    if (builtin == 1) { return; }
    
    // Create new Process
    ProcessType *process = (ProcessType *) malloc(sizeof(ProcessType));
    memset(process, 0, sizeof(ProcessType));
    
    process->input = strdup(cmd_line);
 
    // Determine number of arguments and fill argv
    for (argc = 0; argc < MAX_ARGS; argc++ ) {
        argv[argc] = strsep(&cmd_line, " ");    // separate input into arguments
        if (argv[argc] == 0) { 
            break; 
        }
        #ifdef DEBUG
            printf("argument[%d]: %s\n", argc, argv[argc]);
        #endif
    }

    argv[argc] = NULL;  // execvp arguments must end in NULL 
    
    #ifdef DEBUG
        printf("%d total arguments\n", argc);
    #endif

    char **proc_args = argv;

    // Creates new Process called job
    ProcessType *job = (ProcessType *) malloc(sizeof(ProcessType));
    memset(job, 0, sizeof(ProcessType));
   
    job->std_out = 1;
    job->std_err = 2;

    ProcessType *nextProcess = job;

    // File redirection
    for (int i = 0; i < argc; i++) {

        if (strcmp(argv[i], ">") == 0) {    // parses >
            if (i >= argc - 1) continue;
            
            argv[i] = NULL; // remove delimiter from arguments
            
            char *file_path = argv[i+1];
            int fd = open(file_path, O_WRONLY | O_CREAT, S_IRWXU);
            
            if (fd < 0) {
                perror("open");
                #ifdef DEBUG
                    printf("Error: cannot open file: %s\n", file_path);
                #endif
                return;
            } else {
                nextProcess->std_out = fd;
                ++i;    // skips file token
            }
        } else if (strcmp(argv[i], "<") == 0) { // parses <
            if (i >= argc - 1) continue;
            
            argv[i] = NULL; // remove delimiter from arguments
            
            char *file_path = argv[i+1];
            int fd = open(file_path, O_RDONLY);
            if (fd < 0) {
                perror("open");
                #ifdef DEBUG
                    printf("Error: cannot open file: %s\n", file_path);
                #endif
                return;
            } else {
                nextProcess->std_in = fd;
                ++i;    // skips file token
            }
        } else if (strcmp(argv[i], "2>") == 0) {    // parses 2>
            if (i >= argc - 1) continue;
            
            argv[i] = NULL; // remove delimiter from arguments

            char *file_path = argv[i+1];
            int fd = open(file_path, O_WRONLY | O_CREAT, S_IRWXU);
            
            if (fd < 0) {
                perror("open");
                #ifdef DEBUG
                    printf("Error: cannot open file: %s\n", file_path);
                #endif
                return;
            } else {
                nextProcess->std_err = fd;
                ++i;    // skips file token
            }
        } else if (strcmp(argv[i], PIPE) == 0) {    // parses |
            if (i >= argc - 1) continue;
            
            argv[i] = NULL;  // remove delimiter from arguments
            
            nextProcess->argv = proc_args;
            
            nextProcess->next = (ProcessType *) malloc(sizeof(ProcessType));
            nextProcess = nextProcess->next;
            memset(nextProcess, 0, sizeof(ProcessType));
            
            nextProcess->std_out = 1;
            nextProcess->std_err = 2;
            proc_args = &argv[i+1];

        } else if (strcmp(argv[i], BACKGROUND)== 0) {   // parses &
            argv[i] = NULL;  // remove delimiter from arguments
            job->background = true;
        }        
    }

    nextProcess->argv = proc_args;
    
    // Background process if &
    if (job->background) {
        nextProcess = job;
        while (nextProcess) {
            nextProcess->background = true;
            nextProcess = nextProcess->next;
        }
    }

    // Execute process
    if (job) {  
        process->pid = executeProcess(job);
        process->background = job->background;
        
        if (process->pid > 0) {
            processSchedule(process);
        } else {
            removeProcess(&root, foreground);
            freeProcess(&process);
        }   // wait for process to exit or be moved to background
    }
    
    tcsetpgrp(0, process_group);
    freeProcessPipe(job);
    free(cmd_line);
}

/**
 * updates current process statuses and removes processes that are done
 */
void updateProcessList() {
    pid_t pid;
    siginfo_t signal;
    ProcessType *currentProcess = root.next;

    while (currentProcess) {
        signal.si_pid = 0;
        pid = waitid(P_PID, 
                        currentProcess->pid, 
                        &signal, WEXITED | WSTOPPED | WCONTINUED | WNOHANG | WNOWAIT);
       
        if (signal.si_pid != 0) {                                           
            if (signal.si_code == CLD_CONTINUED) {
                currentProcess->status = RUNNING;
                #ifdef DEBUG
                    printf("Status updated: Running\n");
                #endif 
            } else if (signal.si_code == CLD_STOPPED) {
                currentProcess->status = STOPPED;
                #ifdef DEBUG
                    printf("Status updated: Stopped\n");
                #endif
            } else if (signal.si_code == CLD_EXITED) {
                currentProcess->status = DONE;
                currentProcess->sig = signal.si_status;
                #ifdef DEBUG
                    printf("Status updated: Done\n");
                #endif
            }
        }
        
        if (currentProcess->status == DONE) {
            printf("[%d]+ %s  %s\n", 
                   currentProcess->pid, 
                   currentProcess->status, 
                   currentProcess->input);
            
            if (currentProcess->parent) {
                currentProcess->parent->next = currentProcess->next;
            }

            if (currentProcess->next) { 
                currentProcess->next->parent = currentProcess->parent;
            }
            
            ProcessType *next = currentProcess->next;
            freeProcess(&currentProcess);
            currentProcess = next;
        } else {
            currentProcess = currentProcess->next;
        }
    }
}

/**
 * ======================================
 * Custom shell commands and handlers
 * ======================================
 */

/**
 * built-in shell command: bg
 */
void sendProcessToBack() {
    ProcessType *recentProcess = getNewestProcess();
    
    if (recentProcess) {
        if (kill(recentProcess->pid, SIGCONT) < 0) {
            perror("kill");     // kill error
        }  // send process to background
    } else {
        printf("warning: no process to move\n");
    }
}

/**
 * built-in shell command: fg
 */
void sendProcessToFront() {
    ProcessType *recentProcess = getNewestProcess();
    
    if (recentProcess) {
        printf("%s\n", recentProcess->input);
        tcsetpgrp(0, recentProcess->pid);
        
        if (kill(recentProcess->pid, SIGCONT) < 0) {
            perror("kill");
        }
        
        foreground = recentProcess;
        foreground->background = false; 
        
        removeProcess(&root, foreground);
        processSchedule(foreground); // send job to foreground
    } else {
        printf("warning: no process to move\n");
    }
}

/**
 * built-in shell command: jobs
 */
void displayJobs() {
    updateProcessList();
    
    ProcessType *recentProcess = getNewestProcess();
    ProcessType *currentProcess = root.next;
 
    if (!currentProcess) {
        printf("warning: no jobs to display\n");
    }

    while (currentProcess) {    // display jobs, status, fg/bg, and command
        printf("[%d] %s %s  %s\n", 
                currentProcess->pid, 
                ((currentProcess == recentProcess) ? "+" : "-") ,
                currentProcess->status, 
                currentProcess->input);
        currentProcess = currentProcess->next;
    }
}

/**
 * checks if input command is "bg" "fg" "jobs" "exit" or not built-in
 * if shell command, return 1; if custom command, return 0
 */
int isShellCommand(char* cmd_line) {
    if (strcmp(cmd_line, "bg") == 0) {
            #ifdef DEBUG
                printf("Shell command found: BG\n");
            #endif
            sendProcessToBack();
            free(cmd_line);
            return 1;
    } else if (strcmp(cmd_line, "fg") == 0) {
            #ifdef DEBUG
                printf("Shell command found: FG\n");
            #endif
            sendProcessToFront();
            free(cmd_line);
            return 1;
    } else if (strcmp(cmd_line, "jobs") == 0) {
            #ifdef DEBUG
                printf("Shell command found: JOBS\n");
            #endif
            displayJobs();
            free(cmd_line);
            return 1;
    } else if (strcmp(cmd_line, "exit") == 0) {
            #ifdef DEBUG
                printf("Shell command found: EXIT\n");
            #endif 
            killProcess(); 
            _exit(0);          
    } else {
            #ifdef DEBUG
                printf("Input is not a shell command\n");
            #endif
            return 0;
    }
}

/**
 * ============================================
 * Miscellaneous Input Functions, Init(), and Main()
 * ============================================
 */

/**
 *  Replaces newline with /0 at the end 
 */
char* removeNewLine(char* input) {
  size_t length = strlen(input); 

  if (input[length-1] == '\n') {
    input[length-1] = '\0';     
   } // replace newLine

   return input; 
}

/**
 * gets input; returns cmd_line
 * if CTRL+D or no input, exit
 */
char *getCmdLine() {
    char* cmd_line = NULL;

    cmd_line = readline("# ");  // yash prompt
    
    if (!cmd_line) { 
        #ifdef DEBUG
            printf("CTRL+D was pressed. Exiting...\n");
        #endif
        killProcess();
        _exit(0);
    } // if CTRL+D (EOF in Linux), exit

    size_t cmdline_len = strlen(cmd_line);
    if (cmdline_len < 1) {  // no input
        free(cmd_line);
        return NULL;
    }

    cmd_line = removeNewLine(cmd_line);    // removes /n from keyboard input

    add_history(cmd_line);

    return cmd_line;
}

/**
 * initializes things
 */
void init() {
    memset(&root, 0, sizeof(root));             // initalization 
    setbuf(stdout, NULL);
    using_history();

    signal(SIGTTOU, SIG_IGN);                 // ignore these signals
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);   

    process_group = getpgrp(); 
    while (tcgetpgrp(0) != process_group) {
        kill(process_group, SIGTTIN);           // kill all processes not in this session
    }

    process_group = getpid();                      // get process id
    if (setpgid(process_group, process_group) < 0) {
        perror("setpgid");
        _exit(1);
    }

    tcsetpgrp(0, process_group);                  // set current process group id   
}

void main() {
    init(); //initializes signals (only run once)

    while(1) {
        char* input = getCmdLine();  // display prompt
        if (!input) continue;               // if no input, keep polling
        processInput(input);             // otherwise, process input
        updateProcessList();            // update jobs
    }
}