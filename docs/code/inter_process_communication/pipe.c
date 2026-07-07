#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Returns a human-readable process state string for the given PID.
 * Reads /proc/<pid>/status and parses the "State:" line.
 */
const char* get_process_state(pid_t pid)
{
    static const char *result = "UNKNOWN"; /* static so we can return a pointer */

    char status_path[64];
    snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);

    FILE *f = fopen(status_path, "r");
    if (!f)
        return "UNREADABLE";

    char line[256];
    char state_char = '?';

    while (fgets(line, sizeof(line), f)) {
        char *p = strstr(line, "State:");
        if (p) {
            p += 6;                       /* skip past "State:" */
            while (*p == ' ' || *p == '\t') p++; /* skip whitespace */
            state_char = *p;
            break;
        }
    }
    fclose(f);

    switch (state_char) {
        case 'R': result = "RUNNING";                              break;
        case 'S': result = "SLEEPING (interruptible)";             break;
        case 'D': result = "SLEEPING (uninterruptible / I/O wait)";break;
        case 'T': result = "STOPPED";                              break;
        case 't': result = "TRACE STOP";                           break;
        case 'Z': result = "ZOMBIE";                               break;
        default:  result = "UNKNOWN";                              break;
    }

    return result;
}

int main(void)
{
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }
    printf("pipe created, fd[0]=%d, fd[1]=%d\n", pipefd[0], pipefd[1]);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        /* --- Child Process: Write --- */
        close(pipefd[0]); // Close unused read end

        pid_t parent_pid = getppid();

        for (int i = 0; i < 10; i++) {
            sleep(1);
            printf("[Child] Parent %d is currently: %s\n",
                   parent_pid, get_process_state(parent_pid));
        }

        write(pipefd[1], "Hello from child!", 17);
        close(pipefd[1]);
        exit(0);
    } else {
        /* --- Parent Process: Read --- */
        close(pipefd[1]); // Close unused write end (triggers EOF when child exits)
        char buffer[1024] = {0};
        printf("Parent #%d waiting for child to write in pipe...\n", getpid());
        // read child status while its sleeping

        read(pipefd[0], buffer, sizeof(buffer));
        printf("Parent read from child: %s \n", buffer);
        sleep(2);
        printf("[Parent] Child is currently: %s\n", get_process_state(pid));
    }

    return 0;
}
