#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define NUM_BLOCKS 10
#define BLOCK_SIZE 1024

int main() {
    // Allocate memory blocks
    void *memory_blocks[NUM_BLOCKS];
    for (int i = 0; i < NUM_BLOCKS; i++) {
        memory_blocks[i] = malloc(BLOCK_SIZE);
        if (memory_blocks[i] == NULL) {
            perror("Failed to allocate memory");
            return 1;
        }
        printf("Allocated memory block %d\n", i);
    }

    // Open files
    int file_descriptors[3];
    file_descriptors[0] = open("file1.txt", O_RDONLY | O_CREAT, 0644);
    file_descriptors[1] = open("file2.txt", O_WRONLY | O_CREAT, 0644);
    file_descriptors[2] = open("file3.txt", O_RDWR | O_CREAT, 0644);

    for (int i = 0; i < 3; i++) {
        if (file_descriptors[i] == -1) {
            perror("Failed to open file");
            return 1;
        }
        printf("Opened file descriptor %d\n", file_descriptors[i]);
    }
    // Keep the process alive to allow for manual termination
    printf("Process PID: %d\n", getpid());
    sleep(60);  // Sleep for 60 seconds to allow for manual termination

    // Cleanup (this will not be reached if the process is terminated externally)
    for (int i = 0; i < NUM_BLOCKS; i++) {
        free(memory_blocks[i]);
        printf("Freed memory block %d\n", i);
    }
    for (int i = 0; i < 3; i++) {
        close(file_descriptors[i]);
        printf("Closed file descriptor %d\n", file_descriptors[i]);
    }
    return 0;
}
