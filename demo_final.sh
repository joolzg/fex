#!/bin/bash

echo "=== FEX Library Demo - Complete Functionality ==="
echo ""
echo "This demo shows the complete FEX library functionality:"
echo "1. File I/O interception with LD_PRELOAD"
echo "2. Selective tracking of .fex files opened for reading only"
echo "3. Automatic C code generation for tracked files"
echo ""

# Create some demo .fex files with different content
echo "Creating demo .fex files..."
echo "Hello World!" > /tmp/hello.fex
echo -e "Binary\x00\x01\x02\x03Data" > /tmp/binary.fex
echo "Configuration file content" > /tmp/config-file.fex
echo "Asset data for game" > /tmp/game_asset.fex

echo ""
echo "=== Demo 1: Reading .fex files (should be tracked) ==="
echo ""
echo "Reading hello.fex:"
cat /tmp/hello.fex
echo ""
echo "Reading binary.fex:"
xxd /tmp/binary.fex
echo ""

echo "=== Demo 2: Writing to .fex files (should NOT be tracked) ==="
echo ""
echo "Appending to hello.fex (write operation):"
echo "Additional content" >> /tmp/hello.fex
echo ""

echo "=== Demo 3: Reading files with special characters in names ==="
echo ""
echo "Reading config-file.fex (name with dash):"
cat /tmp/config-file.fex
echo ""
echo "Reading game_asset.fex (name with underscore):"
cat /tmp/game_asset.fex
echo ""

echo "=== Demo Complete ==="
echo ""
echo "Check the debug output above to see:"
echo "- Which files were tracked (read-only .fex files)"
echo "- Which files were skipped (write operations)"
echo "- The generated C code headers and footers"
echo "- Variable name sanitization for special characters"

# Cleanup
rm -f /tmp/*.fex
echo ""
echo "Demo files cleaned up."