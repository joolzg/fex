#!/bin/bash

# FEX LD_PRELOAD Wrapper Script
# Usage: ./fex_run.sh [options] <command> [args...]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIBRARY_PATH="$SCRIPT_DIR/build/src/libfex.so"

# Default options
DEBUG=0
SHOW_STATUS=0
HELP=0

# Parse options
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            DEBUG=1
            shift
            ;;
        -s|--show-status)
            SHOW_STATUS=1
            shift
            ;;
        -h|--help)
            HELP=1
            shift
            ;;
        *)
            break
            ;;
    esac
done

# Show help
if [ $HELP -eq 1 ] || [ $# -eq 0 ]; then
    echo "FEX LD_PRELOAD Wrapper Script"
    echo
    echo "Usage: $0 [options] <command> [args...]"
    echo
    echo "Options:"
    echo "  -d, --debug        Enable debug output (FEX_DEBUG=1)"
    echo "  -s, --show-status  Show .fex files status on exit (FEX_SHOW_STATUS=1)"
    echo "  -h, --help         Show this help message"
    echo
    echo "Examples:"
    echo "  $0 ls /etc"
    echo "  $0 --debug cat /etc/hostname"
    echo "  $0 --show-status ./your_application arg1 arg2"
    echo
    echo "The library will intercept and log all file I/O operations."
    exit 0
fi

# Check if library exists
if [ ! -f "$LIBRARY_PATH" ]; then
    echo "Error: Library not found at $LIBRARY_PATH"
    echo "Please build the project first:"
    echo "  make build"
    exit 1
fi

# Set up environment
export LD_PRELOAD="$LIBRARY_PATH"
if [ $DEBUG -eq 1 ]; then
    export FEX_DEBUG=1
    echo "[FEX Wrapper] Debug mode enabled"
fi
if [ $SHOW_STATUS -eq 1 ]; then
    export FEX_SHOW_STATUS=1
    echo "[FEX Wrapper] Status display on exit enabled"
fi
if [ $DEBUG -eq 1 ]; then
    echo "[FEX Wrapper] Library: $LIBRARY_PATH"
    echo "[FEX Wrapper] Command: $*"
    echo
fi

# Execute the command
exec "$@"