# Building UVAPI

## Prerequisites

- CMake 3.10 or higher
- C++11 compatible compiler (GCC 4.8+, Clang 3.3+, MSVC 2015+)
- Git (for cloning submodules)

## Quick Start

### 1. Clone the repository

```bash
git clone https://github.com/adam-ikari/uvapi.git
cd uvapi
```

### 2. Initialize submodules

```bash
./setup.sh
```

Or manually:

```bash
git submodule add https://github.com/adam-ikari/uvhttp.git deps/uvhttp
git submodule update --init --recursive

# Build UVHTTP
cd deps/uvhttp
mkdir -p build && cd build
cmake .. -DUVHTTP_ALLOCATOR_TYPE=1
make -j$(nproc)
cd ../..
```

### 3. Build UVAPI

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Build Options

- `-DUVHTTP_ALLOCATOR_TYPE=0`: Use system allocator (default)
- `-DUVHTTP_ALLOCATOR_TYPE=1`: Use mimalloc allocator (recommended)
- `-DCMAKE_BUILD_TYPE=Debug`: Debug build with no optimization
- `-DCMAKE_BUILD_TYPE=Release`: Release build with -O2 optimization (default)

## Running Examples

After building, you can run the examples:

```bash
# Benchmark server
./benchmark_server

# Multi-server test
./test_multi_server

# Test server
./test_server
```

## Dependencies

All dependencies are managed as git submodules through UVHTTP:

- **UVHTTP**: Main HTTP server library
- **libuv**: Asynchronous I/O library
- **llhttp**: HTTP parser
- **mbedtls**: TLS/SSL support
- **cjson**: JSON library
- **mimalloc**: Memory allocator
- **xxhash**: Fast hash algorithm
- **uthash**: Hash table implementation

## Updating Dependencies

To update all submodules:

```bash
git submodule update --remote --merge
```

To update a specific submodule:

```bash
cd deps/uvhttp
git pull origin main
cd ../..
git add deps/uvhttp
git commit -m "Update UVHTTP to latest version"
```

## Troubleshooting

### Build Errors

If you encounter build errors:

1. Ensure all submodules are initialized:
   ```bash
   git submodule update --init --recursive
   ```

2. Ensure UVHTTP is built:
   ```bash
   cd deps/uvhttp/build
   make -j$(nproc)
   ```

3. Check CMake version:
   ```bash
   cmake --version  # Should be 3.10 or higher
   ```

### Linker Errors

If you encounter linker errors:

1. Ensure UVHTTP libraries are built:
   ```bash
   ls deps/uvhttp/build/dist/lib/
   ls deps/uvhttp/dist/lib/
   ```

2. Check that all dependency libraries are available in their build directories.

## Cross-Platform Building

### Linux/macOS

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Windows (Visual Studio)

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

### Windows (MinGW)

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j$(nproc)
```
