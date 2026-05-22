// demo.c — exercises all 6 kernel responsibilities
// Compile: gcc -o demo demo.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <fcntl.h>

int main() {
    // --- 2) MEMORY MANAGEMENT (mmap/munmap) ---
    printf("[parent] Requesting virtual memory from kernel...\n");
    char *mem = (char *)mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == (void *)-1) { perror("mmap"); return 1; }
    strcpy(mem, "Hello from virtual memory!");
    printf("[parent] Got memory at %p -> \"%s\"\n", mem, mem);

    // --- 3) FILE SYSTEM (open/read/write/close) ---
    printf("[parent] Asking kernel to create a file...\n");
    int fd = open("/tmp/demo_kernel.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    const char *msg = "Written via kernel syscall!";
    write(fd, msg, strlen(msg));
    close(fd);

    // --- 4) PROCESS CREATION (fork) ---
    printf("[parent] Asking kernel to clone a new process...\n");
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // --- Child process ---
        printf("[child  pid=%d] Kernel scheduled me!\n", getpid());
        sleep(1);
        // --- 5) NETWORKING (socket/connect/send) ---
        printf("[child] Asking kernel to send a network packet...\n");
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(80);
        addr.sin_addr.s_addr = inet_addr("93.184.216.34");   // example.com
        connect(sock, (struct sockaddr *)&addr, sizeof(addr));
        const char *req = "GET / HTTP/1.0\r\nHost: example.com\r\n\r\n";
        send(sock, req, strlen(req), 0);
        char buf[256] = {0};
        recv(sock, buf, sizeof(buf) - 1, 0);
        printf("[child] Kernel delivered %zu bytes from network\n", strlen(buf));
        close(sock);

        // --- 4b) PROCESS TERMINATION ---
        printf("[child] Done. Kernel will reclaim my resources.\n");
        _exit(0);
    }

    // --- 1) PROCESS SCHEDULING (parent/child interleaved by kernel) ---
    // The kernel decides whether parent or child runs next.
    // wait() is a syscall — kernel notifies parent when child exits.
    printf("[parent] Kernel will schedule parent/child as it sees fit.\n");
    int status;
    wait(&status);
    printf("[parent] Kernel told me child exited with status %d\n", status);

    // --- cleanup ---
    munmap(mem, 4096);
    printf("[parent] All done. Kernel managed scheduling, memory, files, processes, and network.\n");
    return 0;
}
