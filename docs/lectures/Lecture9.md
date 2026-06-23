## Agenda
How does C aggregate data into larger structures?
a. Array allocation and access  
b. Pointer arithmetic  
c. Nested arrays (multidimensional)  
d. Heterogeneous data structures — structs, unions, alignment  

---

## Array Allocation and Access

### Basic Principles

Arrays in C occupy **contiguous memory locations**. The array name acts as a pointer to the first element, so accessing `A[i]` is just address arithmetic under the hood.

### Declaration and Memory Layout

```c
T A[N];
```

| Parameter | Meaning |
| --- | --- |
| `T` | Element data type |
| `N` | Number of elements |
| `L` | Size of type `T` in bytes (given by `sizeof(T)`) |

**Total memory allocated:** `L × N` bytes, all sitting next to each other.

```
Higher addresses
┌─────────────────────────┐
│  A[N-1]                 │   ← last element
├─────────────────────────┤
│    ...                  │
├─────────────────────────┤
│  A[1]                   │
├─────────────────────────┤
│  A[0]  (== *A)          │   ← first element, array name points here
└─────────────────────────┘ ← base address x_A
Lower addresses
```


---

### Address Calculation

If the array starts at address `x_A` and each element is `L` bytes wide:

| Access | Computed Address |
| --- | --- |
| `A[0]` | `x_A + L × 0` = `x_A` |
| `A[i]` | `x_A + L × i` |
| `A[N-1]` | `x_A + L × (N - 1)` |

---

### Example Arrays

| Array Declaration | Element Type | Element Size (`L`) | Total Size |
| --- | --- | --- | --- |
| `char A[12]` | `char` | 1 byte | 12 bytes |
| `char *B[8]` | `char pointer` | 8 byte | 12 bytes |
| `double C[6]` | `double` | 8 bytes | 48 bytes |
| `int E[10]` | `int` | 4 bytes | 40 bytes |

---

### Array Access in Assembly

```asm
movl (%edx, %ecx, 4), %eax
# memory address = edx + (ecx * 4)
```

Assuming `int E[]`, with `%edx` holding the base address and `%ecx` holding index `i`:

| Component | Role |
| --- | --- |
| `%edx` | Base address of array (`x_E`) |
| `%ecx` | Index value (`i`) |
| Scale factor `4` | Element size (`sizeof(int)`) |

This computes the effective address `x_E + 4 × i` and loads `E[i]` into `%eax`.

---

### Summary

| Concept | Mechanism |
| --- | --- |
| Memory layout | Contiguous block of `L × N` bytes |
| Array name | Pointer to first element (`&A[0]`) |
| Address of `A[i]` | `base_address + element_size × i` |
| Assembly access | Scaled-index addressing mode: `(%base, %index, scale)` |

---

## Pointer Arithmetic

### What Is It?

If a pointer `p` points to type `T` and holds address `x_p`, then:

```c
p + i    → evaluates to    x_p + L × i
```

where `L = sizeof(T)`. The compiler automatically scales by the element size — you think in **elements**, the hardware works in **bytes**.

> Think of it as taking steps across a field. If `T` is an `int` (4 bytes), each "step" covers 4 metres. `p + 3` means three steps, landing at `x_p + 12`. The compiler does the multiplication for you.

---

### Address-of and Dereference Operators

| Operator | Meaning | Example |
| --- | --- | --- |
| `&Expr` | Returns address of object | `&A[2]` gives pointer to third element |
| `*AExpr` | Returns value stored at address | `*(A + 1)` reads the second element |

---

### Fundamental Equivalence

```c
A[i]        is identical to        *(A + i)
```

Both compile to the exact same machine code. Array indexing **is** pointer arithmetic in disguise.

