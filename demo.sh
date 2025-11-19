#!/bin/bash

# FEX LD_PRELOAD Demo Script

set -e

echo "=== FEX LD_PRELOAD Library Demo ==="
echo

# Check if library exists
if [ ! -f "build/src/libfex.so" ]; then
    echo "Error: libfex.so not found. Please build the project first:"
    echo "  mkdir -p build"
    echo "  cd build"
    echo "  cmake .."
    echo "  make"
    exit 1
fi

# Check if test app exists
if [ ! -f "build/src/test_app" ]; then
    echo "Error: test_app not found. Please build the project first."
    exit 1
fi

echo "1. Running test application WITHOUT LD_PRELOAD:"
echo "   (No function interception should occur)"
echo
./build/src/test_app
echo

echo "=================================================="
echo

echo "2. Running test application WITH LD_PRELOAD:"
echo "   (All file I/O functions should be intercepted)"
echo "   Set FEX_DEBUG=1 to enable debug output"
echo
export FEX_DEBUG=1
export LD_PRELOAD="./build/src/libfex.so"
./build/src/test_app
echo

echo "=================================================="
echo

echo "3. Testing with a simple command (ls):"
echo
ls /etc/passwd
echo

echo "=================================================="
echo

echo "4. Testing with cat command:"
echo
echo "Hello, World!" > /tmp/fex_test.txt
cat /tmp/fex_test.txt
rm /tmp/fex_test.txt
echo

unset LD_PRELOAD
unset FEX_DEBUG

echo "Demo completed!"
echo
echo "To use FEX with any application:"
echo "  export LD_PRELOAD=/path/to/libfex.so"
echo "  export FEX_DEBUG=1  # Optional: enable debug output"
echo "  your_application"
echo
echo "Or in one command:"
echo "  LD_PRELOAD=./build/src/libfex.so FEX_DEBUG=1 your_application"