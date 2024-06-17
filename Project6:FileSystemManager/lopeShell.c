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
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <dirent.h>

// Function prototypes
void execute_command(char *command);
void execute_batch_commands(const char *filename);
void create_file(const char *path, size_t size) ;
void modify_file(const char *path, const char *content);
void move_file(const char *source, const char *destination);
void delete_file(const char *path);
void search_file(const char *directory, const char *filename);
void duplicate_file(const char *source_path, const char *dest_path);
void rename_file(const char *old_path, const char *new_path);
void get_basic_info(const char *path);
void get_detailed_info(const char *path);
void list_directory(const char *path);
void create_directory(const char *path);
void modify_directory(const char *path, const char *newPath);
void delete_directory(const char *path);
int delete_directory_recursive(const char *path);
void list_directory_recursive(const char *path, int level);
void rename_directory(const char *old_path, const char *new_path);
void duplicate_directory(const char *source_path, const char *dest_path);
void get_basic_info_dir(const char *path);
void get_detailed_info_dir(const char *path);
void batch_mode(const char *filename);
void list_processes(int detailed, int sort_by_id);
void display_process_info(int id, int detailed);
void modify_process_priority(int id, int new_priority);


// Define constants for maximum input size and argument count
#define MAX_INPUT_SIZE 1024
#define MAX_ARG_COUNT 100
#define MAX_HISTORY_COUNT 100

typedef struct FileDescriptor {
    char name[MAX_INPUT_SIZE];
    char path[MAX_INPUT_SIZE];
    size_t size;
    mode_t permissions;
    uid_t owner;
    gid_t group;
    time_t creation_time;
    time_t modification_time;
    time_t access_time;
} FileDescriptor;

typedef struct DirectoryDescriptor {
    char name[MAX_INPUT_SIZE];
    char path[MAX_INPUT_SIZE];
    struct DirectoryDescriptor *subdirectories[MAX_ARG_COUNT];
    struct FileDescriptor *files[MAX_ARG_COUNT];
    int subdirectory_count;
    int file_count;
} DirectoryDescriptor;

typedef enum { READY, RUNNING, WAITING, TERMINATED } State;

typedef struct {
    int id;
    char name[MAX_INPUT_SIZE];
    State state;
    int priority;
    int burst_time;
    int time_left;
    int io_request; // 0 if no I/O needed, >0 for I/O requests
    int io_time_left;
} Process;

typedef struct {
    Process* processes[MAX_ARG_COUNT];
    int process_count;
    int time_quantum;
    int current_index;
} Scheduler;

Scheduler scheduler;

// Global variable to store command history
char *history[MAX_HISTORY_COUNT];
int history_count = 0;

void initialize_scheduler(int time_quantum) {
    scheduler.process_count = 0;
    scheduler.time_quantum = time_quantum;
    scheduler.current_index = 0;
}


// Root directory
DirectoryDescriptor *root_directory;

void initialize_root_directory() {
    root_directory = malloc(sizeof(DirectoryDescriptor));
    strcpy(root_directory->name, "root_directory");
    strcpy(root_directory->path, "root_directory");
    root_directory->subdirectory_count = 0;
    root_directory->file_count = 0;
}

void add_process(Process *process) {
    scheduler.processes[scheduler.process_count++] = process;
}

void remove_process(int id) {
    for (int i = 0; i < scheduler.process_count; i++) {
        if (scheduler.processes[i]->id == id) {
            for (int j = i; j < scheduler.process_count - 1; j++) {
                scheduler.processes[j] = scheduler.processes[j + 1];
            }
            scheduler.process_count--;
            break;
        }
    }
}

Process* find_process(int id) {
    for (int i = 0; i < scheduler.process_count; i++) {
        if (scheduler.processes[i]->id == id) {
            return scheduler.processes[i];
        }
    }
    return NULL;
}

void update_process_state(int id, State new_state) {
    Process *process = find_process(id);
    if (process != NULL) {
        process->state = new_state;
    }
}

