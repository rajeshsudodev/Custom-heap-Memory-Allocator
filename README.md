# Custom Heap Memory Allocator

## Overview

A custom heap memory allocator implemented from scratch in C to explore low-level memory management, heap internals, allocator design, and systems programming concepts.

The allocator provides custom implementations of:

* `my_malloc()`
* `my_free()`
* `my_realloc()`

while managing heap memory through manual block metadata, free-list traversal, block splitting, coalescing, and runtime corruption detection.

The project was built as a deep dive into how dynamic memory allocation works internally beneath standard library functions such as `malloc()`, `free()`, and `realloc()`.

---

## Architecture Highlights

* Header/Footer metadata architecture
* First-Fit free block search
* Heap growth using `sbrk()`
* Free block reuse
* Block splitting
* Adjacent block coalescing
* In-place realloc expansion
* Realloc shrink optimization
* Canary-based corruption detection
* Double-free detection
* Heap integrity validation
* Randomized stress testing

---

## Features

### Allocation Operations

* Custom `my_malloc()`
* Custom `my_free()`
* Custom `my_realloc()`
* First-Fit allocation strategy
* Heap extension using `sbrk()`

### Memory Management

* Dynamic heap growth
* Free block reuse
* Block splitting
* Adjacent block coalescing
* In-place realloc expansion
* Realloc shrink optimization
* Fallback realloc allocation

### Safety Mechanisms

* Canary-based corruption detection
* Double-free detection
* Heap integrity validation
* Metadata consistency verification

### Testing Infrastructure

* Advanced fragmentation tests
* Alignment verification tests
* Realloc edge-case validation
* Randomized stress testing
* Continuous heap validation checks

---

## Memory Layout

Each allocation is represented using a Header-Payload-Footer architecture.

```text
+----------------------+
| Header Metadata (24) |
+----------------------+
|       Payload        |
+----------------------+
| Footer Metadata (16) |
+----------------------+
```

### Header

```c
typedef struct Header_block{
    size_t size;
    int free;
    uint32_t canary;
    struct Header_block *next;
} Header_block;
```

### Footer

```c
typedef struct Footer_block{
    size_t size;
    int free;
    uint32_t canary;
} Footer_block;
```

The footer mirrors critical metadata and assists with integrity validation.

---

## Allocation Strategy

### my_malloc()

Allocation follows a First-Fit strategy:

1. Scan free list.
2. Reuse suitable free block.
3. Split oversized blocks when beneficial.
4. Extend heap using `sbrk()` if no reusable block exists.

```text
Request
   |
   v
Find Free Block
   |
   +---- Found ----> Reuse / Split
   |
   +---- Not Found -> Extend Heap
```

---

## Block Splitting

When a free block is significantly larger than the requested size:

```text
Before

+--------------------------+
|      Large Free Block    |
+--------------------------+

After

+------------+-------------+
| Used Block | Free Block  |
+------------+-------------+
```

This reduces internal fragmentation and improves heap utilization.

---

## Coalescing

Adjacent free blocks are merged to reduce external fragmentation.

```text
Before

+---------+---------+
| Free A  | Free B  |
+---------+---------+

After

+-------------------+
|  Merged Free Block|
+-------------------+
```

---

## Fragmentation Recovery Example

```text
Heap Before

+------+--------+------+--------+
|Used  | Free   |Used  | Free   |
| 64B  | 128B   |256B  | 512B   |
+------+--------+------+--------+

Free(Used 256B)

+------+--------+------+--------+
|Used  | Free   | Free | Free   |
| 64B  | 128B   |256B  | 512B   |
+------+--------+------+--------+

After Coalescing

+------+------------------------+
|Used  |       Free 896B        |
| 64B  |                        |
+------+------------------------+
```

This demonstrates how coalescing reduces external fragmentation by combining adjacent free regions into larger reusable blocks.

---

## Reallocation Paths

The allocator supports three realloc scenarios.

### Shrink

Split the current block and return excess memory to the free list.

### Expand In-Place

Merge with an adjacent free block when possible.

### Fallback Allocation

Allocate a new block, copy data, and free the old block.

```text
Realloc
   |
   +--> Shrink
   |
   +--> Expand In Place
   |
   +--> Allocate + Copy + Free
```

---

## Security Features

### Canary Protection

Each block stores a canary value in both header and footer.

```c
#define CANARY 0xDEC0DE
```

Corruption detection occurs during deallocation and validation.

### Double-Free Detection

Attempts to free an already freed block are detected and reported.

### Heap Validation

Continuous structural verification ensures metadata consistency across allocator operations.

---

## Advanced Testing Suite

The allocator includes dedicated validation scenarios.

### Fragmentation Saw

Creates alternating allocation/free patterns to simulate fragmentation.

### Alignment Exhaustion

Tests odd-sized allocations to verify alignment guarantees.

### Zero-Byte Realloc

Validates POSIX-compliant realloc behavior.

### Cascade Coalescing

Verifies multi-block merge operations across adjacent free regions.

---

## Stress Testing

A randomized stress-testing framework performs:

* 10,000 allocator operations
* Random allocations
* Random frees
* Random reallocations
* Payload write verification
* Heap validation after every operation

The allocator successfully completes all stress-test phases without heap corruption.


## Bug Fix
### Heap Validation Return Value

**Issue:**
The cascade coalescing and final heap validation tests incorrectly interpreted the return value of `heap_validate()`.

**Root Cause:**
The validation function returns:
- `1` when validation succeeds.
- `-1` when validation fails.

However, the calling conditions were checking for `0` as the success value, resulting in successful validation being incorrectly reported as a failure.

**Changes Made:**
- Corrected the success-condition checks from `heap_validate() == 0` to `heap_validate() == 1`.
- Updated both the cascade coalescing test and final heap validation test.

**Reason:**
To align the caller's conditional checks with the actual return-value convention of the validation function.

**Result:**
Successful validation is now correctly identified and reported by the test suite.
---

## Project Structure

```text
Custom_Heap_Allocator_Project/
│
├── include/
│   ├── allocator.h
│   └── allocator_test.h
│
├── src/
│   ├── allocator.c
│   ├── debug.c
│   ├── main.c
│   └── test.c
│
├── object/
│
├── Makefile
├── .gitignore
└── README.md
```

---

## Build

Compile:

```bash
make
```

Run:

```bash
make run
```

Clean:

```bash
make clean
```

Rebuild:

```bash
make rebuild
```

---

## Technologies Used

* C Programming Language
* GCC
* Linux
* GNU Make
* GDB
* Valgrind
* Git

---

## Concepts Explored

* Heap Memory Internals
* Dynamic Memory Allocation
* Metadata Layout Design
* Pointer Arithmetic
* Free List Management
* Memory Fragmentation
* Block Splitting
* Memory Coalescing
* Defensive Programming
* Systems Programming
* Runtime Validation

---

## Author

**Rajesh P**

Custom Heap Memory Allocator developed to study allocator internals, low-level memory management, and systems programming fundamentals through hands-on implementation.
