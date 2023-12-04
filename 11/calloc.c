#include <stdio.h>
#include <stdlib.h>

int main() {
    int n = 5;
    int *arr = (int*) calloc(n, sizeof(int));
    // int *arr = (int*) malloc(n);

    if (arr == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]); // Prints 0, as calloc initializes all elements to zero
    }

    free(arr); // Don't forget to free the memory
    return 0;
}