void round_robin_schedule() {
    if (scheduler.process_count == 0) return;

    Process *current_process = scheduler.processes[scheduler.current_index];
    
    if (current_process->state == RUNNING) {
        current_process->time_left -= scheduler.time_quantum;
        if (current_process->time_left <= 0) {
            current_process->state = TERMINATED;
            remove_process(current_process->id);
        } else if (current_process->io_request) {
            current_process->state = WAITING;
            current_process->io_time_left = current_process->io_request;
        } else {
            current_process->state = READY;
        }
    }
    
    scheduler.current_index = (scheduler.current_index + 1) % scheduler.process_count;
    current_process = scheduler.processes[scheduler.current_index];
    
    if (current_process->state == READY) {
        current_process->state = RUNNING;
    } else if (current_process->state == WAITING && current_process->io_time_left <= 0) {
        current_process->state = READY;
    }
}



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
    else if (strcmp(args[0], "create") == 0 && strcmp(args[1], "process") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            fprintf(stderr, "create process: expected name and burst time\n");
        } else {
            Process *new_process = malloc(sizeof(Process));
            new_process->id = scheduler.process_count + 1;
            strcpy(new_process->name, args[2]);
            new_process->state = READY;
            new_process->priority = 0;
            new_process->burst_time = atoi(args[3]);
            new_process->time_left = new_process->burst_time;
            new_process->io_request = 0;
            new_process->io_time_left = 0;
            add_process(new_process);
            printf("Process %s created with burst time %d.\n", args[2], new_process->burst_time);
        }
        return;
    } else if (strcmp(args[0], "procs") == 0) {
        int detailed = 0;
        int sort_by_id = 0;
        for (int j = 1; args[j] != NULL; j++) {
            if (strcmp(args[j], "-a") == 0) {
                detailed = 1;
            } else if (strcmp(args[j], "-si") == 0) {
                sort_by_id = 1;
            }
        }
        list_processes(detailed, sort_by_id);
        return;
    } else if (strcmp(args[0], "schedule") == 0) {
        round_robin_schedule();
        return;
    } else if (strcmp(args[0], "delete") == 0 && strcmp(args[1], "process") == 0) {
        if (args[2] == NULL) {
            fprintf(stderr, "delete process: expected process ID\n");
        } else {
            int id = atoi(args[2]);
            remove_process(id);
            printf("Process %d deleted.\n", id);
        }
        return;
    } else if (strcmp(args[0], "modify") == 0 && strcmp(args[1], "priority") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            fprintf(stderr, "modify priority: expected process ID and new priority\n");
        } else {
            int id = atoi(args[2]);
            int new_priority = atoi(args[3]);
            Process *process = find_process(id);
            if (process != NULL) {
                process->priority = new_priority;
                printf("Process %d priority changed to %d.\n", id, new_priority);
            } else {
                fprintf(stderr, "Process %d not found.\n", id);
            }
        }
        return;
    } else if (strcmp(args[0], "info") == 0 && strcmp(args[1], "process") == 0) {
        if (args[2] == NULL) {
            fprintf(stderr, "info process: expected process ID\n");
        } else {
            int id = atoi(args[2]);
            Process *process = find_process(id);
            if (process != NULL) {
                printf("ID: %d\n", process->id);
                printf("Name: %s\n", process->name);
                printf("State: %d\n", process->state);
                printf("Priority: %d\n", process->priority);
                printf("Burst Time: %d\n", process->burst_time);
                printf("Time Left: %d\n", process->time_left);
                printf("I/O Request: %d\n", process->io_request);
                printf("I/O Time Left: %d\n", process->io_time_left);
            } else {
                fprintf(stderr, "Process %d not found.\n", id);
            }
        }
        return;
    } else if (strcmp(args[0], "priority") == 0 && strcmp(args[1], "process") == 0) {
        if (args[2] == NULL) {
            fprintf(stderr, "priority process: expected process ID\n");
        } else {
            int id = atoi(args[2]);
            Process *process = find_process(id);
            if (process != NULL) {
                printf("Process %d priority: %d\n", id, process->priority);
            } else {
                fprintf(stderr, "Process %d not found.\n", id);
            }
        }
        return;
    } else if (strcmp(args[0], "cd") == 0) {
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
    } else if (strcmp(args[0], "create") == 0 && strcmp(args[1], "dir") == 0) {
        if (args[2] == NULL) {
            fprintf(stderr, "create dir: expected directory path\n");
        } else {
            printf("Creating directory...\n");
            create_directory(args[2]);
        }
        return;
    } else if (strcmp(args[0], "modify") == 0 && strcmp(args[1], "dir") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            fprintf(stderr, "modify dir: expected directory path and new path\n");
        } else {
            printf("Modifying directory...\n");
            modify_directory(args[2], args[3]);
        }
        return;
    } else if (strcmp(args[0], "delete") == 0 && strcmp(args[1], "dir") == 0) {
        if (args[2] == NULL) {
            fprintf(stderr, "delete dir: expected directory path\n");
        } else {
            printf("Deleting directory recursively...\n");
            if (delete_directory_recursive(args[2]) != 0) {
                fprintf(stderr, "Failed to delete directory: %s\n", args[2]);
            }
        }
        return;
    } else if (strcmp(args[0], "create") == 0 && strcmp(args[1], "file") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            fprintf(stderr, "create file: expected file path and size\n");
        } else {
            printf("Creating file with random data...\n");
            size_t size = atoi(args[3]);
            create_file(args[2], size);
        }
        return;
    } else if (strcmp(args[0], "modify") == 0 && strcmp(args[1], "file") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            fprintf(stderr, "modify file: expected file path and content\n");
        } else {
            printf("Modifying file...\n");
            modify_file(args[2], args[3]);
        }
        return;
    } else if (strcmp(args[0], "move") == 0 && strcmp(args[1], "file") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            fprintf(stderr, "move file: expected source path and destination path\n");
        } else {
            printf("Moving file...\n");
            move_file(args[2], args[3]);
        }
        return;
    } else if (strcmp(args[0], "delete") == 0 && strcmp(args[1], "file") == 0) {
        if (args[2] == NULL) {
            fprintf(stderr, "delete file: expected file path\n");
        } else {
            printf("Deleting file...\n");
            delete_file(args[2]);
        }
        return;
    } else if (strcmp(args[0], "search") == 0 && strcmp(args[1], "file") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            fprintf(stderr, "search file: expected directory and filename\n");
        } else {
            printf("Searching for file...\n");
            search_file(args[2], args[3]);
        }
        return;
    }else if (strcmp(args[0], "list") == 0 && strcmp(args[1], "dir") == 0) {
        if (args[2] == NULL) {
            fprintf(stderr, "list dir: expected directory path\n");
        } else {
            printf("Listing directory...\n");
            list_directory(args[2]);
        }
        return;
    } else if (strcmp(args[0], "list") == 0 && strcmp(args[1], "recursive") == 0) {
        if (args[2] == NULL) {
            fprintf(stderr, "list recursive: expected directory path\n");
        } else {
            printf("Listing directory recursively...\n");
            list_directory_recursive(args[2], 0);
        }
        return;
    } else if (strcmp(args[0], "rename") == 0 && strcmp(args[1], "file") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            fprintf(stderr, "rename file: expected old path and new path\n");
        } else {
            printf("Renaming file...\n");
            rename_file(args[2], args[3]);
        }
        return;
    } else if (strcmp(args[0], "rename") == 0 && strcmp(args[1], "dir") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            fprintf(stderr, "rename dir: expected old path and new path\n");
        } else {
            printf("Renaming directory...\n");
            rename_directory(args[2], args[3]);
        }
        return;
    } else if (strcmp(args[0], "duplicate") == 0 && strcmp(args[1], "file") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            fprintf(stderr, "duplicate file: expected source path and destination path\n");
        } else {
            printf("Duplicating file...\n");
            duplicate_file(args[2], args[3]);
        }
        return;
    } else if (strcmp(args[0], "duplicate") == 0 && strcmp(args[1], "dir") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            fprintf(stderr, "duplicate dir: expected source path and destination path\n");
        } else {
            printf("Duplicating directory...\n");
            duplicate_directory(args[2], args[3]);
        }
        return;
    } else if (strcmp(args[0], "info") == 0 && strcmp(args[1], "file") == 0) {
        if (args[2] == NULL) {
            fprintf(stderr, "info file: expected file path\n");
        } else {
            if (args[3] != NULL && strcmp(args[3], "--detailed") == 0) {
                printf("Getting detailed information about file...\n");
                get_detailed_info(args[2]);
            } else {
                printf("Getting basic information about file...\n");
                get_basic_info(args[2]);
            }
        }
        return;
    } else if (strcmp(args[0], "info") == 0 && strcmp(args[1], "dir") == 0) {
        if (args[2] == NULL) {
            fprintf(stderr, "info dir: expected directory path\n");
        } else {
            if (args[3] != NULL && strcmp(args[3], "--detailed") == 0) {
                printf("Getting detailed information about directory...\n");
                get_detailed_info_dir(args[2]);
            } else {
                printf("Getting basic information about directory...\n");
                get_basic_info_dir(args[2]);
            }
        }
        return;
    } else if (strcmp(args[0], "info") == 0 && strcmp(args[1], "process") == 0) {
        if (args[2] == NULL) {
            fprintf(stderr, "info process: expected process ID\n");
        } else {
            int id = atoi(args[2]);
            int detailed = 0;
            if (args[3] != NULL && strcmp(args[3], "--detailed") == 0) {
                detailed = 1;
            }
            display_process_info(id, detailed);
        }
        return;
    } else if (strcmp(args[0], "priority") == 0 && strcmp(args[1], "process") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            fprintf(stderr, "priority process: expected process ID and new priority\n");
        } else {
            int id = atoi(args[2]);
            int new_priority = atoi(args[3]);
            modify_process_priority(id, new_priority);
        }
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
        // Remove newline character if present
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        printf("Executing: %s", line);  // Echo the command being executed
        execute_commands(line);  // Execute the commands in the line
    }
    fclose(file);  // Close the batch file
    // Wait for all child processes to finish
    while (wait(NULL) > 0);
}

