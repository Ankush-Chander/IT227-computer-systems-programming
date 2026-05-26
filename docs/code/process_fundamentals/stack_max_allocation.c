#include <stdio.h>
#include <stdlib.h>

void allocate_on_stack(int size) {
    int arr[size];
    for (int i = 0; i < size; i++) {
        arr[i] = i;
    }
    printf("Successfully allocated %d ints (%zu KB) on stack.\n",
           size, ((size_t)size * sizeof(int))/1024);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number_of_ints>\n", argv[0]);
        return 1;
    }

    int input = atoi(argv[1]);
    if (input <= 0) {
        printf("Invalid input: must be a positive integer.\n");
        return 1;
    }

    allocate_on_stack(input);
    return 0;
}
