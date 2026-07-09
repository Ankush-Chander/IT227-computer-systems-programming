#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    pid_t pid1, pid2;

    // 1. Create the pipe BEFORE forking
    // pipefd[0] is the Read end
    // pipefd[1] is the Write end
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    // 2. Create Child 1 (The Writer: e.g., "ls -l")
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
        char *cmd1[] = {"ls", "-l", NULL};
        execvp(cmd1[0], cmd1);

        // execvp only returns if it fails
        perror("execvp ls failed");
        exit(EXIT_FAILURE);
    }

    // 3. Create Child 2 (The Reader: e.g., "wc -l")
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
        char *cmd2[] = {"wc", "-l", NULL};
        execvp(cmd2[0], cmd2);

        // execvp only returns if it fails
        perror("execvp wc failed");
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

    return 0;
}
