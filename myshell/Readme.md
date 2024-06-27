# Custom Shell Program

This is a custom shell program implemented in C that supports basic command execution, input/output redirection, background processes, and piped commands.

## Table of Contents
- [Features](#features)
- [Usage](#usage)
- [Compilation](#compilation)
- [Running the Shell](#running-the-shell)
- [Command Examples](#command-examples)
- [Design Components](#design-components)
- [Limitations and Future Improvements](#limitations-and-future-improvements)

## Features

- **Command Execution**: Execute standard Unix commands.
- **Input/Output Redirection**: Support for input (`<`) and output (`>`) redirection.
- **Background Processes**: Run commands in the background using `&`.
- **Piped Commands**: Execute commands with a pipe (`|`).
- **Command History**: Use `!!` to execute the last command.
- **Quit Command**: Exit the shell using the `quit` command.
- **Background Process Tracking**: Track and manage up to user defined number of background processes. (currently set to 3)

## Usage

### Compilation

To compile the shell program, use the following command:

gcc -o myshell myshell.c

### Running the Shell

Start the shell by executing the compiled program:

./myshell

### Command Examples

- **Standard Command**:
  myshell> ls -l

- **Input Redirection**:
  myshell> sort < unsorted.txt

- **Output Redirection**:
  myshell> ls -l > output.txt

- **Background Process**:
  myshell> sleep 10 &
  
- **Piped Commands**:
  myshell> ls -l | grep txt

- **Execute Last Command**:
  myshell> !!

- **Quit Shell**:
  myshell> quit


## Design Components

- **`main` function**: The main loop of the shell, handles user input and command parsing.
- **`execute_command` function**: Executes a command with optional input/output redirection.
- **`execute_piped_command` function**: Executes two commands with a pipe.
- **`sigchld_handler` function**: Signal handler for `SIGCHLD` to manage background processes.

### Process Creation and Execution

The shell uses system calls to create and manage processes. When a command is entered, the shell uses the `fork()` system call to create a new child process. The child process uses the `execvp()` system call to execute the specified command. The parent process uses the `waitpid()` system call to wait for the child process to complete if the command is not set to run in the background. This approach ensures that the shell can handle multiple commands, including those that require input/output redirection and piping.

### Background Processes

The shell supports running commands in the background using `&`. After appending `&` to a command, the shell will `fork` a new child process and immediately return control to the parent process, allowing it to accept new commands while the child process runs asynchronously. The shell can track up to a user-defined number of background processes. If more background processes are attempted than allowed, the shell will display an error message. The shell uses the `SIGCHLD` signal to handle the termination of background processes. When a background process completes, the shell prints a message indicating the process ID and its exit status.

### History Feature

The shell supports a basic command history feature. Using the `!!` command, users can execute the most recently entered command. This feature is useful for quickly re-running previous commands without having to retype them. The shell stores the last executed command and retrieves it when `!!` is entered. If no commands have been executed yet, the shell will notify the user that there are no commands in history.

### Piping

The shell supports command piping using the `|` character, allowing the output of one command to be used as the input for another command. When a pipe is detected, the shell splits the commands at the pipe character and sets up a pipe using the `pipe()` system call. It then creates two child processes: one for the left-hand command and one for the right-hand command. The left-hand command writes to the pipe, and the right-hand command reads from the pipe. The shell manages these processes and ensures that the commands are executed in the correct sequence, with the appropriate data passed through the pipe.

## Limitations and Future Improvements
- Limited error handling for input/output redirection and piped commands. Need to add enhance error handling and input validation.
- No support for complex command chaining or advanced shell scripting features. Need to add additional shell features such as command chaining (`&&`, `||`). Also can have more sophisticated command history and recall features.


