# Control Flow
## Condition Codes

In addition to the integer registers, the CPU maintains a set of **single-bit condition code registers** describing attributes of the most recent arithmetic or logical operation. These registers can then be tested to perform conditional branches.

### Most Useful Condition Codes

| Code | Name         | Description                                                                                      |
|------|--------------|--------------------------------------------------------------------------------------------------|
| CF   | Carry Flag   | The most recent operation generated a carry out of the most significant bit. Used to detect overflow for **unsigned** operations. |
| ZF   | Zero Flag    | The most recent operation yielded zero.                                                          |
| SF   | Sign Flag    | The most recent operation yielded a negative value.                                              |
| OF   | Overflow Flag| The most recent operation caused a two's-complement overflow — either negative or positive.      |

### Condition Codes After Addition

For the equivalent of `t = a + b` (where `a`, `b`, and `t` are integers):

| Flag | C Expression                                            | Meaning          |
|------|---------------------------------------------------------|------------------|
| CF   | `(unsigned) t < (unsigned) a`                           | Unsigned overflow|
| ZF   | `(t == 0)`                                              | Zero             |
| SF   | `(t < 0)`                                               | Negative         |
| OF   | `(a < 0 == b < 0) && (t < 0 != a < 0)`                  | Signed overflow  |

### Notes on Condition Code Behavior

- The `leal` instruction does **not** alter any condition codes, since it is intended for address computations.
- All other instructions in the arithmetic/logical set cause condition codes to be updated.
- For **logical operations** (e.g., XOR), CF and OF are set to 0.
- For **shift operations**, CF is set to the last bit shifted out, while OF is set to 0.
- The `inc` and `dec` instructions set OF and ZF but leave CF unchanged.

---

### Comparison and Test Instructions

Two instruction classes set condition codes **without altering any other registers**:

| Instruction | Based On    | Description        |
|-------------|-------------|--------------------|
| `cmp`       | `S1 - S2`   | Compare double word |
| `cmpw`      | `S1 - S2`   | Compare word       |
| `cmpb`      | `S1 - S2`   | Compare byte       |
| `test`      | `S1 & S2`   | Test double word   |
| `testw`     | `S1 & S2`   | Test word          |
| `testb`     | `S1 & S2`   | Test byte          |

- **`cmp`** behaves like `sub` but without updating the destination. Sets ZF if operands are equal.
- **`test`** behaves like `and` but without altering destinations. Typically used with the same operand twice (e.g., `testl %eax, %eax`) or with a mask to check specific bits.

---

### Set Instructions

Each `set` instruction sets a single byte to **0 or 1** based on some combination of condition codes.

#### Signed Comparisons

| Instruction | Synonyms     | Effect                                      |
|-------------|--------------|---------------------------------------------|
| `setg`      | `setnle`, `setnl`   | `D ← ~(SF ^ OF) & ~ZF` — Greater (signed >)       |
| `setge`     | `setnl`    | `D ← ~(SF ^ OF)`        — Greater or equal (>=)  |
| `setl`      | `setnge`, `setng`   | `D ← SF ^ OF`               — Less (<)             |
| `setle`     | `setng`         | `D ← (SF ^ OF) \| ZF`  — Less or equal (<=)      |

#### Unsigned Comparisons

| Instruction | Synonyms       | Effect                              |
|-------------|----------------|-------------------------------------|
| `seta`      | `setnbe`, `setnb`, `setnae` | `D ← ~CF & ~ZF` — Above (>)         |
| `setae`     | `setnb`, `setnbe`       | `D ← ~CF`       — Above or equal (>=)|
| `setb`      | `setnae`, `setna`       | `D ← CF`        — Below (<)          |
| `setbe`     | `setna`         | `D ← CF \| ZF`  — Below or equal (<=)|

#### Equality and Sign Tests

| Instruction | Synonyms | Effect                  |
|-------------|----------|-------------------------|
| `setz`      | `sete`   | `D ← ZF` — Equal / zero |
| `setnz`     | `setne`  | `D ← ~ZF` — Not equal   |
| `sets`      | —        | `D ← SF` — Negative     |
| `setns`     | —        | `D ← ~SF` — Nonnegative |

---

## Jump Instructions and Their Encodings

Under normal execution, instructions follow each other in order. A **jump instruction** causes execution to switch to a completely new position in the program, with destinations indicated by a **label** in assembly code.

### Example: Unconditional Jump

```asm
movl $0, %eax        # Set %eax to 0
jmp .L1              # Goto .L1
movl (%eax), %edx    # Skipped — would be null pointer dereference
.L1:
popl %edx            # Execution resumes here
```

The `jmp .L1` causes the program to skip over the `movl` instruction and resume at the `popl`. The assembler determines addresses of all labeled instructions and encodes jump targets as part of the jump instructions.

