#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

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

    pid_t fg = tcgetpgrp(STDIN_FILENO);
    // printf("Foreground# %d  PGID = %d\n", instance, fg);
    printf("Instance %d,  PID = %d, PGID = %d\n", instance, getpid(), fg);

    printf("Stopping...\n");

    printf("I'm back!\n");

    while (1)
    {
        pid_t fg = tcgetpgrp(STDIN_FILENO);
        if (fg == getpgrp())
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