DirectoryDescriptor* find_directory(const char *path, DirectoryDescriptor *current_dir) {
    if (strcmp(path, current_dir->path) == 0) {
        return current_dir;
    }

    for (int i = 0; i < current_dir->subdirectory_count; i++) {
        DirectoryDescriptor *result = find_directory(path, current_dir->subdirectories[i]);
        if (result != NULL) {
            return result;
        }
    }

    return NULL;
}

// Function to create a file with a specified amount of random data
void create_file(const char *path, size_t size) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror("Failed to create file");
        exit(EXIT_FAILURE);
    }

    // Generate random data for the file
    srand((unsigned int)time(NULL));
    for (size_t i = 0; i < size; i++) {
        fputc(rand() % 256, file);
    }
    fclose(file);

    // Update internal structure
    FileDescriptor *new_file = malloc(sizeof(FileDescriptor));
    char *name = strrchr(path, '/') ? strrchr(path, '/') + 1 : (char *)path;
    strcpy(new_file->name, name);
    strcpy(new_file->path, path);
    new_file->size = size;
    struct stat file_stat;
    stat(path, &file_stat);
    new_file->permissions = file_stat.st_mode; // The file is created with the specified permissions
    new_file->owner = file_stat.st_uid; // The file is created with the specified owner and group
    new_file->group = file_stat.st_gid; // The file is created with the specified owner and group
    new_file->creation_time = file_stat.st_ctime; // The file is created with the current time as the creation time
    new_file->modification_time = file_stat.st_mtime; // The file is created with the current time as the modification time
    new_file->access_time = file_stat.st_atime; // The file is created with the current time as the access time

    // Find the parent directory and add the new file to it
    char parent_path[MAX_INPUT_SIZE];
    strcpy(parent_path, path);
    char *last_slash = strrchr(parent_path, '/');
    if (last_slash) {
        *last_slash = '\0';
    } else {
        strcpy(parent_path, ".");
    }

    DirectoryDescriptor *parent_dir = find_directory(parent_path, root_directory);
    if (parent_dir != NULL) {
        parent_dir->files[parent_dir->file_count++] = new_file;
    }

    printf("File %s created with %zu bytes of random data.\n", path, size);
}



