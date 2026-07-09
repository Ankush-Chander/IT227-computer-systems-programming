#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* ---- Minimal  wrappers ---- */

static void unix_error(const char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    _exit(1);
}

static pid_t Fork(void)
{
    pid_t pid;

    if ((pid = fork()) < 0)
        unix_error("Fork error");

    return pid;
}

/* ---- Program starts here ---- */

#define N 10

int main(void)
{
    int status, i;
    pid_t myid = getpid();
    pid_t pid;

    /* Self-documenting aliases for waitpid arguments */
    const pid_t WAIT_FOR_ANY_CHILD  = -1;   /* which child to reap (wildcard) */
    const int   WAIT_BLOCKING       = 0;    /* how to wait (block until one dies) */
    pid_t ppid = getppid();

    /* Parent creates N children */
    for (i = 0; i < N; i++)
        if ((pid = Fork()) == 0){ /* Child */
            pid_t cid = getpid();
            printf("child: %d",cid);
             sleep(2 + i);
             pid_t ppid = getppid();
             printf("child:%d with parent %d exiting with status: %d\n", cid, ppid, 100+i);
             exit(100 + i);
        }else{
            if (pid%2==0){
                kill(pid, SIGINT);
            }
        }
    /* Parent reaps N children in no particular order */
    // "Block until any child dies, then tell me which one and why."
    while ((pid = waitpid(WAIT_FOR_ANY_CHILD, &status, WAIT_BLOCKING)) > 0) {
        if (WIFEXITED(status))
            printf("child %d terminated normally and reaped with exit status=%d\n",
                   pid, WEXITSTATUS(status));
        else if (WIFSIGNALED(status)){
            printf("child %d terminated by signal #%d\n", pid, WTERMSIG(status));
        }
        else
            printf("child %d terminated abnormally\n", pid);
    }

    /* The only normal termination is if there are no more children */
    // if (errno != ECHILD)
    //     unix_error("waitpid error");
    sleep(5);
    printf("Parent %d terminated normally", myid);
    exit(0);
}
