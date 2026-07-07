#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main() {
    int p2c[2]; // Pipe 1: Parent -> Child
    int c2p[2]; // Pipe 2: Child -> Parent

    if (pipe(p2c) == -1 || pipe(c2p) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        /* --- Child Process --- */
        close(p2c[1]); // Close unused write end of parent->child pipe
        close(c2p[0]); // Close unused read end of child->parent pipe

        // 1. Read message from Parent
        char recv_buf[1024] = {0};
        ssize_t n = read(p2c[0], recv_buf, sizeof(recv_buf));
        if (n > 0) printf("Child %d received: %s\n", getpid(), recv_buf);

        // Simulate some work before replying
        sleep(1); 

        // 2. Send reply back to Parent
        const char *reply = "Hello from child!";
        write(c2p[1], reply, strlen(reply));

        close(p2c[0]);
        close(c2p[1]);
        exit(0);
    } else {
        /* --- Parent Process --- */
        close(p2c[0]); // Close unused read end of parent->child pipe
        close(c2p[1]); // Close unused write end of child->parent pipe

        printf("Parent %d sending message to child...\n", getpid());

        // 1. Send initial message to Child
        const char *msg = "Hello from parent!";
        write(p2c[1], msg, strlen(msg));

        // 2. Wait for reply from Child
        char recv_buf[1024] = {0};
        printf("Parent %d waiting for child reply...\n", getpid());
        ssize_t n = read(c2p[0], recv_buf, sizeof(recv_buf));
        if (n > 0) printf("Parent received: %s\n", recv_buf);

        close(p2c[1]);
        close(c2p[0]);
    }

    return 0;
}