// Function to modify a file by appending content
void modify_file(const char *path, const char *content) {
    FILE *file = fopen(path, "a");  // Use "a" mode to append content
    if (file == NULL) {
        perror("Failed to open file for modification");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%s\n", content);
    fclose(file);
} 

// Function to delete a file
void delete_file(const char *path) {
    if (remove(path) != 0) {
        perror("Failed to delete file");
        exit(EXIT_FAILURE);
    }
}

// Function to move a file
void move_file(const char *source_path, const char *dest_path) {
    if (rename(source_path, dest_path) != 0) {
        perror("Failed to move file");
        exit(EXIT_FAILURE);
    }
}

// Function to get basic information about a file
void get_basic_info(const char *path) {
    struct stat file_stat;
    if (stat(path, &file_stat) != 0) {
        perror("Failed to get file status");
        return;
    }

    printf("Basic information for %s:\n", path);
    printf("Size: %lld bytes\n", (long long)file_stat.st_size);
    printf("Last modified: %s", ctime(&file_stat.st_mtime));
}


// Function to get detailed information about a file
void get_detailed_info(const char *path) {
    struct stat file_stat;
    if (stat(path, &file_stat) != 0) {
        perror("Failed to get file status");
        return;
    }

    printf("Detailed information for %s:\n", path);
    printf("Size: %lld bytes\n", (long long)file_stat.st_size);
    printf("Permissions: %o\n", file_stat.st_mode & 0777);
    printf("Owner UID: %d\n", file_stat.st_uid);
    printf("Group GID: %d\n", file_stat.st_gid);
    printf("Last accessed: %s", ctime(&file_stat.st_atime));
    printf("Last modified: %s", ctime(&file_stat.st_mtime));
    printf("Last status change: %s", ctime(&file_stat.st_ctime));
}

// Function to duplicate a file
void duplicate_file(const char *source_path, const char *dest_path) {
    FILE *src = fopen(source_path, "rb");
    if (src == NULL) {
        perror("Failed to open source file");
        exit(EXIT_FAILURE);
    }

    FILE *dst = fopen(dest_path, "wb");
    if (dst == NULL) {
        perror("Failed to open destination file");
        fclose(src);
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, n, dst) != n) {
            perror("Failed to write to destination file");
            fclose(src);
            fclose(dst);
            exit(EXIT_FAILURE);
        }
    }

    fclose(src);
    fclose(dst);
    printf("File duplicated from %s to %s successfully.\n", source_path, dest_path);
}

