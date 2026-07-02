## Lecture 14- Process Control

### Different ways of process creation
- System boot – During system booting several background processesor daemons are started  
- User Request – When command is executed on CLI shell or double-click on application  
- A process can spawn another i.e. child process using fork() e.g. server can create new process for each client request or init daemon waits for login and spawns new shell  
-  Batch system takes next job to process  


### Obtaining Process IDs
```c
#include <sys/types.h>
#include <unistd.h>
pid_t getpid(void);
pid_t getppid(void);
```

Refer `code/process_control/pid_demo.c`

### Creating and Terminating Processes

#### `fork()` System Call

`fork()` creates a new process.

```c
pid_t pid = fork();
```

After fork:

```text
Parent
   |
   +---- Child
```

The child receives:

* Copy of code
* Copy of data
* Copy of stack
* Copy of open file descriptors

but

* Different PID

---




| Property                                  | Explanation                                                                                                                                                                                                   |
| ----------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Called once, returns twice**            | `fork()` is invoked once by the parent but returns **twice**: once in the parent (returns child's PID) and once in the child (returns `0`).                                                                   |
| **Creates concurrent processes**          | After `fork()`, the parent and child execute **independently and concurrently**. The kernel may schedule them in any order, so execution order is **non-deterministic**.                                      |
| **Duplicate but separate address spaces** | The child starts with a copy of the parent's address space (stack, heap, globals, code, etc.), but each process has its **own private memory**. Changes made by one process are **not visible** to the other. |
| **Open files are inherited**              | The child inherits all open file descriptors from the parent. For example, if `stdout` is connected to the terminal, both processes write to the same terminal.                                               |


---

#### Fork Visualization

Before:

```text
Process P
```

After:

```text
        P
       /
fork()
     /
    C
```

One call to `fork()` creates:

```text
2 processes
```

Two forks:

```c
fork();
fork();
```

creates:

```text
4 processes
```

Three forks:

```c
fork();
fork();
fork();
```

creates:

```text
8 processes
```

General rule:

```text
n forks → 2^n processes
```

## Reaping Child Processes

### What is Reaping?

**Reaping** is the process by which a parent collects the termination status of one of its child processes by calling a member of the `wait()` family (typically `wait()` or `waitpid()`).

Once the parent reaps the child, the kernel removes the child's remaining process table entry, completely cleaning up the process.

![](https://i.sstatic.net/R53RN.png)

---

### Why is Reaping Necessary?

When a child process terminates, it does **not** disappear immediately.

Instead:

- The child stops executing.
- Most of its resources are released:
  - Virtual memory
  - Open file descriptors
  - Other kernel resources
- However, **its process table entry is intentionally retained**.

The kernel preserves this entry so that the parent can later retrieve information such as:

- Process ID (PID)
- Exit status
- Termination signal
- Resource usage (on some systems)

This remaining process is known as a **zombie process**.

> A zombie is **not** a running process—it is simply a small record kept by the kernel until its parent acknowledges the child's termination.

---

### Waiting for a Child

A parent waits for one or more child processes using one of the `wait()` family functions. The most commonly used is `waitpid()`.

```c
#include <sys/types.h>
#include <sys/wait.h>

pid_t waitpid(pid_t pid, int *status, int options);
/* Returns:
 *   >0 : PID of terminated child
 *    0 : No child exited yet (when WNOHANG is used)
 *   -1 : Error
 */
```

Calling `waitpid()` both:

1. Retrieves the child's termination status.
2. Reaps the child by removing its process table entry.

---

### What Happens If the Parent Never Reaps?

| Parent's Action | Result |
|-----------------|--------|
| Calls `wait()` or `waitpid()` | Child is reaped; kernel removes its process table entry. |
| Never calls `wait()` | Child remains a **zombie**, occupying a process table slot indefinitely. |
| Parent exits before reaping | The zombie is adopted by **`init` (PID 1)** (or another designated subreaper such as `systemd` on many Linux systems), which eventually calls `wait()` and reaps it. |

---

### Key Points

- A terminated child is **not immediately destroyed**.
- The kernel keeps a minimal process entry until the parent collects its exit status.
- Reaping is performed using `wait()` or `waitpid()`.
- Failure to reap children creates **zombie processes**.
- If the parent terminates first, the child is adopted by `init` (or another subreaper), ensuring it is eventually reaped.

---

| Scenario                          | Parent waits for child?         | Library / System Calls                                      | Child survives parent?                         | Typical examples                       |
| --------------------------------- | ------------------------------- | ----------------------------------------------------------- | ---------------------------------------------- | -------------------------------------- |
| **Synchronous child execution**   | Yes                           | `wait()`, `waitpid(pid, &status, 0)`                         | Usually parent stays alive until child exits | Shell commands, compilers, build tools |
| **Long-running supervised child** | Usually reaped asynchronously | `waitpid(-1, NULL, WNOHANG)`, `sigaction(SIGCHLD, handler)` | Often terminated with parent                 | Web servers, browsers, IDEs            |
| **Detached/background child**     | No                            | Double-fork + `setsid()`, or simply omitting any `wait` call | Yes                                          | Daemons, services, background jobs     |


### Example

See:

```text
../code/process_control/reap_demo.c
../code/process_control/reap_demo2.c
```



<!--

### Putting Processes to Sleep

### Loading and Running Programs

### Using fork and execve to Run Programs-->
