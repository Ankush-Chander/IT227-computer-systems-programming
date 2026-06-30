## Lecture 13 - Signal handling basics

A signal is a small message that notifies a process that some event has occurred in the system.

```bash
# gives you list of signals
man 7 signal
```

**Why need signals?**  
Low-level exception handling is done by the kernel. Signals are used to notify processes of events such as errors, interrupts, and other system events.


**Hardware Exception Signals**

| Signal | Number | Event |
|--------|--------|-------|
| `SIGFPE` | 8 | Arithmetic error (e.g., divide by zero) |
| `SIGILL` | 4 | Illegal instruction |
| `SIGSEGV` | 11 | Invalid memory reference |

**Software-Generated Signals**

| Signal | Number | Event |
|--------|--------|-------|
| `SIGINT` | 2 | User presses `Ctrl+C` (terminal interrupt) |
| `SIGKILL` | 9 | Forceful termination of a process |
| `SIGCHLD` | 17 | Child process terminates or stops |  


## Signal lifecycle

Signal delivery occurs in **two distinct steps**:

| Step | Description |
|------|-------------|
| **Send (Deliver)** | The kernel marks a signal as pending for a process. A signal may be sent because (1) the kernel detects an event (e.g., divide-by-zero, segmentation fault, child termination), or (2) a process explicitly requests it using `kill()`. A process can also send a signal to itself. |
| **Receive** | The kernel forces the destination process to react. The process may **ignore** the signal, **terminate**, or **catch** it by executing a **signal handler**. |

### Signal Handler Flow

```text
Program executing
        │
Signal delivered
        │
        ▼
Kernel transfers control
        │
        ▼
Signal handler executes
        │
Handler returns
        │
        ▼
Program resumes at the next instruction
```

**Why do you need kernel intervention even though the process is executing user code(handler code)?**

Because there is no explicit jump to handler or back to user code after executing a signal handler written by the user. So the kernel must intervene.
In case of user-defined signal handlers:  

- The kernel sets the PC to the address of the handler and transfers control to it.  
- After the handler has executed, the kernel must intervene to return control to the user code.  

---

## Pending and Blocked Signals

| Term               | Meaning                                                                |
| ------------------ | ---------------------------------------------------------------------- |
| **Pending signal** | A signal that has been delivered but not yet received.                 |
| **Blocked signal** | A signal whose delivery is temporarily deferred until it is unblocked. |

### Important properties

* At most **one pending signal of a given type** can exist for a process.
* If the same signal is delivered again while already pending, the new signal is **discarded** (standard Unix signals are **not queued**).
* A pending signal is received **at most once**.
* The kernel maintains two bit vectors per process:

| Bit Vector | Purpose                                             |
| ---------- | --------------------------------------------------- |
| `pending`  | Indicates which signal types are pending.           |
| `blocked`  | Indicates which signal types are currently blocked. |

---

## Process Groups

A **process group** is a collection of related processes that can receive signals together.

### Properties

* Every process belongs to **exactly one** process group.
* Each process group has a **Process Group ID (PGID)**.
* By default, a child inherits its parent's process group.
* A process can create a new process group (typically using setpgid()), in which case it becomes the group leader and its PID becomes the PGID.
* Note that the PGID may or may not be the same as the PID. In the case of a process group leader, the PGID is the same as the PID.

```bash
sleep 1000 | grep cat
# press Ctrl+Z to send SIGTSTP to the process

ps -o pid,ppid,pgid,stat,cmd
# output

# PID    PPID    PGID STAT CMD
# 13858   10136   13858 Ss   /bin/bash
# 132800   13858  132800 T    sleep 1000
# 132801   13858  132800 T    grep --color=auto cat
# 132863   13858  132863 R+   ps -o pid,ppid,pgid,stat,cmd
```

### APIs

| Function             | Purpose                                 |
| -------------------- | --------------------------------------- |
| `getpgrp()`          | Returns the calling process's PGID.     |
| `setpgid(pid, pgid)` | Changes the process group of a process. |

Example:

```c
setpgid(0, 0);
```

Creates a new process group whose PGID equals the calling process's PID and makes the calling process its leader.

---

## Ways to Send Signals

### 1. `/bin/kill` command

| Command          | Effect                                                         |
| ---------------- | -------------------------------------------------------------- |
| `kill -9 15213`  | Sends `SIGKILL` to process `15213`.                            |
| `kill -9 -15213` | Sends `SIGKILL` to **every process** in process group `15213`. |

**Rule:** A **negative PID** refers to a **process group**, not an individual process.

---

### 2. `kill()` system call

```c
int kill(pid_t pid, int sig);
```

| `pid` value | Effect                                                    |
| ----------- | --------------------------------------------------------- |
| `pid > 0`   | Send signal to the specified process.                     |
| `pid < 0`   | Send signal to every process in process group `abs(pid)`. |

A process may also use `kill()` to signal itself.

---

### 3. Keyboard-generated signals

The shell organizes commands into **jobs**.

* **One foreground job** at a time.
* **Zero or more background jobs**.

Each job is assigned its own **process group**.

| Keyboard | Signal    | Shell action                                                                                  |
| -------- | --------- | --------------------------------------------------------------------------------------------- |
| `Ctrl+C` | `SIGINT`  | Sends `SIGINT` to every process in the **foreground process group** (default: terminate).     |
| `Ctrl+Z` | `SIGTSTP` | Sends `SIGTSTP` to every process in the **foreground process group** (default: stop/suspend). |

Background jobs do **not** receive these keyboard-generated signals because they are not in the terminal's foreground process group.

---

## Key Takeaways

| Concept                | Summary                                                             |
| ---------------------- | ------------------------------------------------------------------- |
| Signal delivery        | Two-step process: **send** → **receive**.                           |
| Pending signals        | One pending instance per signal type; duplicates are discarded.     |
| Blocked signals        | Delivery is deferred until unblocked.                               |
| Process groups         | Signals can target an entire group of related processes.            |
| Negative PID in `kill` | Refers to a process group.                                          |
| Shell job control      | `Ctrl+C` and `Ctrl+Z` affect only the **foreground process group**. |

---