// Function to get basic information about a directory
void get_basic_info_dir(const char *path) {
    struct stat dir_stat;
    if (stat(path, &dir_stat) != 0) {
        perror("Failed to get directory status");
        return;
    }

    printf("Basic information for directory %s:\n", path);
    printf("Size: %lld bytes\n", (long long)dir_stat.st_size);
    printf("Last modified: %s", ctime(&dir_stat.st_mtime));
}

// Function to get detailed information about a directory
void get_detailed_info_dir(const char *path) {
    struct stat dir_stat;
    if (stat(path, &dir_stat) != 0) {
        perror("Failed to get directory status");
        return;
    }

    printf("Detailed information for directory %s:\n", path);
    printf("Size: %lld bytes\n", (long long)dir_stat.st_size);
    printf("Permissions: %o\n", dir_stat.st_mode & 0777);
    printf("Owner UID: %d\n", dir_stat.st_uid);
    printf("Group GID: %d\n", dir_stat.st_gid);
    printf("Last accessed: %s", ctime(&dir_stat.st_atime));
    printf("Last modified: %s", ctime(&dir_stat.st_mtime));
    printf("Last status change: %s", ctime(&dir_stat.st_ctime));
}

// Function to create a directory
void create_directory(const char *path) {
    if (mkdir(path, 0777) != 0) {
        perror("Failed to create directory");
        exit(EXIT_FAILURE);
    }

    // Update internal structure
    DirectoryDescriptor *new_dir = malloc(sizeof(DirectoryDescriptor));
    if (new_dir == NULL) {
        perror("Failed to allocate memory for new directory");
        exit(EXIT_FAILURE);
    }
    char *name = strrchr(path, '/') ? strrchr(path, '/') + 1 : (char *)path;
    strcpy(new_dir->name, name);
    strcpy(new_dir->path, path);
    new_dir->subdirectory_count = 0;
    new_dir->file_count = 0;

    // Find the parent directory and add the new directory to it
    char parent_path[MAX_INPUT_SIZE];
    strcpy(parent_path, path);
    char *last_slash = strrchr(parent_path, '/');
    if (last_slash) {
        *last_slash = '\0';
    } else {
        strcpy(parent_path, ".");
    }

    // Add new_dir to its parent directory in the internal structure
    DirectoryDescriptor *parent_dir = find_directory(parent_path, root_directory);
    if (parent_dir != NULL) {
        parent_dir->subdirectories[parent_dir->subdirectory_count++] = new_dir;
    }

    printf("Directory %s created successfully.\n", path);
}