| Expression | Meaning | Equivalent Pointer Form |
| --- | --- | --- |
| `E` | Pointer to first element | `&E[0]` |
| `E[0]` | First element | `*E` |
| `E[i]` | i-th element | `*(E + i)` |
| `&E[2]` | Address of third element | `E + 2` |
| `E + i - 1` | Pointer to (i-1)-th element | — |
| `*(E + i - 3)` | Value 3 positions before index i | `E[i - 3]` |
| `&E[i] - E` | Difference in number of elements | `i` |

---

### Summary

| Concept | Rule |
| --- | --- |
| Pointer arithmetic | Scaled by `sizeof(pointed_type)` automatically |
| `A[i]` == `*(A + i)` | Array indexing is just syntactic sugar for pointer arithmetic |
| `&E[i] - E` | Gives index `i` (difference measured in elements, not bytes) |

---

## Nested Arrays (Multidimensional) (Optional)

### Two-Dimensional Arrays

```c
int A[5][3];
```

is equivalent to:

```c
typedef int row3_t[3];
row3_t A[5];   // array of 5 rows, each row has 3 ints
```

| Dimension | Meaning | Size Contribution |
| --- | --- | --- |
| `[5]` | Number of rows | 5 × (row size) |
| `[3]` | Elements per row | 3 × `sizeof(int)` = 12 bytes |

**Total:** 5 × 12 = **60 bytes**.

---

### Row-Major Storage Order

C stores multidimensional arrays in **row-major order** — the rightmost index changes fastest:

```
Memory layout of A[5][3]:

Address increasing →

A[0][0] | A[0][1] | A[0][2] | A[1][0] | A[1][1] | A[1][2] | ...
 ───────┼──────────┼──────────┼─────────┼─────────┼─────────
 │      Row 0     │   Row 1   │         ...continues with Row 2, 3, 4...
```

> Think of it as filling out a form row by row — complete the first row entirely before moving to the next.

---

### Address Formula

For `T D[R][C]` where `L = sizeof(T)`:

| Access | Address |
| --- | --- |
| `D[i][j]` | `x_D + L × (C × i + j)` |

**Example:** For `int A[5][3]`, `L = 4`:

```
Address of A[i][j] = x_A + 4 × (3i + j)
```

---

### Compiler Optimization — Address Computation

Instead of computing multiplications at runtime, compilers use:

| Technique | Example |
| --- | --- |
| Shifts | `3i` becomes `(i << 1) + i` when power-of-2 |
| Additions | Pre-add instead of multiply in loops |
| LEA instructions | `lea (%rax, %rax, 4), %edx` computes `5 × rax` without an integer multiplier |

---

### Summary

| Question | Answer |
| --- | --- |
| How are 2D arrays stored? | Row-major — entire row before the next |
| Address of `D[i][j]`? | `base + sizeof(T) × (columns × i + j)` |
| Why does order matter? | Accessing along rows is cache-friendly; jumping columns skips memory |

---

## Fixed-Size Arrays  (Optional)

### The Advantage

When dimensions are known at compile time, the compiler can use constants and eliminate repeated calculations:

```c
#define N 16
typedef int fix_matrix[N][N];
```

For matrix multiplication:

```c
result += A[i][j] * B[j][k];
```

The compiler rewrites this into efficient pointer traversals instead of recalculating `N × i + j` on every iteration.

---

### Optimization 1 — Row Pointer

```c
int *Arow = &A[i][0];
// Instead of A[i][j], use:
Arow[j]    // just a simple increment, no multiplication
```

| Before | After |
| --- | --- |
| `A[i][j]` → `x_A + 4 × (N×i + j)` every time | `Arow[j]` → just walk through a flat row |

---

### Optimization 2 — Column Stride Pointer

```c
int *Bptr = &B[0][k];
// In the j-loop:
result += Arow[j] * Bptr[j * N];
// Or stride-based:
for (j = 0; j < N; j++) {
    result += Arow[j] * Bptr[j];   // if Bptr walks column k
}
```

| Technique | What it avoids |
| --- | --- |
| Row pointer (`Arow`) | Repeated `N × i` multiplication |
| Column pointer (`Bptr`) with stride of `N` | Repeated 2D index calculation for `B[j][k]` |

