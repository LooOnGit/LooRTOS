#!/bin/bash

echo "[1/2] Configuring CMake..."
cmake -G "MinGW Makefiles" -B build
if [ $? -ne 0 ]; then
    echo "[ERROR] CMake configuration failed!"
    exit 1
fi

# Detect number of processors for parallel build (fallback to 1 if nproc is not available)
NPROC=$(nproc 2>/dev/null || echo 1)

echo "[2/2] Building project..."
cmake --build build -j$NPROC "$@"
if [ $? -ne 0 ]; then
    echo "[ERROR] Build failed!"
    exit 1
fi

echo "[SUCCESS] Build completed successfully!"
