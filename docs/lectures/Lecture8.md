
## Agenda
How procedures are implemented?  
a. Passing data  
b. passing control  
c. allocating/deallocating memory  

## Stack Manipulation

### `pushq`

```asm
pushq   %rbp
```

A single instruction that does **two things atomically**:

|Implicit Operation|Details|
|---|---|
|`RSP ← RSP - 8`|Decrements stack pointer by 8 bytes (making room)|
|`[RSP] ← src`|Writes the value onto the stack at the new top|

> Think of it as: _"Make space on the stack, then place the value there."_

---

### `popq`

```asm
popq    %rbp
```

The exact mirror of `pushq` — also does **two things**:

|Implicit Operation|Details|
|---|---|
|`dst ← [RSP]`|Reads the value from the top of the stack into the destination|
|`RSP ← RSP + 8`|Increments stack pointer by 8 bytes (reclaiming the space)|

> Think of it as: _"Pick up the value from the top of the stack, then shrink the stack back."_

---

### `subq` — Allocating space

```asm
subq    $32, %rsp
```

|Operation|Details|
|---|---|
|`RSP ← RSP - 32`|Moves the stack pointer **down**, carving out blank space|

- Does **not** write any values — just creates room
- Used to reserve space for local variables all at once
- Must be a multiple of 16 to maintain stack alignment

> Think of it as: _"Tear off a chunk of the notepad — don't write anything yet, just reserve the space."_

---

### `addq` — Deallocating space

```asm
addq    $32, %rsp
```

|Operation|Details|
|---|---|
|`RSP ← RSP + 32`|Moves the stack pointer **up**, discarding the reserved space|

- Does **not** erase any values — the data is still physically there, just treated as gone
- The counterpart to `subq` — used when local variable space is no longer needed
- `leave` does this implicitly via `movq %rbp, %rsp`

> Think of it as: _"Rip those pages off the notepad — we're done with them."_

---

W
### Summary Table

|Instruction|RSP change|Reads stack?|Writes stack?|
|---|---|---|---|
|`pushq src`|`-8`|No|Yes|
|`popq dst`|`+8`|Yes|No|
|`subq $N, %rsp`|`-N`|No|No|
|`addq $N, %rsp`|`+N`|No|No|

> The stack **always grows downward** in x86-64 — lower address = deeper in the stack.


## Passing control

### Call

```asm
call func
```

What does call do?

- call pushes the return address on stack
- copies the address of called function on Program counter(RIP)

| Implicit Operation                 | Details                                                     |
| ---------------------------------- | ----------------------------------------------------------- |
| `RSP ← RSP - 8`                    | Decrements stack pointer by 8 bytes (64-bit return address) |
| `[RSP] ← RIP + instruction_length` | Pushes exact return address onto stack                      |
| `RIP ← target_address`             | Jumps to the called function                                |

---

### Function Prologue

The first few instructions of every function — sets up a **fresh stack frame** for the called function.

```asm
pushq   %rbp
movq    %rsp, %rbp
```

| Instruction       | What it does                                                                       |
| ----------------- | ---------------------------------------------------------------------------------- |
| `pushq %rbp`      | Save the **caller's** base pointer onto the stack so we can restore it later       |
| `movq %rsp, %rbp` | Set our **own** base pointer to the current stack top — this is our frame's anchor |

> Think of it as: _"Before I set up my own desk, let me remember where my boss's desk was."_

After the prologue, the function optionally does:

```asm
subq $N, %rsp       ; carve out space for local variables
```

---

### Function Epilogue

The last few instructions — **tears down the stack frame** and hands control back to the caller.

```asm
leave
ret
```

These two instructions are the epilogue. They undo everything the prologue and body did to the stack.

---

### `leave`

`leave` is a single instruction that does the job of **two instructions**:

```asm
movq    %rbp, %rsp      ; step 1: throw away all local variables at once
popq    %rbp            ; step 2: restore caller's base pointer
```

| Step              | What it does                                                                       |
| ----------------- | ---------------------------------------------------------------------------------- |
| `movq %rbp, %rsp` | Move stack pointer back to where the frame started — instantly discards all locals |
| `popq %rbp`       | Pop the saved `%rbp` off the stack, restoring the **caller's frame**               |

> Think of it as: _"Clear everything off my desk, then hand the desk back to my boss exactly as it was."_

---

### `ret`

```asm
ret
```

`ret` is the mirror image of `call`. It does:

| Implicit Operation | Details                                             |
| ------------------ | --------------------------------------------------- |
| `RIP ← [RSP]`      | Reads the return address that `call` pushed earlier |
| `RSP ← RSP + 8`    | Pops it off the stack                               |

> The CPU looks at the top of the stack, finds the return address `call` left there, and jumps back to it — resuming the caller right after where it called us.

---

### Full Picture Together

```
CALLER                          CALLED FUNCTION
──────                          ───────────────
call func   ──────────────────► pushq %rbp        ← prologue start
  (saves return address)        movq  %rsp, %rbp
  (jumps to func)               subq  $N, %rsp    ← prologue end

                                 ... function body ...

                                leave             ← epilogue start
ret         ◄─────────────────  ret               ← epilogue end
  (resumes here)                  (return address popped, jump back)
```

Every `call` has a matching `ret`. Every prologue has a matching epilogue. The stack is always left exactly as it was found.

## Passing Data Back and Forth Between Procedures

### Argument Passing — Calling Convention

In x86-64, the first **6 integer/pointer arguments** are passed in registers, in this fixed order:

|Argument|Register|
|---|---|
|1st|`%rdi`|
|2nd|`%rsi`|
|3rd|`%rdx`|
|4th|`%rcx`|
|5th|`%r8`|
|6th|`%r9`|
|7th onwards|pushed onto the stack|

> Think of it as: _"The caller fills up to 6 labeled boxes before knocking on the door. The called function knows exactly which box to look in for each argument."_

---

### Return Value

The return value is always placed in `%rax` before `ret`:

```asm
movl    %edx, %eax      ; put return value in %eax (32-bit)
ret                     ; hand it back to the caller
```

|Size|Register used|
|---|---|
|64-bit|`%rax`|
|32-bit|`%eax`|
|16-bit|`%ax`|
|8-bit|`%al`|

> Think of it as: _"Before leaving, the called function places its answer in a specific handoff spot. The caller always knows to look there."_

---

### Passing by Value vs Passing by Pointer

**By value** — the actual number is placed directly in the register:

```asm
movl    $10, %edi       ; pass the number 10 itself
movl    $20, %esi       ; pass the number 20 itself
call    add
```

**By pointer** — the address of the variable is placed in the register:

```asm
leaq    -28(%rbp), %rdi ; pass the address of variable a
leaq    -24(%rbp), %rsi ; pass the address of variable b
call    add
```


| By Value                             | By Pointer           |                            |
| ------------------------------------ | -------------------- | -------------------------- |
| What's passed                        | The actual data      | A memory address           |
| Called function can modify original? | ❌ No — it got a copy | ✅ Yes — it has the address |
| Instruction used                     | `movl`               | `leaq`                     |


> **By value** is like handing someone a **photocopy** — they can scribble on it, your original is safe. **By pointer** is like handing someone your **home address** — they can show up and rearrange your furniture.

---

### Caller-Saved vs Callee-Saved Registers

Not all registers are equal — there's a contract about **who is responsible for preserving them**:

**Caller-saved (volatile)** — the caller must save these if it needs them after the call, because the called function is free to overwrite them:

|Registers|
|---|
|`%rax`, `%rdi`, `%rsi`, `%rdx`, `%rcx`, `%r8`, `%r9`, `%r10`, `%r11`|

**Callee-saved (non-volatile)** — the called function must restore these before returning, because the caller expects them unchanged:

|Registers|
|---|
|`%rbx`, `%rbp`, `%r12`, `%r13`, `%r14`, `%r15`|

> **Caller-saved** is like a whiteboard in a meeting room — anyone can erase it, so save your work before someone else uses the room. **Callee-saved** is like a borrowed book — if you borrow it, you must return it exactly as you found it.

---

### Full Picture Together

```
CALLER                              CALLED FUNCTION
──────                              ───────────────
; set up arguments
movl  $10,         %edi  ─────────► 1st arg in %rdi
movl  $20,         %esi  ─────────► 2nd arg in %rsi

call  add          ───────────────► prologue
                                    ... use %rdi, %rsi ...
                                    movl result, %eax   ← return value
                                    epilogue
                                    ret
; return value waiting in %rax
movl  %eax, -4(%rbp)  ◄──────────  (caller picks it up from %rax)
```


### Summary


| Concept                  | Mechanism                        |
| ------------------------ | -------------------------------- |
| Pass arguments (1st–6th) | Registers `%rdi` → `%r9`         |
| Pass arguments (7th+)    | Push onto stack                  |
| Return a value           | Place in `%rax` before `ret`     |
| Pass by value            | `movl $val, %rdi`                |
| Pass by pointer          | `leaq addr, %rdi`                |
| Volatile registers       | Caller saves if needed           |
| Non-volatile registers   | Callee must restore before `ret` |