// Function to duplicate a directory recursively
void duplicate_directory(const char *source_path, const char *dest_path) {
    DIR *d = opendir(source_path);
    if (d == NULL) {
        perror("Failed to open source directory");
        exit(EXIT_FAILURE);
    }

    if (mkdir(dest_path, 0777) != 0) {
        perror("Failed to create destination directory");
        closedir(d);
        exit(EXIT_FAILURE);
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
            continue;
        }

        char src_path[MAX_INPUT_SIZE];
        char dst_path[MAX_INPUT_SIZE];
        snprintf(src_path, sizeof(src_path), "%s/%s", source_path, dir->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", dest_path, dir->d_name);

        if (dir->d_type == DT_DIR) {
            duplicate_directory(src_path, dst_path);
        } else {
            duplicate_file(src_path, dst_path);
        }
    }

    closedir(d);
    printf("Directory duplicated from %s to %s successfully.\n", source_path, dest_path);
}

// Function to modify a directory (rename a directory)
void modify_directory(const char *path, const char *newPath) {
    printf("Modifying directory: %s to %s\n", path, newPath);
    // Rename the directory
    if (rename(path, newPath) != 0) {
        perror("Failed to rename directory");
        exit(EXIT_FAILURE);
    }
    printf("Directory renamed successfully.\n");
}


// Function to delete a directory
void delete_directory(const char *path) {
    if (rmdir(path) != 0) {
        perror("Failed to delete directory");
        exit(EXIT_FAILURE);
    }

    //printf("Deleting directory...\n");
}

// Function to search for a file in a directory tree
void search_file(const char *directory, const char *filename) {
    DIR *d = opendir(directory);
    if (d == NULL) {
        perror("Failed to open directory");
        return;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
            continue;
        }

        char path[MAX_INPUT_SIZE];
        snprintf(path, sizeof(path), "%s/%s", directory, dir->d_name);

        if (dir->d_type == DT_DIR) {
            search_file(path, filename);
        } else if (strcmp(dir->d_name, filename) == 0) {
            printf("File found: %s\n", path);
        }
    }

    closedir(d);
}

// Function to delete a directory recursively
int delete_directory_recursive(const char *path) {
    DIR *d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1;

    if (d) {
        struct dirent *p;
        r = 0;
        while (!r && (p = readdir(d))) {
            int r2 = -1;
            char *buf;
            size_t len;

            // Skip the names "." and ".." as we don't want to recurse on them.
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
                continue;
            }

            len = path_len + strlen(p->d_name) + 2;
            buf = malloc(len);

            if (buf) {
                struct stat statbuf;
                snprintf(buf, len, "%s/%s", path, p->d_name);

                if (!stat(buf, &statbuf)) {
                    if (S_ISDIR(statbuf.st_mode)) {
                        r2 = delete_directory_recursive(buf);
                    } else {
                        r2 = unlink(buf);
                    }
                }
                free(buf);
            }
            r = r2;
        }
        closedir(d);
    }

    if (!r) {
        r = rmdir(path);
    }

    if (r) {
        perror("Failed to delete directory recursively");
        return -1;
    }

    return 0;
}


