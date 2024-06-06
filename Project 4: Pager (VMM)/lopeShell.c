#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

// Constants for memory management and limits
#define MAX_INPUT_SIZE 1024
#define MAX_ARG_COUNT 100
#define MAX_HISTORY_COUNT 100
#define PAGE_SIZE 4096  // Size of each virtual page in bytes
#define PHYSICAL_MEMORY_SIZE (1 << 30)  // 1 GB of physical memory
#define VIRTUAL_MEMORY_SIZE (1 << 32)  // 4 GB of virtual memory
#define MAX_FRAMES (PHYSICAL_MEMORY_SIZE / PAGE_SIZE)
#define MAX_OPEN_FILES 256

// Structure representing an entry in the page table
typedef struct {
    int frame_number;  // The frame number in physical memory
    int valid;  // 1 if the page is valid, 0 otherwise
    int modified;  // 1 if the page has been modified, 0 otherwise
} PageTableEntry;

// Structure representing a page table
typedef struct {
    PageTableEntry *entries;  // Array of page table entries
    int num_entries;  // Number of entries in the page table
} PageTable;

// Structure representing an entry in the frame table
typedef struct {
    int is_free;  // 1 if the frame is free, 0 if it is allocated
    int process_id;  // ID of the process to which this frame is allocated
    int page_number;  // Page number within the process's page table
} FrameTableEntry;

// Global frame table
FrameTableEntry frame_table[MAX_FRAMES];

// Structure representing an entry in the LRU list for page replacement
typedef struct {
    int frame_number;  // The frame number
    int last_used_time;  // The last time the frame was used
} LRUEntry;

// Global LRU list and current time for LRU replacement
LRUEntry lru_list[MAX_FRAMES];
int current_time = 0;

// Structure representing the resources allocated to a process
typedef struct {
    void **allocated_memory;  // Array of pointers to allocated memory blocks
    int num_allocated_blocks;  // Number of allocated memory blocks
    int open_files[MAX_OPEN_FILES];  // Array of open file descriptors
    int num_open_files;  // Number of open file descriptors
} ProcessResources;

// Array to keep track of resources for multiple processes
ProcessResources process_resources[100];

// Function forward declarations
void free_page_table(PageTable *pt, int process_id);
void cleanup_process_resources(int process_id);
void set_path_environment();

// Array to store command history
char *history[MAX_HISTORY_COUNT];
int history_count = 0;

// Function to add a command to the history
void add_to_history(char *command) {
    if (history_count < MAX_HISTORY_COUNT) {
        history[history_count++] = strdup(command);
    } else {
        // Remove the oldest command if history is full
        free(history[0]);
        for (int i = 1; i < MAX_HISTORY_COUNT; i++) {
            history[i - 1] = history[i];
        }
        history[MAX_HISTORY_COUNT - 1] = strdup(command);
    }
}

// Function to display the command history
void show_history() {
    printf("Command History:\n");
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

// Function to end execution of the shell
void end_execution() {
    printf("Ending execution...\n");
    exit(0);
}

// Function to exit the shell
void exit_shell() {
    printf("\nExiting shell...\n");
    exit(0);
}

// Signal handler for SIGINT and SIGQUIT
void handle_signal(int sig) {
    if (sig == SIGINT) {
        exit_shell();
    } else if (sig == SIGQUIT) {
        end_execution();
    }
}

// Function to execute a command
void execute_command(char *command, int process_id, PageTable *pt) {
    char *args[MAX_ARG_COUNT];
    char *token = strtok(command, " \t\n");
    int i = 0;

    // Tokenize the command string into arguments
    while (token != NULL && i < MAX_ARG_COUNT - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;

    if (args[0] == NULL) {
        return;  // No command entered
    }

    // Handle built-in commands
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

    // Fork a child process to execute the command
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process executes the command
        execvp(args[0], args);
        perror("execvp failed");  // If execvp fails
        exit(1);
    } else {
        wait(NULL);  // Parent process waits for the child to finish
    }
}

// Function to execute multiple commands separated by semicolons
void execute_commands(char *input, int process_id, PageTable *pt) {
    add_to_history(input);
    char *command = strtok(input, ";");
    while (command != NULL) {
        execute_command(command, process_id, pt);
        command = strtok(NULL, ";");
    }
    while (wait(NULL) > 0);  // Wait for all child processes to finish
}

// Function to run commands from a batch file
void batch_mode(const char *filename) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        execlp("sh", "sh", filename, (char *)NULL);
        perror("execlp failed");  // If execlp fails
        exit(1);
    } else {
        wait(NULL);  // Wait for the child process to finish
    }
}

