
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

Right before a function is called, `%rsp` is 16-byte aligned. Then things happen:


```

%rsp = 16-byte aligned          ← Guaranteed right before `call`

call my_func                    → %rsp -= 8   ← misaligned by 8! (return address pushed)
pushq %rbp                      → %rsp -= 8   ← aligned again! (saved base pointer)

```

Because the `call` instruction breaks the alignment and `pushq %rbp` restores it, `%rsp` is perfectly aligned after the function prologue. So, when you allocate space for local variables with `subq $N, %rsp`, you must be careful not to break it again:


```

N must be a multiple of 16 (e.g. 16, 32, 48...)

```

---

### Walking Through an Example

```asm
; %rsp = 1024  (16-byte aligned right before the call)

call main           ; %rsp = 1016  (Misaligned! pushed 8-byte return address)
pushq %rbp          ; %rsp = 1008  (Aligned! saved caller's %rbp)
subq $32, %rsp      ; %rsp = 976   (1008 - 32 = 976 ✅ multiple of 16)

```

| Event | `%rsp` | Aligned? |
| --- | --- | --- |
| Right before `call` | 1024 | ✅ |
| After `call main` | 1016 | ❌ (off by 8) |
| After `pushq %rbp` | 1008 | ✅ (restored) |
| After `subq $32` | 976 | ✅ |
| After `subq $24` | 984 | ❌ |
| After `subq $30` | 978 | ❌ |

---

### Why Multiples of 16 for Local Variables?

Say your function has locals that only need 17 bytes:

```
int a;   4 bytes
int b;   4 bytes
int c;   4 bytes
int d;   4 bytes
char e;  1 byte
─────────────────
total    17 bytes  ← actual need

```

You **cannot** do `subq $17, %rsp` or even `subq $24, %rsp` — both misalign the stack. So the compiler **rounds up to the nearest multiple of 16**:

```
17 bytes → round up → 32 bytes
20 bytes → round up → 32 bytes

```

```
┌─────────────────────────┐ ← %rbp
│    Saved %rbp           │   (8 bytes, from pushq)
├─────────────────────────┤
│    a, b, c, d, e        │   17 bytes actually used
├─────────────────────────┤
│    padding              │   15 bytes wasted — just to stay aligned
└─────────────────────────┘ ← %rsp  (subq $32 keeps alignment)

```

> The padding bytes are **intentionally wasted** — a small price for correctness and performance.

---

### Performance Bonus

Even beyond SSE requirements, aligned memory access is **faster** on all modern CPUs:

| Access type | Cost |
| --- | --- |
| Aligned (address % 16 == 0) | Single memory transaction |
| Misaligned (crosses cache line) | Two memory transactions — up to 2× slower |

---

### Summary

| Question | Answer |
| --- | --- |
| Why align at all? | SSE instructions require it; misalignment causes segfault |
| What is the rule? | `%rsp` must be multiple of 16 before `call` |
| What breaks the alignment? | The `call` instruction pushes an 8-byte return address |
| How is it restored? | `pushq %rbp` pushes another 8 bytes, restoring alignment |
| How does `subq` maintain it? | N must be a multiple of 16 so alignment isn't broken again |
| Why round locals up to multiple of 16? | To satisfy that rule regardless of how many locals you have |
| What fills the gap? | Padding bytes — unused but necessary |

---

## Red Zone

### What Is It?

In x86-64 SysV ABI (Linux/macOS), the **Red Zone** is a **128-byte region immediately below `%rsp`** that the calling convention guarantees will not be clobbered by signal handlers or asynchronous interrupts.

```
Higher addresses
┌─────────────────────────┐
│    Normal stack frame   │
├─────────────────────────┤ ← %rsp points here
│  ┌───────────────────┐  │
│  │     Red Zone      │  │
│  │   (128 bytes)     │  │
│  └───────────────────┘  │
├─────────────────────────┤ ← %rsp - 128
│   OS / signal handlers  │
└─────────────────────────┘
Lower addresses
```

> Think of it as a private scratch-pad right below the stack pointer — you can scribble on it without adjusting `%rsp`, because the ABI guarantees nobody else will touch it.

---

### What Does It Enable?

Because these 128 bytes are guaranteed safe, a leaf function can store locals in the Red Zone **without adjusting `%rsp` at all**:

```
Without Red Zone                  With Red Zone (leaf function)
──────────────                    ─────────────────────────
pushq %rbp                        ; no pushq needed
movq  %rsp, %rbp                  ; no movq needed
subq  $32, %rsp                   ; no subq needed!
movl  %edi, -20(%rbp)             movl  %edi, -4(%rsp)
movl  %esi, -24(%rbp)             movl  %esi, -8(%rsp)
                                  ; ... use locals freely ...
addq  $32, %rsp                   ; no addq needed
leave                             ret
ret                               ; just return directly
```

---

### Benefits

| Benefit | Explanation |
| --- | --- |
| **Fewer instructions** | No `subq`/`addq` on `%rsp`, sometimes no full prologue/epilogue |
| **Less register pressure** | Don't need `%rbp` for frame anchoring — access locals directly via `%rsp` offsets |
| **Faster leaf functions** | Eliminates entire stack setup and teardown overhead |

---

### When Can You Use It?

The Red Zone is only valid for **leaf functions** — functions that do **not call any other function** (directly or indirectly):

| Scenario | Safe to use Red Zone? |
| --- | --- |
| Simple computation, no calls, small locals | ✅ Yes |
| Function that calls another function | ❌ No — the callee will step into your Red Zone |
| Function using `alloca()` | ❌ No |
| Function using `setjmp`/`longjmp` | ❌ No |
| Code compiled with `-fno-omit-frame-pointer` | ❌ Typically disabled |
| Kernel code / interrupt handlers | ❌ No — signals can clobber it |

---

### Why Does It Exist?

Without the Red Zone, **every single function** — even trivial ones like:

```c
int square(int x) {
    return x * x;
}
```

...would need to adjust `%rsp` if it wanted to spill a local value. The Red Zone eliminates this overhead for simple leaf functions.

---

### Controlling the Red Zone

| Compiler Flag | Effect |
| --- | --- |
| Default (Linux x86-64) | Red Zone enabled — compiler uses it automatically |
| `-mno-red-zone` | Disables it entirely — all locals go above `%rsp` with explicit allocation |
| `-fPIC` / position-independent | Red Zone still available by default |

> **Important:** Windows x86-64 ABI (Microsoft) does **not** have a Red Zone. Code compiled for Windows always needs explicit stack allocation.

---

### Walking Through an Example

Compile this C code with and without the Red Zone:

```c
int sum(int a, int b) {
    int result = a + b;
    return result;
}
```

**With Red Zone (default, `-O0`):**

```asm
sum:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $32, %rsp
    movl    %edi, -20(%rbp)     ; a
    movl    %esi, -24(%rbp)     ; b
    movl    -20(%rbp), %edx
    movl    -24(%rbp), %eax
    addl    %edx, %eax
    movl    %eax, -4(%rbp)      ; result — stored in Red Zone or stack area
    movl    -4(%rbp), %eax
    leave
    ret
```

**With `-mno-red-zone` (same code):**

```asm
sum:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $32, %rsp           ; same layout — but now ALL locals are above %rsp,
                                ;        not in the 128-byte region below it
    movl    %edi, -20(%rbp)
    movl    %esi, -24(%rbp)
    ; ... (rest is conceptually the same, just different placement guarantee)
```

At `-O2` with Red Zone enabled, the compiler may skip the entire frame:

```asm
sum:
    leal    (%rdi,%rsi), %eax   ; just add directly into return register
    ret                         ; no push, no subq, no leave — just return
```

---

### Red Zone and Signals

The 128-byte guarantee is key. When a signal arrives:

| Without Red Zone | With Red Zone |
| --- | --- |
| Signal handler could write anywhere near `%rsp` | Signal handler must skip past the 128-byte zone below `%rsp` |
| No safe scratch area exists | The zone below `%rsp` is your private scratch space |

This is why it's called a **Red** Zone — it's marked off-limits for everything except the current function.

---

### Summary

| Question | Answer |
| --- | --- |
| What is the Red Zone? | 128 bytes below `%rsp` guaranteed safe by ABI |
| What does it save? | Stack adjustment instructions in leaf functions |
| Who benefits? | Simple functions that don't call other functions |
| Can I use it explicitly? | No — it's a compiler optimization, not an instruction you invoke |
| What if my function calls another? | Don't use it — the callee will overwrite those 128 bytes |
| Available on Windows? | No — Microsoft x86-64 ABI has no Red Zone |
| How to disable? | Compile with `-mno-red-zone` |