> Think of it as unrolling a map and laying it flat on the table — once you're on the right row, walking along is just stepping forward. No need to recalculate your GPS coordinates every step.

---

### Summary

| Optimization | How It Works |
| --- | --- |
| Row pointer | Cache `&A[i][0]`, increment for each column access |
| Column stride pointer | Step by `N` instead of recomputing `j × N + k` |
| LEA for address math | Compute `base + scale × index` without a multiply instruction |

---

## Variable-Size Arrays (VLA)  (Optional)

### What Are They?

Introduced in **C99**, VLAs allow array dimensions to be determined at **runtime**:

```c
int var_ele(int n, int A[n][n], int i, int j)
{
    return A[i][j];
}
```

---

### Address Calculation

| Array Type | Formula for `A[i][j]` |
| --- | --- |
| Fixed-size: `int A[N][N]` | `x_A + 4 × (N × i + j)` — `N` is a compile-time constant |
| VLA: `int A[n][n]` | `x_A + 4 × (n × i + j)` — `n` must be loaded from memory at runtime |

---

### Key Difference in Assembly

| Fixed-size arrays | Variable-size arrays |
| --- | --- |
| Compiler uses `leal` to combine shifts and adds | Must use `imull` to compute `n × i` at runtime |
| Constant factor folded into instruction | Extra register needed to hold `n` |
| Generally faster | Slightly more overhead per access |

---

### Optimization Still Applies

Even with VLAs, compilers still:

| Technique | Effect |
| --- | --- |
| Row pointers | Cache base of current row, increment linearly |
| Column pointers with stride | Add `n` (stored in a register) instead of multiply |
| Loop-invariant code motion | Pull dimension loads outside inner loops |

---

### Summary

| Question | Answer |
| --- | --- |
| What makes VLAs different? | Dimensions are runtime values, not compile-time constants |
| Assembly impact? | Needs actual `imull` instead of fused `lea`/shifts |
| Can they still be optimized? | Yes — row/column pointer tricks still apply inside loops |

---

## Heterogeneous Data Structures  (Optional)

C provides two tools for combining different types:

| Keyword | Purpose | Memory Behavior |
| --- | --- | --- |
| `struct` | Combine multiple fields of different types | Fields stored sequentially, each at its own offset |
| `union` | Interpret same memory as different types | All fields share the **same** starting address |

---

### Structures

#### Declaration and Field Access

```c
struct rect {
    int llx;
    int lly;
    int color;
    int width;
    int height;
};
```

| Access Method | Syntax | What It Does |
| --- | --- | --- |
| Direct access | `r.width` | Reads the `width` field of struct `r` |
| Pointer access | `rp->width` | Reads `width` through pointer `rp` |
| Equivalent form | `(*rp).width` | Dereference first, then select field — same as above |

> **`->`** is syntactic sugar for dereferencing and accessing in one step. It reads naturally: *"follow the pointer, then pick the field."*

---

#### Structure Memory Layout

```c
struct rec {
    int i;
    int j;
    int a[3];
    int *p;
};
```

| Offset | Field | Size |
| --- | --- | --- |
| 0 | `i` | 4 bytes |
| 4 | `j` | 4 bytes |
| 8 | `a[0]` | 4 bytes |
| 12 | `a[1]` | 4 bytes |
| 16 | `a[2]` | 4 bytes |
| 20 | `p` | 4 bytes (pointer, 32-bit) |

**Total size: 24 bytes.**

```
┌─────────────────────────┐ ← offset 0  (%edx)
│      field i            │   movl 0(%edx), %eax  → reads r.i
├─────────────────────────┤ ← offset 4
│      field j            │   movl 4(%edx), %eax  → reads r.j
├─────────────────────────┤ ← offset 8
│      a[0]               │
├─────────────────────────┤ ← offset 12
│      a[1]               │
├─────────────────────────┤ ← offset 16
│      a[2]               │
├─────────────────────────┤ ← offset 20
│      pointer p          │
└─────────────────────────┘ ← offset 24 (total)
```