// Function to initialize a page table for a process
void init_page_table(PageTable *pt, int num_pages) {
    pt->num_entries = num_pages;
    pt->entries = (PageTableEntry *)malloc(num_pages * sizeof(PageTableEntry));
    for (int i = 0; i < num_pages; i++) {
        pt->entries[i].frame_number = -1;
        pt->entries[i].valid = 0;
        pt->entries[i].modified = 0;
    }
}

// Function to calculate the number of pages needed for a process
int calculate_pages_needed(int process_memory) {
    return (process_memory + PAGE_SIZE - 1) / PAGE_SIZE;
}

// Function to initialize the frame table
void init_frame_table() {
    for (int i = 0; i < MAX_FRAMES; i++) {
        frame_table[i].is_free = 1;
        frame_table[i].process_id = -1;
        frame_table[i].page_number = -1;
    }
}

// Function to allocate a frame for a process
int allocate_frame(int process_id, int page_number) {
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (frame_table[i].is_free) {
            frame_table[i].is_free = 0;
            frame_table[i].process_id = process_id;
            frame_table[i].page_number = page_number;
            return i;
        }
    }
    return -1;  // No free frame found
}

// Function to free a frame
void free_frame(int frame_number) {
    frame_table[frame_number].is_free = 1;
    frame_table[frame_number].process_id = -1;
    frame_table[frame_number].page_number = -1;
}

// Function to load a page from an executable file into a frame
void load_page_from_executable(int process_id, int page_number, int frame) {
    char filename[256];
    snprintf(filename, sizeof(filename), "process_%d_executable.bin", process_id);

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening executable file");
        return;
    }

    off_t offset = page_number * PAGE_SIZE;
    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("Error seeking to page in executable file");
        close(fd);
        return;
    }

    char buffer[PAGE_SIZE];
    if (read(fd, buffer, PAGE_SIZE) != PAGE_SIZE) {
        perror("Error reading page from executable file");
        close(fd);
        return;
    }

    printf("Loading page %d of process %d from executable into frame %d\n", page_number, process_id, frame);
    close(fd);
}

// Function to write a page to swap space
void write_page_to_swap(int process_id, int page_number) {
    char filename[256];
    snprintf(filename, sizeof(filename), "process_%d_swap.bin", process_id);

    int fd = open(filename, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        perror("Error opening swap file");
        return;
    }

    off_t offset = page_number * PAGE_SIZE;
    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("Error seeking to page in swap file");
        close(fd);
        return;
    }

    char buffer[PAGE_SIZE];
    memset(buffer, 0, PAGE_SIZE);  // Clear the buffer before writing

    if (write(fd, buffer, PAGE_SIZE) != PAGE_SIZE) {
        perror("Error writing page to swap file");
        close(fd);
        return;
    }

    printf("Writing page %d of process %d to swap space\n", page_number, process_id);
    close(fd);
}

// Function to check if a page is modified
int is_page_modified(PageTable *pt, int page_number) {
    return pt->entries[page_number].modified;
}

// Function to update the LRU list
void update_lru(int frame_number) {
    lru_list[frame_number].frame_number = frame_number;
    lru_list[frame_number].last_used_time = current_time++;
}

// Function to find the least recently used (LRU) frame
int find_lru_frame() {
    int lru_frame = 0;
    for (int i = 1; i < MAX_FRAMES; i++) {
        if (lru_list[i].last_used_time < lru_list[lru_frame].last_used_time) {
            lru_frame = i;
        }
    }
    return lru_frame;
}