---

## Stack frame

```
frame header:
    return address (used by callee to return the control to the right place in the callers body) [at rbp+8] 
    saved rbp (used by caller to refer it"s citizen once control is returned to it)  [at rbp]

frame body:
    locals [typically space reserved in multiple of 16]
    spills
    saved registers
```

## Stack Alignment

### The Rule

> **`%rsp` must be 16-byte aligned before any `call` instruction.** Meaning `%rsp` must always be a multiple of 16 at the moment of calling.

---

### Why Does the CPU Care?

Modern CPUs have **SIMD/SSE registers** (`%xmm0`–`%xmm15`) that are **128 bits (16 bytes) wide**. Instructions that move data in/out of these registers (e.g. `movaps`) require the memory address to be **exactly 16-byte aligned** — otherwise the CPU throws a **segfault**.

> Think of it as a truck that can only park at loading docks spaced exactly 16 metres apart. If your dock is off by even 1 metre, the truck can't unload.

Even if _your_ function doesn't use SSE, the **OS, libc, or any function you call** might — so the rule must always be respected.

---

### How Alignment Gets Disturbed

At program start, `%rsp` is 16-byte aligned by the OS. Then things happen:

```
%rsp = 16-byte aligned          ← OS guarantees this at program start

pushq %rbp                      → %rsp -= 8   ← now misaligned by 8!
```

After `pushq %rbp`, `%rsp` is off by 8. So `subq $N, %rsp` must compensate:

```
(8 + N) must be a multiple of 16
→ N must be a multiple of 16 (e.g. 16, 32, 48...)
```

---

### Walking Through an Example

```asm
; %rsp = 1000  (16-byte aligned, given by OS)

call main           ; %rsp = 992   (pushed 8-byte return address)
pushq %rbp          ; %rsp = 984   (saved caller's %rbp)
subq $24, %rsp      ; %rsp = 960   (984 - 24 = 960 ✅ multiple of 16)
```

| Event              | `%rsp` | Aligned?     |
| ------------------ | ------ | ------------ |
| OS hands control   | 1000   | ✅            |
| After `call main`  | 992    | ✅            |
| After `pushq %rbp` | 984    | ❌ (off by 8) |
| After `subq $24`   | 960    | ✅            |
| After `subq $22`   | 962    | ❌            |
| After `subq $30`   | 954    | ❌            |

---

### Why Multiples of 16 for Local Variables?

Say your function has locals that only need 20 bytes:

```
int a;   4 bytes
int b;   4 bytes
int c;   4 bytes
int d;   4 bytes
char e;  1 byte
─────────────────
total    17 bytes  ← actual need
```

You **cannot** do `subq $17, %rsp` or even `subq $20, %rsp` — both misalign the stack. So the compiler **rounds up to the nearest multiple of 16**:

```
17 bytes → round up → 32 bytes
20 bytes → round up → 32 bytes
```

```
┌─────────────────────────┐ ← %rbp
│      Saved %rbp         │   (8 bytes, from pushq)
├─────────────────────────┤
│   a, b, c, d, e         │   17 bytes actually used
├─────────────────────────┤
│   padding               │   15 bytes wasted — just to stay aligned
└─────────────────────────┘ ← %rsp  (subq $32 keeps alignment)
```

> The padding bytes are **intentionally wasted** — a small price for correctness and performance.

---

### Performance Bonus

Even beyond SSE requirements, aligned memory access is **faster** on all modern CPUs:

| Access type                     | Cost                                      |
| ------------------------------- | ----------------------------------------- |
| Aligned (address % 16 == 0)     | Single memory transaction                 |
| Misaligned (crosses cache line) | Two memory transactions — up to 2× slower |

---

### Summary

| Question                               | Answer                                                      |
| -------------------------------------- | ----------------------------------------------------------- |
| Why align at all?                      | SSE instructions require it; misalignment causes segfault   |
| What is the rule?                      | `%rsp` must be multiple of 16 before `call`                 |
| Why does `pushq %rbp` cause a problem? | It shifts `%rsp` by 8, breaking alignment                   |
| How does `subq` fix it?                | N must be chosen so that `8 + N` is a multiple of 16        |
| Why round locals up to multiple of 16? | To satisfy that rule regardless of how many locals you have |
| What fills the gap?                    | Padding bytes — unused but necessary                        |




