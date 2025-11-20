# FEX - File Extension Simulator

A sophisticated LD_PRELOAD library that simulates .fex files as C array declarations, providing transparent file format transformation for legacy applications.

## Overview

FEX intercepts file I/O operations and presents .fex files as if they contain C array code with hex-encoded data. This allows legacy applications to process .fex files without modification while maintaining full compatibility with standard file operations.

## Features

### Core Functionality
- **Transparent File Simulation**: .fex files appear as C arrays with hex data
- **Complete I/O Support**: All standard file operations (fopen, fread, fgetc, fseek, etc.)
- **Lazy Block Loading**: Efficient memory usage with 4KB block management
- **Pre-computed Hex Tables**: Optimized performance with constant-time hex lookups
- **Thread-Safe Operations**: Mutex-protected file tracking and buffer management

### Advanced Capabilities
- **Persistent File Handles**: Original files kept open for efficient block loading
- **Position Tracking**: Accurate simulation of file positions and seeking
- **Error Handling**: Robust EOF detection and bounds checking
- **Memory Management**: Automatic cleanup and leak prevention
- **Debug Logging**: Comprehensive debugging support with FEX_DEBUG

## Quick Start

### Build
```bash
make
```

### Basic Usage
```bash
# Run any application with .fex file transformation
LD_PRELOAD=./build/src/libfex.so your_application file.fex

# With debug output
LD_PRELOAD=./build/src/libfex.so FEX_DEBUG=1 your_application file.fex

# Using the wrapper script
./fex_run.sh your_application file.fex
```

### Example
```bash
# View a .fex file as C array code
LD_PRELOAD=./build/src/libfex.so cat example.fex
# Output: unsigned char example[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, ...};
```

## Architecture

### File Simulation
- **Header**: `unsigned char filename[] = {`
- **Data**: Hex-encoded original content (`0xNN, ` format)
- **Footer**: `};\nunsigned long filename_SIZE = N;`

### Buffer Management
- **Block Size**: Configurable chunks (default 4KB) loaded on-demand via `FEX_BLOCK_SIZE`
- **Hex Table**: Pre-computed 256×6 lookup table for optimal performance
- **Position Mapping**: Accurate translation between simulated and real positions

### Memory Efficiency
- Lazy loading of file blocks
- Persistent file handles for repeated access
- Automatic cleanup on file close
- Memory-efficient chunk-based reading

## Environment Variables

| Variable | Description | Default | Valid Range | Example |
|----------|-------------|---------|-------------|---------|
| `FEX_DEBUG` | Enable debug logging | disabled | - | `export FEX_DEBUG=1` |
| `FEX_SHOW_STATUS` | Print file tracking status on exit | disabled | - | `export FEX_SHOW_STATUS=1` |
| `FEX_BLOCK_SIZE` | Set buffer block size in bytes | 4096 | 1024-1048576 | `export FEX_BLOCK_SIZE=8192` |

### Buffer Size Configuration

The `FEX_BLOCK_SIZE` environment variable allows you to tune memory usage and performance:

**Smaller values (1KB-2KB):**
- Lower memory usage per file
- More frequent I/O operations
- Better for systems with limited memory

**Larger values (64KB-1MB):**
- Higher memory usage per file
- Fewer I/O operations for large files
- Better sequential read performance

**Invalid values:**
- Below 1KB (1024 bytes): Falls back to default 4KB
- Above 1MB (1048576 bytes): Falls back to default 4KB
- Non-numeric values: Falls back to default 4KB

```bash
# Example: Use 8KB blocks for better performance
export FEX_BLOCK_SIZE=8192
LD_PRELOAD=./build/src/libfex.so your_program

# Example: Use 1KB blocks for memory-constrained systems
export FEX_BLOCK_SIZE=1024
LD_PRELOAD=./build/src/libfex.so your_program
```

## Advanced Usage

### Integration with Build Systems
```bash
# CMake integration
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed -ldl")
target_compile_definitions(app PRIVATE FEX_ENABLED)

# Makefile integration
LDFLAGS += -Wl,--no-as-needed -ldl
PRELOAD = LD_PRELOAD=./path/to/libfex.so
```

### Performance Optimization
The library includes several performance optimizations:
- Pre-computed hex table for O(1) byte-to-hex conversion
- Chunked reading for large file operations
- Lazy block loading to minimize memory usage
- Persistent file handles to reduce open/close overhead

## File Format Support

### Supported Operations
- ✅ `fopen()` / `fclose()`
- ✅ `fread()` / `read()`
- ✅ `fgetc()` / `getc()`
- ✅ `fseek()` / `lseek()` / `ftell()`
- ✅ `rewind()` / `fgetpos()` / `fsetpos()`
- ✅ `feof()` / `ferror()` / `clearerr()`
- ✅ `stat()` / `fstat()` / `fstatat()`
- ✅ `ungetc()` character push-back

### File Size Calculation
Original file size N bytes becomes:
- Header: ~30 bytes
- Data: N × 6 bytes (hex encoding)
- Footer: ~40 bytes
- **Total**: ~(N × 6 + 70) bytes

## Development

### Building from Source
```bash
# Standard build
make

# Debug build
make debug

# Clean build
make clean

# Install system-wide (optional)
sudo make install
```

### Testing
```bash
# Run built-in tests
make test

# Manual testing
./temp_tests/test_*.sh

# Performance testing
./demo.sh
```

### Project Structure
```
├── src/           # Source code
│   ├── fex_preload.c   # Main LD_PRELOAD implementation
│   └── test_app.c      # Test application
├── include/       # Header files
│   └── fex.h           # API definitions
├── tests/         # Test suite
├── temp_tests/    # Development test files
├── build/         # Build output directory
└── scripts/       # Utility scripts
```

## Troubleshooting

### Common Issues

**Library not loading**
```bash
# Check if library exists
ls -la build/src/libfex.so

# Verify library dependencies
ldd build/src/libfex.so
```

**Debug output not showing**
```bash
# Ensure debug is enabled
export FEX_DEBUG=1
export FEX_SHOW_STATUS=1
```

**Performance issues**
- Increase block size for large files
- Check memory usage with debug output
- Verify efficient read patterns

### Debug Information
Enable detailed logging:
```bash
export FEX_DEBUG=1
export FEX_SHOW_STATUS=1
LD_PRELOAD=./build/src/libfex.so your_app 2>&1 | grep FEX
```

## Performance

### Benchmarks
- **Small files (<4KB)**: Near-native performance
- **Large files (>1MB)**: ~10% overhead due to hex encoding simulation
- **Memory usage**: ~4KB buffer per open .fex file
- **Hex conversion**: O(1) with pre-computed lookup tables

### Optimization Tips
1. Use larger read operations when possible
2. Minimize seeking for sequential access
3. Enable buffering in your application
4. Consider block size tuning for very large files

## License

MIT License - see LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## Technical Details

For detailed technical information about the implementation, see the inline documentation in `src/fex_preload.c` and the comprehensive test suite in `tests/`.

---

**Note**: This library uses LD_PRELOAD to intercept system calls. Ensure your application and system support this mechanism for proper operation.