---

### Jump Instruction Reference

| Instruction | Synonym | Jump Condition | Description |
|-------------|---------|----------------|-------------|
| `jmp` | — | Always (1) | **Direct jump** — target encoded in instruction |
| `jmp *Operand` | — | Always (1) | **Indirect jump** — target read from register or memory |

#### Conditional Jumps: Equality and Sign

| Instruction | Synonym | Condition | Description |
|-------------|---------|-----------|-------------|
| `je` | `jz` | `ZF` | Equal / zero |
| `jne` | `jnz` | `~ZF` | Not equal / not zero |
| `js` | — | `SF` | Negative |
| `jns` | — | `~SF` | Nonnegative |

#### Conditional Jumps: Signed Comparisons

| Instruction | Synonym | Condition | Description |
|-------------|---------|-----------|-------------|
| `jg` | `jnle`, `jnl` | `~(SF ^ OF) & ~ZF` | Greater (signed >) |
| `jge` | `jnl` | `~(SF ^ OF)` | Greater or equal (>=) |
| `jl` | `jnge`, `jng` | `SF ^ OF` | Less (<) |
| `jle` | `jng` | `(SF ^ OF) \| ZF` | Less or equal (<=) |

#### Conditional Jumps: Unsigned Comparisons

| Instruction | Synonym | Condition | Description |
|-------------|---------|-----------|-------------|
| `ja` | `jnbe`, `jnb`, `jnae` | `~CF & ~ZF` | Above (unsigned >) |
| `jae` | `jnb`, `jnbe` | `~CF` | Above or equal (>=) |
| `jb` | `jnae`, `jna` | `CF` | Below (<) |
| `jbe` | `jna` | `CF \| ZF` | Below or equal (<=) |

---

### Jump Types

- **Direct jump** — target is encoded as part of the instruction, written with a label (e.g., `jmp .L1`).
- **Indirect jump** — target is read from a register or memory location:
  - `jmp *%eax` — uses value in `%eax` as the jump target
  - `jmp *(%eax)` — reads the target from memory at address in `%eax`

> The conditional jump names and conditions match those of the `set` instructions (Figure 3.11). Conditional jumps can **only be direct**.

---

### PC-Relative Encoding

Jump targets are commonly encoded as **PC-relative offsets** — the difference between the target address and the address of the instruction immediately following the jump. These offsets use **1, 2, or 4 bytes**. Alternatively, an **absolute address** uses 4 bytes to directly specify the target.

#### Example: Forward and Backward Jumps

```asm
      jle .L2          # if <=, goto dest2
.L5:  movl %edx, %eax  # dest1:
      sarl %eax
      subl %eax, %edx
      leal (%edx,%edx,2), %edx
      testl %edx, %edx
      jg .L5           # if >, goto dest1
.L2:  movl %edx, %eax  # dest2:
```

Disassembled object-code (`.o`) output:

| Offset | Bytes | Instruction | Target |
|--------|-------|-------------|--------|
| `8` | `7e 0d` | `jle 17 <silly+0x17>` | dest2 (forward) |
| `a` | `89 d0` | `mov %edx, %eax` | — |
| `c` | `d1 f8` | `sar %eax` | — |
| `e` | `29 c2` | `sub %eax, %edx` | — |
| `10` | `8d 14 52` | `lea (%edx,%edx,2), %edx` | — |
| `13` | `85 d2` | `test %edx, %edx` | — |
| `15` | `7f f3` | `jg a <silly+0xa>` | dest1 (backward) |
| `17` | `89 d0` | `mov %edx, %eax` | — |

#### Decoding PC-Relative Targets

**Forward jump (`jle` at address `0x8`, offset `0xd`):**

```
Target = (address of next instruction) + encoded_offset
       = 0xa + 0xd = 0x17  →  line 8 (dest2) ✓
```

**Backward jump (`jg` at address `0x15`, offset `0xf3`):**

```
Target = (address of next instruction) + encoded_offset
       = 0x17 + (-13) = 0xa  →  line 2 (dest1) ✓
```

> **Key convention:** The PC value for PC-relative addressing is the address of the instruction **following** the jump, not the jump itself. This dates to early processor implementations that updated the PC as the first step of executing an instruction.

### Relocation Independence

After linking, instructions are relocated to different absolute addresses, but the **PC-relative encodings remain unchanged** — because the relative offsets between source and target stay the same. This allows compact encoding (as few as 2 bytes per jump) while remaining position-independent.

| Linked Offset | Bytes | Instruction | Target |
|---------------|-------|-------------|--------|
| `804839c` | `7e 0d` | `jle 80483ab <silly+0x17>` | Same relative offset |
| `80483a9` | `7f f3` | `jg 804839e <silly+0xa>` | Same relative offset |
