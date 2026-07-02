#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define N 10

pid_t children[N];  /* Track every child's PID */

void handler(int sig)
{
    // printf("Received signal %d\n", sig);

    switch (sig) {
    case SIGFPE:
        printf("Signal name: SIGFPE\n");
        break;
    case SIGINT:
        // kill all children
        for (int i = 0; i < N; i++){
            printf("killing child %d\n", children[i]);
            kill(children[i], SIGTERM);
        }
        printf("Signal name: SIGINT\n");
        break;
    case SIGSEGV:
        printf("Signal name: SIGSEGV\n");
        break;
    }

    _exit(0);   // Exit immediately; don't return to the faulting instruction.
}

int main(void)
{
    int i;
    pid_t pid;

    /* Parent creates N children and records their PIDs */
    for (i = 0; i < N; i++) {
        if ((pid = fork()) == 0) {            /* Child */
            sleep(i+1);                       /* Simulate long-running work */
            printf("child %d exiting with status: %d\n", getpid(), 100 + i);

            exit(100 + i);
        }
        children[i] = pid;                   /* Parent saves PID */
    }

    // add handler for SIGTERM
    signal(SIGINT, handler);

    // wait for all children to exit
    while(waitpid(-1, NULL, 0)>0);

    printf("Parent %d exiting now — cleaning up all children\n", getpid());


    exit(0);
}
