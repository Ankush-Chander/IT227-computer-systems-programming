> **Goal:** Learn to _read_ assembly, not memorize it. You need enough to understand what your compiler is doing and debug with confidence.

---

## 1. Registers

Registers are small, named storage slots directly inside the CPU — much faster than RAM. On x86-64 (the architecture you'll encounter most), there are a handful of categories you need to know.

### General-Purpose Registers

These hold data your program is actively working with. Each has a 64-bit name, and can be accessed in smaller slices:

|64-bit|32-bit|16-bit|8-bit|Common Use|
|---|---|---|---|---|
|`rax`|`eax`|`ax`|`al`|Return value from functions|
|`rbx`|`ebx`|`bx`|`bl`|General use (callee-saved)|
|`rcx`|`ecx`|`cx`|`cl`|4th function argument / loop counter|
|`rdx`|`edx`|`dx`|`dl`|3rd function argument|
|`rsi`|`esi`|`si`|`sil`|2nd function argument|
|`rdi`|`edi`|`di`|`dil`|1st function argument|
|`r8`–`r15`|`r8d`–`r15d`|`r8w`–`r15w`|`r8b`–`r15b`|Extra arguments / scratch|

When you see `eax` in disassembly, it's the lower 32 bits of `rax`. Writing to `eax` on x86-64 **zeroes out the upper 32 bits** of `rax` — a subtle gotcha.

### Special-Purpose Registers

These have fixed roles the CPU enforces:

| Register | Name                | Role                                                                           |
| -------- | ------------------- | ------------------------------------------------------------------------------ |
| `rsp`    | Stack Pointer       | Points to the **top** (lowest address) of the stack                            |
| `rbp`    | Base Pointer        | Points to the **bottom** of the current stack frame (optional in modern code)  |
| `rip`    | Instruction Pointer | Points to the **next instruction** to execute — you can't write to it directly |
| `rflags` | Flags Register      | Bit-flags set by arithmetic operations (Zero, Sign, Carry, Overflow, etc.)     |

> **Mental model:** `rip` is the CPU's "program counter" — it advances through your code one instruction at a time, jumping around on calls and branches.

---

## 2. Basic Instructions

You don't need to memorize hundreds of instructions. These cover the vast majority of what you'll see in disassembly.

### Data Movement

```asm
mov  rax, 42          ; rax = 42  (immediate value)
mov  rax, rbx         ; rax = rbx (register to register)
mov  rax, [rbx]       ; rax = *rbx (load from memory address in rbx)
mov  [rbx], rax       ; *rbx = rax (store to memory address in rbx)
mov  rax, [rbp - 8]   ; rax = *(rbp - 8) — typical local variable read
```

`[...]` means "the memory at this address." Think of it as the `*` dereference operator in C.

### Stack Operations

```asm
push rax    ; rsp -= 8 ; [rsp] = rax   (grows the stack downward)
pop  rax    ; rax = [rsp] ; rsp += 8   (shrinks the stack)
```

The stack grows **downward** in memory on x86. `push` decrements `rsp` first, then writes. `pop` reads first, then increments `rsp`.

### Arithmetic and Comparison

```asm
add  rax, rbx     ; rax = rax + rbx
sub  rax, 1       ; rax = rax - 1
imul rax, rbx     ; rax = rax * rbx (signed multiply)
inc  rax          ; rax++ (same as add rax, 1, but shorter)
dec  rax          ; rax--

cmp  rax, rbx     ; compute rax - rbx, set FLAGS, discard result
test rax, rax     ; compute rax & rax, set FLAGS — common idiom for "is rax zero?"
```

`cmp` and `test` don't modify their operands — they only update `rflags`. They're always paired with a conditional jump.

### Jumps and Control Flow

```asm
jmp  label        ; unconditional jump — like goto

; After a cmp or test:
je   label        ; jump if Equal         (Zero Flag set)
jne  label        ; jump if Not Equal
jl   label        ; jump if Less (signed)
jg   label        ; jump if Greater (signed)
jle  label        ; jump if Less or Equal
jge  label        ; jump if Greater or Equal
jz   label        ; jump if Zero          (same as je)
jnz  label        ; jump if Not Zero      (same as jne)
```

### Function Calls

```asm
call some_function   ; push rip (return address) onto stack, then jmp to function
ret                  ; pop return address from stack, jmp back to caller
```

`call` is two operations in one: save where to come back, then jump. `ret` is the reverse.

---

## 3. The Stack Frame

This is the most important concept for reading real disassembly. Every function call creates a **stack frame** — a region of the stack that belongs to that function.

### Function Prologue

The first few instructions of almost every function:

```asm
push rbp          ; save caller's base pointer
mov  rbp, rsp     ; set our base pointer to current stack top
sub  rsp, 32      ; reserve 32 bytes for local variables
```

After this, `rbp` is a stable anchor. Local variables live at **negative offsets** from `rbp`:

- `[rbp - 8]` → first local variable
- `[rbp - 16]` → second local variable
- `[rbp + 16]` → first argument (if passed on stack)

### Function Epilogue

The last few instructions before `ret`:

```asm
mov  rsp, rbp     ; discard locals — restore stack pointer
pop  rbp          ; restore caller's base pointer
ret               ; return to caller
```

You'll also see the shorthand instruction `leave`, which does the same as `mov rsp, rbp` + `pop rbp`.

### Calling Convention: How Arguments Are Passed

On Linux/macOS (System V AMD64 ABI), the first six integer arguments go in registers **in this order:**

```
rdi, rsi, rdx, rcx, r8, r9
```

Return value goes in `rax`.

So this C function:

```c
int add(int a, int b) {
    return a + b;
}
```

Compiles to roughly:

```asm
add:
    mov  eax, edi    ; eax = a (first argument)
    add  eax, esi    ; eax += b (second argument)
    ret              ; return value is in eax/rax
```

> **Windows is different.** It uses `rcx, rdx, r8, r9` for the first four arguments. Keep this in mind if you're reading Windows disassembly.

### Stack Layout Diagram

```
High addresses
┌─────────────────────┐
│   caller's frame    │
├─────────────────────┤ ← rbp + 16  (arguments, if spilled)
│   return address    │ ← rbp + 8   (saved rip, pushed by call)
│   saved rbp         │ ← rbp       (old base pointer)
├─────────────────────┤
│   local variable 1  │ ← rbp - 8
│   local variable 2  │ ← rbp - 16
│   ...               │
└─────────────────────┘ ← rsp       (current top of stack)
Low addresses
```

---

## 4. Reading Compiler-Generated Disassembly

The skill you actually need day-to-day is reading what your compiler produces — not writing assembly from scratch.

### Your Main Tool: Compiler Explorer

Go to **[godbolt.org](https://godbolt.org/)**. Paste any C/C++ code, pick a compiler (e.g., `x86-64 gcc 14.1`), and see the assembly output live. This is how professionals "use" assembly.

Useful compiler flags to experiment with:

- `-O0` — no optimization, verbose output, easiest to follow
- `-O2` — typical release optimization
- `-O3` — aggressive optimization, output can be surprising

### Reading Disassembly in GDB

```bash
gdb ./your_binary

(gdb) disas main          # disassemble main()
(gdb) disas some_function # disassemble any function by name
(gdb) layout asm          # split-screen view: source + assembly
(gdb) si                  # step one instruction (step-into)
(gdb) ni                  # next instruction (step-over)
(gdb) info registers      # print all register values
(gdb) x/16xb $rsp         # examine 16 bytes at stack pointer
```

### A Complete Example: Reading a Real Function

Here's a simple C function and what gcc `-O0` produces:

```c
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
```

```asm
factorial:
    push   rbp
    mov    rbp, rsp
    sub    rsp, 16            ; reserve space for locals

    mov    DWORD PTR [rbp-4], edi   ; store argument n on stack

    cmp    DWORD PTR [rbp-4], 1     ; compare n with 1
    jg     .recurse                 ; if n > 1, go recurse

    mov    eax, 1                   ; else return 1
    jmp    .done

.recurse:
    mov    eax, DWORD PTR [rbp-4]   ; eax = n
    sub    eax, 1                   ; eax = n - 1
    mov    edi, eax                 ; first argument = n - 1
    call   factorial                ; factorial(n - 1)
    imul   eax, DWORD PTR [rbp-4]  ; eax = result * n

.done:
    leave                           ; restore rbp and rsp
    ret
```

**How to read this:**

1. Prologue sets up the frame
2. `n` is spilled from `edi` to `[rbp-4]` (common at `-O0`)
3. `cmp` + `jg` is the `if (n <= 1)` check
4. The recursive call follows the calling convention — argument in `edi`, result comes back in `eax`
5. `imul` performs `n * factorial(n-1)`
6. `leave` + `ret` is the epilogue

### Common Patterns to Recognize

|Disassembly Pattern|What it means in C|
|---|---|
|`sub rsp, N`|Function has N bytes of local variables|
|`cmp rax, 0` + `je`|`if (x == 0)` or `if (!x)`|
|`test rax, rax` + `jz`|`if (x == NULL)` or `if (!x)`|
|`mov eax, 0` at function end|`return 0`|
|`call malloc` + `test rax, rax`|`ptr = malloc(...); if (!ptr)`|
|`rep movsb` or `rep movsd`|`memcpy()`|
|Loop with `dec` + `jnz`|Classic counted loop|

---

## Quick Reference Card

```
REGISTERS           FUNCTION ARGS (Linux)    STACK
rax  — return val   1st: rdi                 grows downward ↓
rbx  — callee-saved 2nd: rsi                 rsp = top (lowest addr)
rcx  — 4th arg      3rd: rdx                 rbp = frame base
rdx  — 3rd arg      4th: rcx
rsi  — 2nd arg      5th: r8
rdi  — 1st arg      6th: r9
rsp  — stack top    7th+: on stack
rbp  — frame base
rip  — instr ptr

INSTRUCTIONS
mov dst, src    — copy/load/store
push/pop        — stack operations
add/sub         — arithmetic
cmp/test        — set flags (no result stored)
jmp/je/jne/jl   — control flow
call/ret        — function call & return
leave           — epilogue shorthand (mov rsp,rbp + pop rbp)
```

---

## What to Do Next

1. **Paste simple C functions into [godbolt.org](https://godbolt.org/)** with `-O0`, then `-O2`. Notice what changes.
2. **Write a raw syscall in C** using inline assembly or `syscall()` — trace what happens.
3. **Compile with debug symbols** (`-g`) and step through a function in GDB with `layout asm`.
4. **Disassemble something that segfaults.** The crash site in disassembly will tell you exactly what instruction failed and why.

The goal is not to write assembly. The goal is to never be confused by it.