---

#### How the Compiler Accesses Fields

The compiler replaces field names with **constant byte offsets** from the base address:

| C Expression | Assembly (assuming struct at `%edx`) |
| --- | --- |
| `rp->i` | `movl 0(%edx), %eax` |
| `rp->j` | `movl 4(%edx), %eax` |
| `rp->a[1]` | `movl 12(%edx), %eax` |
| `rp->p` | `movl 20(%edx), %eax` |

> **Key observation:** After compilation, field names **disappear completely**. Only numeric offsets remain. The struct is just a region of memory with known positions.

---

### Summary

| Concept | Detail |
| --- | --- |
| Memory layout | Fields laid out sequentially at fixed offsets |
| `->` operator | Syntactic sugar for `(*pointer).field` |
| Assembly view | Field access = load/store at constant offset from base pointer |
| What's lost? | Field names vanish — only byte offsets survive compilation |

---

### Unions

#### What Makes Them Different?

Unlike structs, **all union members share the same starting address** (offset 0). The union is large enough to hold its largest member:

```c
union U3 {
    char c;           // 1 byte
    int i[2];         // 8 bytes
    double v;         // 8 bytes
};
```

| Property | Value |
| --- | --- |
| `sizeof(union U3)` | **8 bytes** (size of largest member: `double`) |
| All field offsets | **0** — every member starts at the same address |

```
┌─────────────────────────┐ ← offset 0
│  union.c                │   ← occupies byte [0]
├─────────────────────────┤
│  (unused by c)          │   bytes [1..7] wasted when c is active
├─────────────────────────┤ ← offset 0 (same location!)
│  union.i[0]             │   ← occupies bytes [0..3]
├─────────────────────────┤ ← offset 4
│  union.i[1]             │   ← occupies bytes [4..7]
├─────────────────────────┤ ← offset 0 (same location!)
│  union.v (double)       │   ← occupies bytes [0..7] entirely
└─────────────────────────┘
```

> Think of it as a **chameleon slot** — the same memory space can hold different types depending on which member you last wrote to. Writing through one member overwrites what another member sees.

---

#### Use Case 1 — Saving Memory (Variant Types)

```c
union NODE_U {
    struct {
        NODE_U *left;
        NODE_U *right;
    } internal;

    double data;
};
```

A binary tree node stores **either** child pointers (internal node) **or** a value (leaf node), never both:

| Node Type | Active Member | Bytes Used |
| --- | --- | --- |
| Internal | `.internal.left` + `.internal.right` | 8 bytes (two pointers) |
| Leaf | `.data` | 8 bytes (one double) |

---

#### Use Case 2 — Type Reinterpretation

```c
unsigned float2bit(float f)
{
    union {
        float f;
        unsigned u;
    } temp;

    temp.f = f;
    return temp.u;    // same bits, read as unsigned int
}
```

| Step | What Happens |
| --- | --- |
| Write `f` through `.f` | Stores IEEE 754 float representation in memory |
| Read through `.u` | Interprets the **same bytes** as a raw 32-bit integer |

This lets you inspect the bit pattern of a float without pointer casting.

> ⚠️ **Warning:** Unions bypass C's type safety. Reading through a different member than what was written is technically undefined behavior in strict C (though widely supported as a common extension). Improper use causes subtle, hard-to-debug memory corruption.

---

### Summary

| Concept | Struct | Union |
| --- | --- | --- |
| Memory for fields | Each field gets its own space | All fields share the same space |
| Total size | Sum of all fields (+ padding) | Size of largest member |
| Field offsets | Unique per field | Always 0 |
| Typical use | Record-like data (name, age, etc.) | Variant types, type punning |

---

## Data Alignment and Padding  (Optional)

### The Rule

Processors require certain data types to begin at addresses that are multiples of their size:

| Type Size | Required Alignment |
| --- | --- |
| 1-byte (`char`) | Any address (multiple of 1) |
| 2-byte (`short`) | Multiple of 2 |
| 4-byte (`int`, pointer on 32-bit) | Multiple of 4 |
| 8-byte (`double`, pointer on 64-bit) | Multiple of 8 |

> Think of it as furniture placement — a sofa must start at a position divisible by its width, while a small plant can go anywhere. Misalignment is like trying to fit a wide table halfway between two floor tiles.

---

### Why Does Alignment Matter?

| Reason | Detail |
| --- | --- |
| Hardware simplicity | CPU memory buses transfer data in fixed-size chunks — aligned access fits naturally |
| Performance | Aligned access = one memory transaction; misaligned = potentially two or more |
| Correctness | Some architectures **fault** on misaligned access (ARM, MIPS); x86 tolerates it but penalizes speed |

---

### Example: Field Padding

#### Without Padding (Incorrect)

```c
struct S1 {
    int i;     // 4 bytes
    char c;    // 1 byte
    int j;     // 4 bytes
};
```

| Offset | Field | Aligned? |
| --- | --- | --- |
| 0 | `i` (4 bytes) | ✅ (0 is multiple of 4) |
| 4 | `c` (1 byte) | ✅ (any address works for char) |
| 5 | `j` (4 bytes) | ❌ (5 is NOT a multiple of 4!) |

---

#### Actual Compiler Layout (With Padding)

| Offset | Content | Purpose |
| --- | --- | --- |
| 0-3 | `i` | int at aligned address |
| 4 | `c` | char, no alignment requirement |
| 5-7 | **(padding)** | Inserted by compiler to align `j` |
| 8-11 | `j` | int at properly aligned address |

**Total: 12 bytes** (not 9). The gap between `c` and `j` is wasted space paid for correctness.

---

### End Padding — Why It Exists

```c
struct S2 {
    int i;     // 4 bytes
    int j;     // 4 bytes
    char c;    // 1 byte
};
```

| Offset | Content |
| --- | --- |
| 0-3 | `i` |
| 4-7 | `j` |
| 8 | `c` |
| 9-11 | **(end padding)** |

**Total: 12 bytes** (not 9). The compiler pads the end so that in an **array of structs**, every element is still properly aligned:

```
struct S2 arr[3];

arr[0]: offset 0..11   ← 12 bytes (padded)
arr[1]: offset 12..23  ← starts at multiple of 4 ✅
arr[2]: offset 24..35  ← starts at multiple of 4 ✅
```

Without end padding, `arr[1]` would start at offset 9 — misaligning its first `int`.

---

### Alignment Summary

| Where | Why |
| --- | --- |
| **Between fields** | To ensure each field starts at a properly aligned address |
| **At end of struct** | To ensure array elements of that struct type are all aligned |

> Think of padding as the empty space between items in a shipping box. You don't want it, but without it, things either don't fit or break.

---

### Summary

| Question | Answer |
| --- | --- |
| What is alignment? | Data must start at an address divisible by its size |
| Why pad between fields? | To keep each field's offset properly aligned |
| Why pad at the end? | So arrays of that struct type stay aligned |
| Cost of padding? | Wasted bytes — usually a small price for performance and correctness |

---

## Key Takeaways

| Topic | Core Idea |
| --- | --- |
| **Array memory** | Contiguous block; name is pointer to first element |
| **Address calculation** | `A[i]` → `base_address + sizeof(T) × i` |
| **Pointer arithmetic** | Scaled by element size; `A[i] ≡ *(A + i)` |
| **2D arrays** | Row-major storage; rightmost index varies fastest |
| **Fixed-size optimization** | Compiler eliminates multiplications with LEA, shifts, pointers |
| **VLAs** | Runtime dimensions require actual multiply instructions |
| **Structs** | Fields at fixed offsets; names disappear after compilation |
| **Unions** | All members share offset 0; size = largest member |
| **Alignment + padding** | Compiler inserts invisible bytes for hardware efficiency |
