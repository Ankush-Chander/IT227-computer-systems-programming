# Lab: Build Your Own Shell

---

## Learning Objectives

By the end of this lab you will be able to:
- Understand how a shell interprets and executes commands
- Work with process creation (`fork`, `exec`, `wait`) family of system calls
- Handle file descriptors and I/O redirection at the OS level
- Implement piping between processes
- Manage process lifecycle and signal handling
- Transition a working design from a high-level language to raw C system calls

---

## Phase 1 — Python Shell

**Goal:** Build a functional interactive shell in Python to understand the architecture and logic before translating to low-level C.

**Why Python first?** It lets you focus on the *shell's control flow* (read → parse → execute loop) without getting bogged down in memory management and pointer arithmetic. Once the design is solid, you'll reimplement it in C using the same architecture.

---

### Requirements — Phase 1

#### Core shell loop
- Display a custom prompt (e.g. `mysh> `)
- Read a line of input from the user
- Parse the input into **command**, **options**, and **arguments**
- Execute the command in a child process
- Loop until the user types `exit`

#### Built-in commands
- `cd <directory>` — change working directory (must run in the shell's own process, not a child)
- `exit` — terminate the shell
- `history` — print all previously entered commands numbered sequentially
- `pwd` — print working directory (may delegate to the system `pwd` or implement manually)

#### External command execution
- Resolve the command path using the `$PATH` environment variable
- Use `os.fork()`, `os.execvp()`, and `os.wait()` to run external commands
- Print a clear error message for:
  - Command not found
  - Permission denied
  - Invalid syntax

#### I/O Redirection
- Output redirection with `>` — overwrite file
- Output append with `>>` — append to file
- Input redirection with `<` — read from file
- Error redirection `2>` — redirect stderr to file

#### Piping
- Support `|` to pipe the stdout of one command to the stdin of another
- Support chaining: `cmd1 | cmd2 | cmd3`

#### Signal handling
- Handle `Ctrl+C` (`SIGINT`) — clear the current line and print a fresh prompt, do NOT kill the shell
- Handle `Ctrl+Z` (`SIGTSTP`) — suspend the foreground process, print `<stopped> <pid>`
- Support `fg` and `bg` built-ins to resume stopped jobs (extra bonus within the 1 mark)

---

### Deliverables — Phase 1

1. `mysh.py` — the shell source code
2. `test_mysh.sh` — a test script that runs at least 10 test cases covering each requirement above and prints PASS/FAIL for each
3. Screenshot or log of your test script execution
