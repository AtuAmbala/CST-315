// This program is a simple virus detector that scans files in a directory for a specific string.
// It is used to demonstrate how a virus detector works.
//
// Programmers: Atuhaire Ambala and Ricardo Escarcega

#include <stdio.h>
#include <dirent.h>
#include <string.h>

const char *exceptions[] = {"virus_detector.c", NULL};

int is_exception(const char *filename) {
    for (int i = 0; exceptions[i] != NULL; i++) {
        if (strstr(filename, exceptions[i]) != NULL) {
            return 1;
        }
    }
    return 0;
}

void scanFile(const char *filename) {
    if (is_exception(filename)) {
        return;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "--rf *")) {
            printf("Warning: file %s is infected!\n", filename);
            break;
        }
    }

    fclose(file);
}

void scanDirectory(const char *directory) {
    DIR *dir = opendir(directory);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    char path[1024];
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG) { // If it's a regular file
            snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
            scanFile(path);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    scanDirectory(argv[1]);

    return 0;
}
