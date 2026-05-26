# Code Experiments: Process Fundamentals

A collection of C programs that demonstrate process behavior, memory allocation, and system limits.

## Programs

**`infinite_loop.c`** — Runs an endless `for` loop that calls `some_function()` with an incrementing counter. Demonstrates a process that consumes CPU indefinitely and must be killed manually (e.g., with `Ctrl+C`).

**`seg_fault.c`** — A function that recurses infinitely with no base case. Each recursive call adds a stack frame, eventually exhausting the stack and triggering a **segmentation fault** (stack overflow crash).

**`stack_max_allocation.c`** — Allocates a variable-length array on the **stack** of user-specified size (in number of ints). Useful for exploring the maximum stack size a process can allocate before crashing. Takes one CLI argument: number of integers.

**`heap_allocation.c`** — Allocates a block of memory on the **heap** using `malloc()` of user-specified size (in number of ints). On success, writes values to the array and frees it. On failure, prints an error. Useful for comparing heap limits against stack limits. Takes one CLI argument: number of integers.

## Build Instructions

```bash
gcc infinite_loop.c -o infinite_loop
gcc seg_fault.c     -o seg_fault
gcc stack_max_allocation.c -o stack_max_allocation
gcc heap_allocation.c      -o heap_allocation
```

## Usage

```bash
# Infinite loop (kill with Ctrl+C)
./infinite_loop

# Stack overflow via infinite recursion
./seg_fault

# Allocate N ints on the stack
./stack_max_allocation 100000

# Allocate N ints on the heap
./heap_allocation 1000000
```

## Concepts Covered

- Infinite loops and process termination
- Stack overflow and segmentation faults
- Stack vs. heap memory allocation
- OS-imposed process memory limits
