# Kernel Syscall Demo — strace Walkthrough

## The Program: `demo.c`

A single C program that touches all 6 kernel responsibilities.

Compile and trace:
```bash
gcc -o demo demo.c
strace -f -t -TT -e trace=process,memory,file,network,ipc ./demo
```

### 1) PROCESS SCHEDULING — `clone` + interleaving

```
11:17:19 clone(child_stack=NULL, flags=CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLD, ...) = 71937
[pid 71937] 11:17:19 socket(AF_INET, SOCK_STREAM, IPPROTO_IP) = 3
```

**Note:** The parent calls `fork()` (→ `clone` syscall). At that moment the kernel owns both processes and decides which one runs next. Notice how the child's syscalls (`socket`, `connect`, etc.) appear interleaved with the parent's `wait4` — that's the scheduler in action. Neither process controls this.

---

### 2) MEMORY MANAGEMENT — `mmap` / `munmap`

```
11:17:19 mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x795e2d9e1000
...
11:17:19 munmap(0x795e2d9e1000, 4096)   = 0
```

**Note:** The process asks for 4096 bytes. The kernel maps a virtual address range — no physical RAM is touched until the first write. When done, `munmap` returns the memory to the kernel to reuse. Also scroll higher in the output — the dynamic linker called `mmap` dozens of times to load `libc.so.6` into virtual memory. That's the kernel giving each process its own isolated address space.

---

### 3) FILE SYSTEM — `openat` / `write` / `close`

```
11:17:19 openat(AT_FDCWD, "/tmp/demo_kernel.txt", O_WRONLY|O_CREAT|O_TRUNC, 0644) = 3
11:17:19 write(3, "Written via kernel syscall!", 25) = 25
11:17:19 close(3)                       = 0       ← (inherited from write completion)
```

**Note:** The process never talks to the disk. It hands a filename to the kernel via `openat`, gets back a file descriptor (`3`), and `write`/`close` operate on that fd. The kernel translates this to block operations on whatever storage backs `/tmp`.

---

### 4) PROCESS CREATION & TERMINATION — `clone` + `exit_group` + `wait4`

```
  CREATE:   clone(... CLONE_CHILD_SETTID|SIGCHLD ...) = 71937

  SCHEDULE: wait4(-1, ...) = ...                       ← parent blocks, kernel schedules child
  [pid 71937] socket(...) / connect(...) / sendto(...) / recvfrom(...)  ← child runs

  TERMINATE: [pid 71937] exit_group(0) = ?
             [pid 71937] +++ exited with 0 +++

  REAP:     wait4 resumed [{WIFEXITED(s) && WEXITSTATUS(s) == 0}], ...) = 71937
            --- SIGCHLD {si_signo=SIGCHLD, si_pid=71937, si_status=0} ---
```

**Note:** The kernel:
- Allocates a new process table entry and copies page tables (`clone`/fork)
- Runs the child to completion
- On `exit_group`, reclaims the child's memory, fds, and PID
- Signals the parent via `SIGCHLD` and unblocks `wait4`

---

### 5) NETWORKING — `socket` / `connect` / `sendto` / `recvfrom`

```
[pid 71937] socket(AF_INET, SOCK_STREAM, IPPROTO_IP)  = 3
[pid 71937] connect(3, {AF_INET, port=80, addr=93.184.216.34}, 16) = 0
[pid 71937] sendto(3, "GET / HTTP/1.0\r\nHost: example.co"..., 37, 0, ...) = 37
[pid 71937] recvfrom(3, "HTTP/1.1 200 OK\r\nDate: Fri, 22 M"..., 255, 0, ...) = 255
```

**Note:** The child never touches the NIC, IP stack, or TCP state machine. It calls `socket()`
to get a network fd, `sendto()` to hand bytes to the kernel, and `recvfrom()` to get response bytes back.
The kernel assembles IP packets, manages TCP handshakes, segments data, handles retransmits — all hidden
behind these 4 syscalls.

---

### 6) SYSTEM CALL API — everything above

```
execve("/tmp/demo", ...)              ← kernel loads the binary into memory
clone(...)                            ← kernel creates a new process
mmap(...)                             ← kernel manages virtual memory
openat(...)                           ← kernel manages files
socket(...) / connect(...)            ← kernel manages network
wait4(...)                            ← kernel manages process lifecycle
```

