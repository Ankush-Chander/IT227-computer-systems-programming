#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

int is_process_in_foreground(void) {
    // 1. Ensure stdin is actually connected to a terminal
    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Not attached to a terminal\n");
        return -1; // Indeterminate
    }

    pid_t my_pgid = getpgrp();
    if (my_pgid == -1) {
        perror("getpgrp");
        return -1;
    }

    pid_t fg_pgid = tcgetpgrp(STDIN_FILENO);
    if (fg_pgid == -1) {
        perror("tcgetpgrp");
        return -1;
    }

    return (my_pgid == fg_pgid) ? 1 : 0; // 1=foreground, 0=background/stopped
}

void handler(int sig)
{
    switch (sig) {
        case SIGINT:
            printf("Caught SIGINT (Ctrl+C) — terminating\n");
            signal(SIGINT, SIG_DFL);  // Reset to default action
            raise(SIGINT);            // Now this will kill the process
            _exit(0);

        case SIGTSTP:
            printf("Caught SIGTSTP (Ctrl+Z) — suspending\n");
            fflush(stdout);
            signal(SIGTSTP, SIG_DFL);  // Reset to default action
            raise(SIGTSTP);            // Now this will suspend the process
            // The line below runs when resumed by SIGCONT
            printf("\nResumed from background.\n");
            return;
        case SIGCONT:
            printf("Caught SIGCONT — resuming\n");
            // return;

        default:
            printf("Caught unknown signal %d\n", sig);
            _exit(1);
    }
}

int main(int argc, char *argv[])
{
    // take argument to differentiate instance
    int instance = (argc > 1) ? atoi(argv[1]) : 0;

    signal(SIGINT, handler);
    signal(SIGTSTP, handler);

    while (1)
    {
        pid_t fg = getpgrp();
        // getpgrp: Get the process group ID of the calling process.
        if (is_process_in_foreground())
        {
            printf("foreground: Instance %d,  PID = %d, PGID = %d\n", instance, getpid(), fg);
        }
        else
        {
            printf("background: Instance %d,  PID = %d, PGID = %d\n", instance, getpid(), fg);
        }

        printf("Running...\n");
        sleep(1);
    }
}
