#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE (1024 * 1024 * 1024 / sizeof(int))  // 1 GB array

int main() {
    int *large_array = (int *)malloc(ARRAY_SIZE * sizeof(int));
    if (large_array == NULL) {
        perror("Failed to allocate memory");
        return 1;
    }

    // Access each page of the array to trigger page faults
    for (int i = 0; i < ARRAY_SIZE; i += 1024) {
        large_array[i] = i;
        printf("Accessed array[%d] = %d\n", i, large_array[i]);
    }

    free(large_array);
    return 0;
}
