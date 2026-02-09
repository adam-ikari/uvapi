#!/bin/bash
# UVAPI Setup Script - Initialize submodules

set -e

echo "Initializing UVAPI submodules..."

# Add UVHTTP as a submodule
echo "Adding UVHTTP submodule..."
git submodule add https://github.com/adam-ikari/uvhttp.git deps/uvhttp

# Initialize and update submodules
echo "Initializing submodules..."
git submodule update --init --recursive

echo "Building UVHTTP..."
cd deps/uvhttp
mkdir -p build
cd build
cmake .. -DUVHTTP_ALLOCATOR_TYPE=1
make -j$(nproc)
cd ../..

echo "Setup complete!"
echo "You can now build UVAPI:"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make"
