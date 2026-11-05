# Memory Corruption Test Framework

This directory contains a comprehensive test framework designed to detect memory corruption issues in the FEX library, specifically the type of bug we found with the hex_table string termination.

## Test Files

### test_memory_corruption.c
A comprehensive test framework that:

1. **Clean Hex Output Test**: Verifies that .fex files produce clean hex output without format string corruption
2. **Hex Table Integrity Test**: Tests specific problematic bytes (0xf9-0xff) that were affected by the corruption
3. **Memory Boundary Test**: Tests reading across block boundaries with different block sizes
4. **Random Data Corruption Test**: Stress tests with random data to catch edge cases

## Key Features

### Corruption Detection
- Scans output for format specifiers (`%ld`, `%d`, `%s`, etc.)
- Looks for debug string fragments that leaked into hex data
- Validates hex patterns are properly formed
- Checks for non-printable characters in unexpected places

### Isolation Testing
- Uses fork() to run tests in isolated processes with LD_PRELOAD
- Tests different FEX_BLOCK_SIZE values to stress boundary conditions
- Disables debug output to focus on data corruption

### Comprehensive Coverage
- Tests all byte values (0x00-0xFF)
- Tests block boundary conditions
- Tests with different file sizes
- Validates specific patterns that were corrupted

## Usage

```bash
# Build the FEX library first
make

# Compile and run the test framework
gcc -o test_memory_corruption tests/test_memory_corruption.c
./test_memory_corruption
```

## Expected Output

### With the Bug (before fix):
```
❌ FAIL: Clean hex output
   Error: String corruption detected in output
❌ FAIL: Hex table integrity
   Error: Hex table integrity test failed with code 2
```

### After Fix:
```
✅ PASS: Clean hex output
✅ PASS: Hex table integrity
✅ PASS: Memory boundaries
✅ PASS: Random data corruption
```

## How It Caught the Bug

The original bug was in the hex_table declaration:
```c
static const char hex_table[256][6] = {
    "0x00, ", "0x01, ", ...  // Missing null terminators!
};
```

When string functions tried to use these entries, they read past the 6-byte boundary into adjacent memory, picking up format strings from debug logging functions.

The test framework catches this by:
1. Creating .fex files with specific byte patterns
2. Reading the simulated output
3. Scanning for corruption patterns like "%ld", "blocks=", etc.
4. Validating expected hex patterns are present and correctly formatted

## Testing Strategy

This framework can be extended to catch similar bugs:
- String handling issues
- Buffer overflows in hex generation
- Block boundary problems
- Memory corruption from threading issues
- Format string vulnerabilities

The key insight is that **data corruption often has recognizable patterns** - in this case, debug format strings appearing in hex data was a clear indicator of memory boundary violations.