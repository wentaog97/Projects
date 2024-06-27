#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_LINE 80 // Max command length
#define MAX_BG_PROCS 3 // Maximum number of background processes to track 

// Structure to track background processes
typedef struct {
    pid_t pid;
    int active;
} bg_proc_t;

// Specify how many processes to run in the background and declare the counter
bg_proc_t bg_procs[MAX_BG_PROCS];
int bg_proc_count = 0;

// Signal handler for SIGCHLD
void sigchld_handler(int sig)
{
    int status;
    pid_t pid;

    // Use waitpid to handle multiple child processes
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < bg_proc_count; i++) {
          if (bg_procs[i].pid == pid) {
              bg_procs[i].active = 0; // Mark process as inactive
              if (WIFEXITED(status)) {
                  printf("\nBackground process %d completed with status %d\n", pid, WEXITSTATUS(status));
              } else {
                  printf("\nBackground process %d terminated abnormally\n", pid);
              }
              fflush(stdout);
              break;
          }
        }
        // Remove inactive processes from the list
        int j = 0;
        for (int i = 0; i < bg_proc_count; i++) {
            if (bg_procs[i].active) {
                bg_procs[j++] = bg_procs[i];
            }
        }
        bg_proc_count = j;
    }
}

// Function to execute commands with redirection
void execute_command(char **args, int background) {
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork failed!\n");
        return;
    } else if (pid == 0) { // Child process
        int in_redirect = -1, out_redirect = -1;
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "<") == 0) {
                in_redirect = i;
            } else if (strcmp(args[i], ">") == 0) {
                out_redirect = i;
            }
        }
        
        // Handle input redirection
        if (in_redirect != -1) {
            int fd = open(args[in_redirect + 1], O_RDONLY);
            if (fd < 0) {
                perror("open input file failed");
                return;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[in_redirect] = NULL;
        }
        
        // Handle output redirection
        if (out_redirect != -1) {
            int fd = open(args[out_redirect + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
            if (fd < 0) {
                perror("open output file failed");
                return;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[out_redirect] = NULL;
        }

        if (execvp(args[0], args) == -1) {
            perror("execvp failed");
        }
        return;
    } else { // Parent process
        if (!background) {
            waitpid(pid, NULL, 0);
        } else {
            printf("Process running in background with PID %d\n", pid);
            // Add to background process list
            if (bg_proc_count < MAX_BG_PROCS) {
                bg_procs[bg_proc_count].pid = pid;
                bg_procs[bg_proc_count].active = 1;
                bg_proc_count++;
            } else {
                fprintf(stderr, "Too many background processes\n");
            }
        }
    }
}

// Function to execute piped commands
void execute_piped_command(char **left_args, char **right_args, int background) {
    int pipefd[2];
    pid_t pid1, pid2;

    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return;
    }

    pid1 = fork();
    if (pid1 < 0) {
        perror("fork failed");
        return;
    } else if (pid1 == 0) { // First child (left command)
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        if (execvp(left_args[0], left_args) == -1) {
            perror("execvp left command failed");
        }
        return;
    }

    pid2 = fork();
    if (pid2 < 0) {
        perror("fork failed");
        return;
    } else if (pid2 == 0) { // Second child (right command)
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        if (execvp(right_args[0], right_args) == -1) {
            perror("execvp right command failed");
        }
        return;
    }

    close(pipefd[0]);
    close(pipefd[1]);

    if (!background) {
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    } else {
        printf("Processes running in background with PIDs %d and %d\n", pid1, pid2);
        // Add to background process list
        if (bg_proc_count < MAX_BG_PROCS) {
            bg_procs[bg_proc_count].pid = pid1;
            bg_procs[bg_proc_count].active = 1;
            bg_proc_count++;
            bg_procs[bg_proc_count].pid = pid2;
            bg_procs[bg_proc_count].active = 1;
            bg_proc_count++;
        } else {
            fprintf(stderr, "Too many background processes\n");
        }
    }
}

int main(void) {
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    char *left_args[MAX_LINE/2 + 1];
    char *right_args[MAX_LINE/2 + 1];
    int should_run = 1; /* flag to determine when to exit program */
    char *pch;
    char input[MAX_LINE];
    char last_command[MAX_LINE] = ""; // Added to store the last command
    
    // Set up the SIGCHLD handler using sigaction
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, NULL);

    while (should_run) {
        printf("myshell> ");
        fflush(stdout);

        if (fgets(input, MAX_LINE, stdin) == NULL) { // If user inputs nothing
            perror("fgets failed");
            continue;
        }
        input[strcspn(input, "\n")] = 0; 

        if (strcmp(input, "quit") == 0) { // Added to check for "quit"
            should_run = 0;
            for (int i = 0; i < bg_proc_count; i++) {
                if (bg_procs[i].active) {
                    waitpid(bg_procs[i].pid, NULL, 0);
                    printf("Background process %d completed\n", bg_procs[i].pid);
                }
            }
            continue;
        }

        if (strcmp(input, "!!") == 0) { // Added to check for "!!"
            if (strlen(last_command) == 0) {
                printf("No commands in history\n");
                continue;
            } else {
                strcpy(input, last_command);
                printf("%s\n", input);
            }
        } else {
            if (strlen(input) > 0) { // Added to update last_command
                strcpy(last_command, input);
            }
        }

        pch = strtok(input, " ");
        int i = 0;
        while (pch != NULL) {
            args[i++] = pch;
            pch = strtok(NULL, " ");
        }
        
        int background = 0;
        if (i > 0 && strcmp(args[i-1], "&") == 0) {
            background = 1;
            args[--i] = NULL; // Remove the '&' from args
        } else {
            args[i] = NULL; // Null-terminate the args array
        }

        // Check for piping
        int pipe_found = 0;
        for (int j = 0; j < i; j++) {
            if (strcmp(args[j], "|") == 0) {
                pipe_found = 1;
                args[j] = NULL;
                for (int k = 0; k < j; k++) {
                    left_args[k] = args[k];
                }
                left_args[j] = NULL;
                for (int k = j + 1, l = 0; k < i; k++, l++) {
                    right_args[l] = args[k];
                }
                right_args[i - j - 1] = NULL;
                break;
            }
        }

        if (pipe_found) {
            execute_piped_command(left_args, right_args, background);
        } else {
            execute_command(args, background);
        }
    }
    return 0;
}