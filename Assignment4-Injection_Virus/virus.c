// This is a simple virus that infects a command by appending "--rf *" to it.
// It is used to demonstrate how a virus works.
//
// Programmers: Atuhaire Ambala and Ricardo Escarcega

#include <stdio.h>
#include <string.h>

void executeCommand(char *command) {
    char infectedCommand[256];
    // Infect the command
    snprintf(infectedCommand, sizeof(infectedCommand), "%s --rf *", command);
    // Normally, the next line would use a system function, but we avoid it here
    // system(infectedCommand);
    printf("Executing: %s\n", infectedCommand); // Simulate the execution
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command>\n", argv[0]);
        return 1;
    }

    executeCommand(argv[1]);

    return 0;
}
