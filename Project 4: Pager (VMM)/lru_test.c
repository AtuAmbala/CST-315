#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 4096
#define NUM_PAGES 10
#define ARRAY_SIZE (NUM_PAGES * PAGE_SIZE / sizeof(int))

void access_page(int *array, int page) {
    int index = page * PAGE_SIZE / sizeof(int);
    array[index] = page;
    printf("Accessed page %d (array[%d]) = %d\n", page, index, array[index]);
}

int main() {
    int *large_array = (int *)malloc(ARRAY_SIZE * sizeof(int));
    if (large_array == NULL) {
        perror("Failed to allocate memory");
        return 1;
    }

    // Access pages in a pattern to test LRU
    for (int i = 0; i < NUM_PAGES; i++) {
        access_page(large_array, i);
    }

    // Access pages to trigger LRU replacements
    for (int i = 0; i < NUM_PAGES; i++) {
        access_page(large_array, (i + 1) % NUM_PAGES);
    }

    free(large_array);
    return 0;
}
