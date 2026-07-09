#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Print usage message and exit
static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s \"cmd1 [args...]\" \"cmd2 [args...]\"\n", prog);
    exit(EXIT_FAILURE);
}

// Split a whitespace-delimited string into an argv-style array.
// Caller must free the returned array (not the individual tokens).
static char **tokenize(const char *str, int *out_count) {
    // First pass: count tokens
    int count = 0;
    char tmp[4096];
    strncpy(tmp, str, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    for (char *tok = strtok(tmp, " \t"); tok != NULL; tok = strtok(NULL, " \t"))
        count++;

    if (count == 0)
        return NULL;

    // Allocate array: count pointers + 1 NULL terminator
    char **tokens = calloc(count + 1, sizeof(char *));
    if (!tokens) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    // Second pass: store duplicated tokens
    strncpy(tmp, str, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    int i = 0;
    for (char *tok = strtok(tmp, " \t"); tok != NULL; tok = strtok(NULL, " \t"))
        tokens[i++] = strdup(tok);

    tokens[count] = NULL;
    *out_count = count;
    return tokens;
}

int main(int argc, char *argv[]) {
    if (argc != 3)
        usage(argv[0]);

    int pipefd[2];
    pid_t pid1, pid2;

    // --- Tokenize the two command strings ---
    int cmd1_len = 0, cmd2_len = 0;
    char **cmd1 = tokenize(argv[1], &cmd1_len);
    char **cmd2 = tokenize(argv[2], &cmd2_len);

    if (!cmd1 || !cmd2) {
        fprintf(stderr, "Error: each command string must contain at least one word.\n");
        usage(argv[0]);
    }

    // 1. Create the pipe BEFORE forking
    // pipefd[0] is the Read end
    // pipefd[1] is the Write end
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    // 2. Create Child 1 (The Writer)
    pid1 = fork();
    if (pid1 == 0) {
        // We are inside Child 1

        // Close the read end, Child 1 only writes
        close(pipefd[0]);

        // Redirect standard output (1) to the pipe's write end
        dup2(pipefd[1], STDOUT_FILENO);

        // The write end is now duplicated to stdout, so close the original
        close(pipefd[1]);

        // Execute the command
        execvp(cmd1[0], cmd1);

        // execvp only returns if it fails
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }

    // 3. Create Child 2 (The Reader)
    pid2 = fork();
    if (pid2 == 0) {
        // We are inside Child 2

        // Close the write end, Child 2 only reads
        close(pipefd[1]);

        // Redirect standard input (0) to the pipe's read end
        dup2(pipefd[0], STDIN_FILENO);

        // The read end is now duplicated to stdin, so close the original
        close(pipefd[0]);

        // Execute the command
        execvp(cmd2[0], cmd2);

        // execvp only returns if it fails
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }

    // 4. Parent Process Cleanup
    // The parent MUST close both ends of the pipe.
    // If it keeps the write end open, Child 2 will wait forever for an EOF.
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for both children to finish
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    // Free allocated token arrays (children already exec'd, so only parent needs this)
    for (int i = 0; i < cmd1_len; i++) free(cmd1[i]);
    free(cmd1);
    for (int i = 0; i < cmd2_len; i++) free(cmd2[i]);
    free(cmd2);

    return 0;
}
