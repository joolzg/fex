# Fex

An LD_PRELOAD compatible shared library for intercepting file I/O functions.

## Description

Fex is a C library that uses LD_PRELOAD to intercept and monitor file I/O system calls and library functions. It can be used for debugging, profiling, or implementing custom file I/O behavior without modifying existing applications.

## Intercepted Functions

The library intercepts the following functions:

**File Descriptor Functions:**
- `open`, `openat`, `close`, `read`, `lseek`

**FILE Stream Functions:**
- `fopen`, `fclose`, `fread`, `fseek`, `ftell`, `rewind`
- `fgetpos`, `fsetpos`

**Character I/O Functions:**
- `fgetc`, `fgets`, `getc`, `ungetc`

**File Status Functions:**
- `feof`, `ferror`, `clearerr`, `fileno`

**File Statistics Functions:**
- `stat`, `fstat`, `fstatat`

## Prerequisites

- CMake 3.16 or higher
- GCC or Clang compiler
- Make (or Ninja)
- Linux operating system

## Building

### Clone the repository
```bash
git clone <your-repo-url>
cd Fex
```

### Build the project
```bash
mkdir -p build
cd build
cmake ..
make
```

This will create:
- `build/src/libfex.so` - The LD_PRELOAD shared library
- `build/src/test_app` - A test application for demonstration

### Run tests
```bash
make test
# or
ctest
```

## Usage

### Basic Usage

```bash
# Method 1: Set environment variables
export LD_PRELOAD=/path/to/libfex.so
export FEX_DEBUG=1  # Optional: enable debug output
your_application

# Method 2: One-line command
LD_PRELOAD=./build/src/libfex.so FEX_DEBUG=1 your_application
```

### Demo Script

Run the included demo script to see the library in action:

```bash
./demo.sh
```

### Debug Output

Set the `FEX_DEBUG` environment variable to enable detailed logging:

```bash
export FEX_DEBUG=1
LD_PRELOAD=./build/src/libfex.so ls /etc
```

## Example Output

When `FEX_DEBUG=1` is set, you'll see output like:

```
[FEX] FEX library initialized
[FEX] open(/etc/passwd, 0, 0)
[FEX] open() returned 3
[FEX] fstat(3, 0x7fff12345678)
[FEX] fstat() returned 0
[FEX] read(3, 0x7fff12345000, 255)
[FEX] read() returned 1024
[FEX] close(3)
[FEX] close() returned 0
```

## Project Structure

```
Fex/
├── CMakeLists.txt              # Main CMake configuration
├── src/                        # Source files
│   ├── CMakeLists.txt         # Source CMake configuration
│   ├── fex_preload.c          # Main LD_PRELOAD implementation
│   └── test_app.c             # Test application
├── include/                    # Header files
│   └── fex.h                  # Function prototypes and types
├── tests/                      # Test files
│   ├── CMakeLists.txt         # Test CMake configuration
│   └── test_loading.c         # Library loading tests
├── build/                      # Build directory (created during build)
├── demo.sh                     # Demo script
└── README.md                   # This file
```

## Development

### Debug Build
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Release Build
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Adding New Function Intercepts

1. Add function pointer type to `include/fex.h`
2. Declare static pointer variable in `fex_preload.c`
3. Initialize pointer in `fex_init()` using `dlsym()`
4. Implement the wrapper function with logging
5. Call the original function via the function pointer

## Use Cases

- **Debugging**: Monitor file access patterns in applications
- **Profiling**: Track I/O performance and bottlenecks
- **Security**: Audit file access for security analysis
- **Testing**: Mock file operations for unit testing
- **Development**: Debug file-related issues in applications

## Limitations

- Only works on Linux systems that support LD_PRELOAD
- May not work with statically linked binaries
- Some security-enhanced systems may restrict LD_PRELOAD usage
- Not compatible with applications that use alternative libc implementations

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.