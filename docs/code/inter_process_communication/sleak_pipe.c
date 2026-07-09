#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s \"cmd1\" \"cmd2\"\n", argv[0]);
        return EXIT_FAILURE;
    }

    // cmd1's stdout becomes readable to us
    FILE *fp_read = popen(argv[1], "r");
    // we write directly to cmd2's stdin
    FILE *fp_write = popen(argv[2], "w");

    if (!fp_read || !fp_write) {
        perror("popen failed");
        return EXIT_FAILURE;
    }

    char buf[4096];
    size_t n;
    // Copy data through the pipe
    while ((n = fread(buf, 1, sizeof(buf), fp_read)) > 0)
        fwrite(buf, 1, n, fp_write);

    pclose(fp_write);  // Close cmd2's stdin, wait for it to finish
    pclose(fp_read);   // Wait for cmd1 to finish

    return EXIT_SUCCESS;
}
