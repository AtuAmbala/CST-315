// Building a simple shell program 
//
//Program Description: This program implements a simple shell program that supports the following features:
//1. Interactive mode: The shell displays a prompt and waits for user input. The user can enter commands, which are executed by the shell.
//2. Batch mode: The shell can read commands from a file and execute them in sequence.
//3. Command history: The shell maintains a history of commands entered by the user. The user can view the command history by entering the "history" command.
//4. Built-in commands: The shell supports built-in commands such as "cd" to change the current directory.
//5. Signal handling: The shell handles the SIGINT signal (Ctrl+C) to exit gracefully and the SIGQUIT signal (Ctrl+\) to end execution.
//
//Programmers: Atuhaire Ambala and Ricardo Escarcega


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

// Define constants for maximum input size and argument count
#define MAX_INPUT_SIZE 1024
#define MAX_ARG_COUNT 100
#define MAX_HISTORY_COUNT 100

// Global variable to store command history
char *history[MAX_HISTORY_COUNT];
int history_count = 0;

// Function to add a command to history
void add_to_history(char *command) {
    if (history_count < MAX_HISTORY_COUNT) {
        history[history_count++] = strdup(command);
    } else {
        free(history[0]);
        for (int i = 1; i < MAX_HISTORY_COUNT; i++) {
            history[i - 1] = history[i];
        }
        history[MAX_HISTORY_COUNT - 1] = strdup(command);
    }
}

// Function to display command history
void show_history() {
    printf("Command History:\n");
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

// Function to end execution gracefully
void end_execution() {
    printf("Ending execution...\n");
    exit(0);
}

// Function to exit the shell gracefully
void exit_shell() {
    printf("\nExiting shell...\n");
    exit(0);
}

// Signal handler function
void handle_signal(int sig) {
    if (sig == SIGINT) {  // Handle CTRL+C signal
        exit_shell();
    } else if (sig == SIGQUIT) {  // Handle CTRL+\ signal
        end_execution();
    }
}

// Function to execute a single command
void execute_command(char *command) {
    char *args[MAX_ARG_COUNT];
    char *token = strtok(command, " \t\n");
    int i = 0;

    // Tokenize the command string into arguments
    while (token != NULL && i < MAX_ARG_COUNT - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;  // Null-terminate the arguments array

    if (args[0] == NULL) {
        return;  // No command entered
    }

    // Check for built-in commands
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: expected argument\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
        return;
    } else if (strcmp(args[0], "history") == 0) {
        show_history();
        return;
    }

    pid_t pid = fork();  // Create a new child process
    if (pid < 0) {
        perror("Fork failed");  // Error handling for fork failure
        exit(1);
    } else if (pid == 0) {
        execvp(args[0], args);  // Replace child process with new program
        perror("execvp failed");  // Error handling for execvp failure
        exit(1);
    }
}

// Function to execute multiple commands separated by semicolons
void execute_commands(char *input) {
    add_to_history(input);  // Add command to history
    char *command = strtok(input, ";");
    while (command != NULL) {
        execute_command(command);
        command = strtok(NULL, ";");
    }
    // Wait for all child processes to finish
    while (wait(NULL) > 0);
}

// Function to execute commands from a batch file
void batch_mode(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open batch file");  // Error handling for file open failure
        exit(1);
    }

    char line[MAX_INPUT_SIZE];
    while (fgets(line, sizeof(line), file)) {
        printf("Executing: %s", line);  // Echo the command being executed
        execute_commands(line);  // Execute the commands in the line
    }
    fclose(file);  // Close the batch file
    // Wait for all child processes to finish
    while (wait(NULL) > 0);
}

// Main function
int main(int argc, char *argv[]) {
    // Set up signal handlers for SIGINT and SIGQUIT
    signal(SIGINT, handle_signal);
    signal(SIGQUIT, handle_signal);  // Assuming CTRL-\ for end_execution

    // Check if batch mode or interactive mode
    if (argc == 2) {
        batch_mode(argv[1]);  // Run in batch mode if a filename is provided
    } else {
        char input[MAX_INPUT_SIZE];
        while (1) {
            printf("$lopeShell > ");  // Display the shell prompt
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;  // Exit loop if input is NULL (e.g., EOF)
            }
            execute_commands(input);  // Execute the entered commands
        }
    }

    return 0;  // Return success status
}
