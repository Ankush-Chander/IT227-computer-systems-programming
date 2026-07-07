// Use pipe as a synchronization tool to synchronize child processes

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#define N 5

int main(void)
{
    int sync_pipe[2];
    if (pipe(sync_pipe) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t children[N];

    /* --- Spawn N children --- */
    for (int i = 0; i < N; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            close(sync_pipe[0]); // Child only writes

            /* Simulate per-child initialization work */
            sleep(i);
            printf("[Child %d] Initialization done (%ds), signaling parent...\n",
                   getpid(), i);

            /* Signal "I'm ready" by writing a single byte */
            char ready = 1;
            int my_pid = getpid();
            // if(my_pid%2==0){
                write(sync_pipe[1], &ready, 1);
                close(sync_pipe[1]);
                /* Continue with actual work after sync point */
                printf("[Child %d] Synchronized! Starting main work...\n", getpid());

            // }else{
            //     sleep(10);
            // }

            exit(0);
        } else {
            children[i] = pid;
        }
    }

    /* --- Parent synchronization barrier --- */
    close(sync_pipe[1]); // Parent only reads
    printf("Parent %d waiting for all %d children to signal readiness...\n", getpid(), N);

    int count = 0;
    char buf;
    while (read(sync_pipe[0], &buf, 1) == 1) {
        count++;
        printf("[Parent] Received signal #%d from a child\n", count);
    }

    if (count == N)
        printf("Parent: All %d children are ready! Proceeding...\n", N);
    else
        printf("Parent: Only received %d/%d signals\n", count, N);

    close(sync_pipe[0]);

    /* Reap children */
    for (int i = 0; i < N; i++)
        waitpid(children[i], NULL, 0);

    return 0;
}