**Note:** Every single operation this program does is a system call — a controlled trap into kernel mode.
The user program makes a request (the syscall) and the kernel does the actual work. **This boundary — user mode
→ kernel mode — is the fundamental interface of the operating system.** `strace` makes this invisible boundary
visible by logging every trap.

---

### Quick Reference Table

| Kernel Responsibility            | C API         | Syscall (see in strace)       |
| -------------------------------- | ------------- | ----------------------------- |
| 1.  Process Scheduling           | `fork()`      | `clone`                       |
| 2.  Memory Management            | `mmap()`      | `mmap`, `munmap`              |
| 3.  File System                  | `open()`      | `openat`, `write`, `close`    |
| 4.  Process Creation/Termination | `fork/exit()` | `clone`, `exit_group`, `wait4` |
| 5.  Networking                   | `socket()`    | `socket`, `connect`, `sendto`, `recvfrom` |
| 6.  System Call API              | —             | every line in the strace log  |


## python -u read_file.py > read_file.md 
```
Run strace python -u read_file.py 
```
### Strace Output Explanation

1. **Process Execution & Dynamic Linking**
- execve — Kernel loads /usr/bin/python3 with args ["python3", "-u", "read_file.py"].
- brk / mmap — Initial heap and anonymous memory allocation.
- Dynamic library loading — The dynamic linker searches for shared libraries (libm.so.6, libz.so.1, libexpat.so.1, libc.so.6). It first tries hardware-cap specific paths and LD_LIBRARY_PATH entries (opensearch, cuda), gets ENOENT, then falls back to /etc/ld.so.cache and finds them under /lib/x86_64-linux-gnu/. Each library is openat → read (ELF header) → mmap into process address space.
2. **Runtime Initialization**
- arch_prctl, set_tid_address, set_robust_list, rseq — Thread-local storage and robust futex setup.
- mprotect — Sets page protections on loaded library segments (read-only, read+exec, etc.).
- prlimit64 — Confirms stack size limit (8MB).
- Locale & encoding — Loads /usr/lib/locale/locale-archive and gconv-modules.cache.
- Python interpreter boot — Resolves binary path via PATH search (newfstatat on each $PATH dir), reads pyvenv.cfg, discovers Python 3.12 library paths, loads stdlib modules (encodings/__init__.pyc, aliases.pyc, utf_8.pyc), and runs sitecustomize.py and apport_python_hook.py.
3. **Signal & I/O Setup**
- rt_sigaction — Python sets up signal handlers: SIGPIPE and SIGXFSZ are ignored (SIG_IGN), SIGINT gets a Python-specific handler. All other signals queried to check defaults.
- fstat / ioctl / lseek on fd 0, 1, 2 — Python checks whether stdin/stdout/stderr are ttys. stdout and stderr are sockets (not ttys), stdin is a character device.
4. **Reading read_file.py**
- newfstatat — Gets file metadata: 68 bytes, mode 0664.
- openat — Opens read_file.py as fd 3 (O_RDONLY|O_CLOEXEC).
- fstat / ioctl / lseek — Confirms it's a regular file (not a tty), checks seekability.
- read(3, ..., 69) — Reads all 68 bytes of the script source.
- close(3) — Closes the script file.
5. **Opening & Reading demo.c**
- openat(AT_FDCWD, "demo.c", O_RDONLY|O_CLOEXEC) — Python's open() call opens demo.c as fd 3 (2840 bytes).
- fstat(3) — Confirms file size: 2840 bytes.
- read(3, ..., 2841) — Reads all 2840 bytes into memory (this is f.read()).
- read(3, "", 1) — Second read returns 0 bytes → EOF.
- write(1, ..., 2840) — Python's print(content) writes the full C source to stdout.
- write(1, "\n", 1) — Prints the trailing newline.
- close(3) — Closes demo.c.
6. **Cleanup & Exit**
- rt_sigaction(SIGINT) — Restores default SIGINT handler.
- brk (×2) — Shrinks heap back down.
- munmap — Releases Python runtime memory mappings.
- exit_group(0) — Process exits cleanly with status 0.

### Key Takeaway
The syscalls show the full lifecycle: kernel loads Python → resolves shared libs → boots interpreter → reads script → script issues open/read/write/close on demo.c → kernel mediates all I/O → process exits. The -u flag made stdout unbuffered, so write happens immediately after read.

![](../../images/python_flow.png)  
Source: ChatGPT
