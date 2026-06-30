#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void handler(int sig)
{
    printf("Received signal %d\n", sig);

    switch (sig) {
    case SIGFPE:
        printf("Signal name: SIGFPE\n");
        break;
    case SIGILL:
        printf("Signal name: SIGILL\n");
        break;
    case SIGSEGV:
        printf("Signal name: SIGSEGV\n");
        break;
    }

    _exit(0);   // Exit immediately; don't return to the faulting instruction.
}

int main(int argc, char *argv[])
{
    signal(SIGFPE, handler);
    signal(SIGILL, handler);
    signal(SIGSEGV, handler);

    if (argc != 2) {
        printf("Usage: %s [fpe|ill|segv]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "fpe") == 0) {
        volatile int x = 10;
        volatile int y = 0;
        printf("Triggering divide by zero...\n");
        printf("%d\n", x / y);
    }
    else if (strcmp(argv[1], "ill") == 0) {
        printf("Triggering illegal instruction...\n");
        // Emits a trap instruction, which the OS interprets as an illegal instruction and delivers `SIGILL`
        //  to the process
        __builtin_trap();
    }
    else if (strcmp(argv[1], "segv") == 0) {
        printf("Triggering segmentation fault...\n");
        volatile int *p = NULL;
        *p = 42;
    }
    else {
        printf("Unknown option\n");
    }

    return 0;
}