// Function to handle a page fault
void handle_page_fault(int process_id, int page_number, PageTable *pt) {
    int frame = allocate_frame(process_id, page_number);
    if (frame == -1) {
        frame = find_lru_frame();
        int old_process_id = frame_table[frame].process_id;
        int old_page_number = frame_table[frame].page_number;

        if (is_page_modified(&pt[old_process_id], old_page_number)) {
            write_page_to_swap(old_process_id, old_page_number);
        }

        load_page_from_executable(process_id, page_number, frame);
        frame_table[frame].process_id = process_id;
        frame_table[frame].page_number = page_number;
    } else {
        load_page_from_executable(process_id, page_number, frame);
    }
    pt->entries[page_number].valid = 1;
    pt->entries[page_number].frame_number = frame;
    pt->entries[page_number].modified = 0;
    update_lru(frame);
}

// Function to clean up resources for a process
void cleanup_process_resources(int process_id) {
    ProcessResources *resources = &process_resources[process_id];

    // Free dynamically allocated memory blocks
    for (int i = 0; i < resources->num_allocated_blocks; i++) {
        free(resources->allocated_memory[i]);
        resources->allocated_memory[i] = NULL;
    }

    // Close open file descriptors
    for (int i = 0; i < resources->num_open_files; i++) {
        if (resources->open_files[i] != -1) {
            close(resources->open_files[i]);
            resources->open_files[i] = -1;
        }
    }

    // Reset the resource counts
    resources->num_allocated_blocks = 0;
    resources->num_open_files = 0;

    printf("Cleaned up resources for process %d\n", process_id);
}

// Function to terminate a process
void terminate_process(PageTable *pt, int process_id) {
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (frame_table[i].process_id == process_id) {
            free_frame(i);
        }
    }

    free_page_table(pt, process_id);
    cleanup_process_resources(process_id);
}

// Function to free the page table for a process
void free_page_table(PageTable *pt, int process_id) {
    free(pt[process_id].entries);
    pt[process_id].num_entries = 0;
}

// Function to allocate resources for a process
void allocate_resources_for_process(int process_id) {
    ProcessResources *resources = &process_resources[process_id];

    // Simulate allocating memory blocks
    resources->allocated_memory = malloc(10 * sizeof(void *));  // Allocate array for 10 blocks
    resources->num_allocated_blocks = 10;
    for (int i = 0; i < 10; i++) {
        resources->allocated_memory[i] = malloc(1024);  // Allocate 1 KB blocks
    }

    // Simulate opening file descriptors
    resources->num_open_files = 3;
    resources->open_files[0] = open("file1.txt", O_RDONLY | O_CREAT, 0644);
    resources->open_files[1] = open("file2.txt", O_WRONLY | O_CREAT, 0644);
    resources->open_files[2] = open("file3.txt", O_RDWR | O_CREAT, 0644);
}

// Function to set the PATH environment variable
void set_path_environment() {
    setenv("PATH", "/bin:/usr/bin:/usr/local/bin", 1);
}

int main(int argc, char *argv[]) {
    // Set up signal handlers
    signal(SIGINT, handle_signal);
    signal(SIGQUIT, handle_signal);

    set_path_environment();  // Set the PATH environment variable

    init_frame_table();  // Initialize the frame table

    if (argc == 2) {
        batch_mode(argv[1]);  // Run in batch mode if a filename is provided
    } else {
        char input[MAX_INPUT_SIZE];
        PageTable pt[100];
        int process_id = 1;
        int process_memory = 1000000;
        int num_pages = calculate_pages_needed(process_memory);

        init_page_table(&pt[process_id], num_pages);  // Initialize the page table for the process
        allocate_resources_for_process(process_id);  // Allocate resources for the process

        while (1) {
            printf("$lopeShell > ");
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
            execute_commands(input, process_id, &pt[process_id]);  // Execute commands entered by the user
        }

        terminate_process(pt, process_id);  // Terminate the process and free resources
    }

    return 0;
}
