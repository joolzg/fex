# FEX Project Breakdown and Security Assessment

## Overview

This repository contains a C shared library that intercepts file operations using `LD_PRELOAD` so that files with a special `.fex` extension are presented as transformed or synthetic files rather than their original raw contents.

The implementation is centered in:
- [src/fex_preload.c](src/fex_preload.c)
- [include/fex.h](include/fex.h)
- [tests/test_loading.c](tests/test_loading.c)

## What the project does

At a high level, it:

- Hooks libc file APIs using `LD_PRELOAD`
- Tracks opened files and directory file descriptors
- Detects `.fex` files by path inspection
- Rewrites file metadata such as size and block counts
- Returns generated file content during `read()` and `fread()` operations
- Allows compatibility with standard file APIs such as `open`, `openat`, `lseek`, `fopen`, and `stat`

This means a process can be tricked into seeing a file as a different size and content shape than what is actually stored on disk.

## Project structure summary

### 1. Public API and tracking model

The file [include/fex.h](include/fex.h) defines the main data model used to track each `.fex` file. It stores:

- file descriptor
- `FILE *` pointer
- original filename
- original file size
- generated header/footer strings
- simulated position and size
- tracking metadata for buffers and blocks
- linked-list pointer for tracking entries

This structure shows that the library treats each intercepted file as a tracked object with synthetic state.

### 2. Runtime interception layer

The file [src/fex_preload.c](src/fex_preload.c) contains the actual logic. It overrides functions including:

- `open` / `openat`
- `close`
- `read`
- `lseek`
- `fopen` / `fread` / `fseek` / `ftell`
- `stat` / `fstat` / `fstatat`

The library resolves paths, checks whether a file should be processed as `.fex`, and then either:

- serves a transformed virtual file view,
- or falls back to the original libc implementation.

### 3. Test harness

The file [tests/test_loading.c](tests/test_loading.c) is minimal but important: it verifies the library can be loaded with `dlopen()` and that the `fex_init` symbol can be found and invoked.

This is a basic sanity test for the preload library itself, not a full behavioral validation suite.

## How the fake file behavior is implemented

The code is designed to simulate a file that is not the same as the real one on disk.

It does this by:

1. Detecting `.fex` candidates from the filename or resolved path
2. Generating synthetic header/footer data strings
3. Calculating a simulated file size larger than the original
4. Intercepting `read()` and `fread()` calls
5. Returning transformed byte data rather than the raw original file bytes

The synthetic content is built from internal data tables and generated strings, including hex-like byte sequences. The code also keeps per-file position tracking so seeks and reads appear to move through a consistent virtual stream.

## Security assessment

### Overall classification

This project should be considered a research or experimentation prototype, not a production-safe security component.

### Security risks

#### 1. Global process override via `LD_PRELOAD`

The most important risk is that the library modifies process behavior globally whenever loaded. This means any program that uses the library can have its file calls intercepted, potentially altering:

- file reads
- file sizes
- metadata
- path resolution

This is powerful and can be used to hide, rewrite, or manipulate data in ways that are difficult to detect at the application layer.

#### 2. Memory safety concerns

The project is written in C and includes:

- raw pointer manipulation
- manual allocations
- linked-list tracking
- offset arithmetic for file simulation
- custom buffer loading logic

Because of this, there are multiple opportunities for:

- invalid pointer dereferences
- out-of-bounds reads/writes
- mis-sized buffer handling
- race conditions in shared tracking state

These are classic issues for native code that intercepts low-level OS operations.

#### 3. File metadata tampering

The code modifies `stat` and `fstat` results for tracked `.fex` files. This means consumer programs may see a file that appears larger or structured differently from the actual file stored on disk.

That can confuse:

- validation logic
- file integrity checks
- backup systems
- security or auditing tools

#### 4. Abuse of untrusted path input

The library decides whether to treat files specially based on path resolution. If this were used with untrusted or broad path matching, it could interfere with unexpected files and hide the true behavior of the system from normal inspection.

#### 5. Logging and debug surfaces

The code contains debug logging and environment-controlled switches such as `FEX_DEBUG` and `FEX_SIMPLE`. In a real deployment, those could leak internal behavior or expose sensitive filenames and access patterns.

## Threat model

### In scope

- developer or researcher use in a trusted local environment
- testing of file virtualization behavior
- educational or offensive security research contexts

### Out of scope / unsafe

- production deployment in security-sensitive environments
- use in privileged daemons, service accounts, or system-level tooling
- use with untrusted or adversarially controlled processes
- use in setuid contexts or any environment where process integrity is critical

## Recommended mitigations

1. Restrict transformations to allowlisted file paths or names only.
2. Reject all write-related operations on transformed files.
3. Require explicit opt-in instead of automatic preload behavior.
4. Add strict bounds checks before all block and buffer access.
5. Disable debug logging by default.
6. Ensure temp files use secure permissions and cleanup is verified.
7. Perform adversarial testing for malformed paths, large offsets, and concurrent opens.

## Conclusion

This project demonstrates a low-level file interception mechanism using `LD_PRELOAD` and custom file tracking. It is a useful research prototype, but it is not safe for general production use.

The library alters OS-level file semantics in ways that are powerful and potentially dangerous. It should be treated as a high-risk technical demonstration rather than a trusted security boundary.

## Verified status

The project was built and tested successfully in the current workspace:

- build: successful via `cmake --build build`
- tests: passed via `ctest --test-dir build --output-on-failure`

Evidence from the last verification run:

- 1/1 tests passed
- 0 tests failed

This confirms the project compiles and the included automated test passes, but it does not mean the code is safe for production deployment.
