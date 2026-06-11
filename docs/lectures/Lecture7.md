---
model: ollama@my-qwen3.6:27b
---
## Compiler optimization   
1. **Constant Propagation & Folding**    
   Replaces variables with known compile-time values and precomputes constant expressions.    
   *Assembly impact*: Instructions use immediate operands directly; eliminates redundant loads, registers, and arithmetic ops.  

2. **Dead Code Elimination (DCE)**    
   Removes statements that cannot influence program output or observable side effects.  
   *Assembly impact*: Shorter instruction streams, fewer branches, and unused instructions vanish from the output.

3. **Register Allocation & Spill Reduction**    
   Maps C variables/temporaries to hardware registers using graph coloring, delaying or eliminating stack spills.  
   *Assembly impact*: Maximizes use of general-purpose registers; reduces `mov`/stack traffic and improves cache locality.

4. **Function Inlining**  
   Replaces `call` instructions with the callee’s body when beneficial.  
   *Assembly impact*: Eliminates call/return overhead, exposes code for further optimization, but may increase binary size.

5. **Loop Unrolling**  
   Duplicates the loop body to reduce iteration count and branch frequency.  
   *Assembly impact*: Fewer conditional jumps, better pipeline utilization, and opportunities for independent instruction parallelism.

6. **Loop-Invariant Code Motion (LICM)**  
   Hoists computations that don't change across iterations outside the loop.  
   *Assembly impact*: Moves instructions above the loop header, reducing repeated executions per iteration.

7. **Strength Reduction**  
   Replaces expensive operations with cheaper equivalents (e.g., `x * 8` → `x << 3`, or address math via `lea`).  
   *Assembly impact*: Uses shifts, adds, or `lea` instead of `imul`/`idiv`, lowering instruction latency and size.

8. **Common Subexpression Elimination (CSE)**  
   Identifies identical expressions and computes them once, reusing the result in a register.  
   *Assembly impact*: Avoids redundant instruction sequences; results are cached in registers instead of recomputed.

9. **Tail Call Optimization**  
   Converts final function calls in a function into direct jumps instead of calls.  
   *Assembly impact*: Replaces `call` + `ret` with `jmp`, preserving stack depth and enabling constant-space recursion.

10. **Instruction Scheduling & Reordering**  
    Reorders independent instructions to hide memory/CPU latency or fill the instruction window.  
    *Assembly impact*: Changes instruction order without altering semantics; improves instruction-level parallelism (ILP) and pipeline throughput.

11. **Stack Frame Optimizations (Frame Omission, Stack Coloring)**  
    Merges stack slots, eliminates unused slots, or removes prologue/epilogue for leaf functions.  
    *Assembly impact*: Fewer `sub rsp`/`push`/`mov [rsp+...]` instructions; faster function entry/exit.

12. **Auto-Vectorization (SIMD)**  
    Detects independent array/loop operations and converts them to SIMD instructions.  
    *Assembly impact*: Uses AVX/NEON/RISC-V V extensions to process multiple data elements per cycle.

13. **Jump Threading & Branch Simplification**  
    Removes redundant conditionals, merges identical blocks, and straightens control flow.  
    *Assembly impact*: Fewer branch instructions, cleaner control flow, and improved branch prediction behavior.