// Function to list contents of a directory
void list_directory(const char *path) {
    DIR *d;
    struct dirent *dir;
    d = opendir(path);
    if (d) {
        printf("Contents of directory %s:\n", path);
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
                continue;
            }
            if (dir->d_type == DT_DIR) {
                printf("%s/\n", dir->d_name);
            } else {
                printf("%s\n", dir->d_name);
            }
        }
        closedir(d);
    } else {
        perror("Failed to open directory");
        exit(EXIT_FAILURE);
    }
}

// Function to list contents of a directory recursively
void list_directory_recursive(const char *path, int level) {
    DIR *d;
    struct dirent *dir;
    d = opendir(path);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
                continue;
            }
            for (int i = 0; i < level; i++) {
                printf("  ");
            }
            // if its a subdirectory, print it with a trailing slash
            if (dir->d_type == DT_DIR) {
                printf("%s/\n", dir->d_name);
            } else {
                printf("%s\n", dir->d_name);
            }

            // Recursively list subdirectories
            if (dir->d_type == DT_DIR) {
                char new_path[MAX_INPUT_SIZE];
                snprintf(new_path, sizeof(new_path), "%s/%s", path, dir->d_name);
                list_directory_recursive(new_path, level + 1);
            }
        }
        closedir(d);
    } else {
        perror("Failed to open directory");
        exit(EXIT_FAILURE);
    }
}

// Function to rename a file
void rename_file(const char *old_path, const char *new_path) {
    if (rename(old_path, new_path) != 0) {
        perror("Failed to rename file");
        exit(EXIT_FAILURE);
    } else {
        printf("File renamed from %s to %s successfully.\n", old_path, new_path);
    }
}

// Function to rename a directory
void rename_directory(const char *old_path, const char *new_path) {
    if (rename(old_path, new_path) != 0) {
        perror("Failed to rename directory");
        exit(EXIT_FAILURE);
    } else {
        printf("Directory renamed from %s to %s successfully.\n", old_path, new_path);
    }
}


void list_processes(int detailed, int sort_by_id) {
    // Sort by ID if requested
    if (sort_by_id) {
        for (int i = 0; i < scheduler.process_count - 1; i++) {
            for (int j = 0; j < scheduler.process_count - i - 1; j++) {
                if (scheduler.processes[j]->id > scheduler.processes[j + 1]->id) {
                    Process *temp = scheduler.processes[j];
                    scheduler.processes[j] = scheduler.processes[j + 1];
                    scheduler.processes[j + 1] = temp;
                }
            }
        }
    }

    for (int i = 0; i < scheduler.process_count; i++) {
        Process *p = scheduler.processes[i];
        if (detailed) {
            printf("ID: %d, Name: %s, State: %d, Priority: %d, Burst Time: %d, Time Left: %d\n",
                   p->id, p->name, p->state, p->priority, p->burst_time, p->time_left);
        } else {
            printf("ID: %d, Name: %s, State: %d\n", p->id, p->name, p->state);
        }
    }
}


void display_process_info(int id, int detailed) {
    Process *p = find_process(id);
    if (p == NULL) {
        printf("Process with ID %d not found.\n", id);
        return;
    }

    if (detailed) {
        printf("ID: %d, Name: %s, State: %d, Priority: %d, Burst Time: %d, Time Left: %d\n",
               p->id, p->name, p->state, p->priority, p->burst_time, p->time_left);
    } else {
        printf("ID: %d, Name: %s, State: %d\n", p->id, p->name, p->state);
    }
}

void modify_process_priority(int id, int priority) {
    Process *p = find_process(id);
    if (p == NULL) {
        printf("Process with ID %d not found.\n", id);
        return;
    }

    p->priority = priority;
    printf("Priority of process %d set to %d.\n", id, priority);
}



// Main function
int main(int argc, char *argv[]) {
    // Initialize the root directory
    initialize_root_directory();

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

