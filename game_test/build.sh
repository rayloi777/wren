#!/bin/bash

set -e

echo "=== Building Wren ==="
make -C ../projects/make config=release_64bit wren

echo "=== Building game_test ==="
make

echo "=== Build complete ==="
echo "Run: ./game_test"