# Memory hierarchy
![](https://raw.githubusercontent.com/Ankush-Chander/Tech-Talks/refs/heads/main/img/relative-time-latencies-computer-programming.jpg)
# Programmer visible state

In a CPU, the **programmer-visible state** means the registers and memory locations that you (or a compiler) can directly read or write using instructions.  
For x86-64, this includes:

- **General-purpose registers** (for data and addresses)
- **Special registers** (like flags, stack pointer, instruction pointer)
- **Memory** (but you asked about registers first)


![](../images/program_visible_state.png)
---

### General-Purpose Registers (GPRs)

x86-64 has **16 general-purpose registers**, each 64 bits wide.  
But they have a history: originally 16-bit, then 32-bit, now 64-bit.

**Names and size variants:**

You can use the lower parts for backward compatibility:

| 64-bit | 32-bit   | 16-bit   | 8-bit (low) | 8-bit (high, legacy) | Purpose hint                     |
| ------ | -------- | -------- | ----------- | -------------------- | -------------------------------- |
| rax    | eax      | ax       | al          | ah                   | Accumulator (math, return value) |
| rbx    | ebx      | bx       | bl          | bh                   | Base (sometimes memory base)     |
| rcx    | ecx      | cx       | cl          | ch                   | Counter (loops, shifts)          |
| rdx    | edx      | dx       | dl          | dh                   | Data (I/O, multiply/divide)      |
| rsi    | esi      | si       | sil         | —                    | Source index (string ops)        |
| rdi    | edi      | di       | dil         | —                    | Destination index                |
| rbp    | ebp      | bp       | bpl         | —                    | Base pointer (stack frames)      |
| rsp    | esp      | sp       | spl         | —                    | Stack pointer (special)          |
| r8–r15 | r8d–r15d | r8w–r15w | r8b–r15b    | —                    | New in x86-64                    |

> For the 8 low bytes of rsi, rdi, rbp, rsp, r8–r15, use the `l` suffix (sil, dil, bpl, spl, r8b..r15b). No `ah`/`bh`/etc for them.

---

## Special Registers

### a. **RFLAGS** (64-bit flags register)

Holds status and control flags. Only some bits are used; the rest are reserved.

**Common status flags you should know:**

- `ZF` (Zero flag): set if result of last operation was zero.
- `SF` (Sign flag): set if result negative (MSB=1).
- `CF` (Carry flag): set if unsigned overflow.
- `OF` (Overflow flag): set if signed overflow.
- `AF` (Aux carry): for BCD arithmetic.
- `PF` (Parity flag): parity of low byte.

Also control flags:  
- `IF` (Interrupt enable flag)  
- `DF` (Direction flag for string instructions)

**Size:** 64 bits, but only lower 32 are visible in 32-bit mode (EFLAGS). In 64-bit mode, upper 32 bits are reserved.

---

### b. **Stack Pointer: RSP**

- **RSP** (64-bit) points to the top of the stack in memory.
- Stack grows **downward** (higher to lower addresses).
- Used implicitly by `push`, `pop`, `call`, `ret`.

Size: 64 bits.  
You can also use `SP` (16-bit), `ESP` (32-bit) for lower parts.

---

### c. **Instruction Pointer: RIP**

- **RIP** holds the memory address of the **next instruction** to execute.
- You can’t write to it directly, but branch instructions (`jmp`, `call`, `ret`, conditional jumps) change it.
- In 64-bit mode, **RIP-relative addressing** is common (`mov rax, [rip+offset]`).

Size: 64 bits.  
In 32-bit mode, it’s called `EIP` (32-bit).

### Why registers?

**Registers exist because CPU logic operates in cycles (1-5 cycles), but RAM operates in hundreds of cycles — without registers, the CPU would spend >99% of its time waiting for memory instead of computing.**

---
# Basic Instructions

You don't need to memorize hundreds of instructions. These cover the vast majority of what you'll see in disassembly.

### Data Movement

```asm
movq $42, %rax          # rax = 42  (immediate value)
movq %rbx, %rax         # rax = rbx (register to register)
movq (%rbx), %rax       # rax = *rbx (load from memory address in rbx)
movq %rax, (%rbx)       # *rbx = rax (store to memory address in rbx)
movq -8(%rbp), %rax     # rax = *(rbp - 8) — typical local variable read
```

`[...]` means "the memory at this address." Think of it as the `*` dereference operator in C.

### Stack Operations

```asm
pushq %rbp; # implicitly decrements stack by 8 bytes and then stores the current base pointer there
movq %rsp, %rbp   (grows the stack downward)

popq  %rbp;   (shrinks the stack)
```

The stack grows **downward** in memory on x86. `push` decrements `rsp` first, then writes. `pop` reads first, then increments `rsp`.

### Arithmetic and Comparison

```asm
addq %rbx, %rax          # rax = rax + rbx
subq $1, %rax            # rax = rax - 1
imulq %rbx, %rax         # rax = rax * rbx (signed multiply)
incq %rax                # rax++ (same as addq $1, %rax, but shorter)
decq %rax                # rax--

cmpq %rbx, %rax          # compute rax - rbx, set FLAGS, discard result
testq %rax, %rax         # compute rax & rax, set FLAGS — common idiom for "is rax zero?"
```

`cmp` and `test` don't modify their operands — they only update `rflags`. They're always paired with a conditional jump.


## The Suffix Meaning

| Suffix | Size (bytes) | Size (bits) | C equivalent      |
| ------ | ------------ | ----------- | ----------------- |
| `b`    | 1 byte       | 8 bits      | `char`            |
| `w`    | 2 bytes      | 16 bits     | `short`           |
| `l`    | 4 bytes      | 32 bits     | `int`             |
| `q`    | 8 bytes      | 64 bits     | `long` or pointer |

**Note:** `l` stands for "long" (32 bits on x86-64, historical naming). `q` stands for "quad" (quad-word, 64 bits).


---
## References
1. [What exactly is the base pointer and stack pointer? - Stack overflow](https://stackoverflow.com/a/1395934/2893777)
