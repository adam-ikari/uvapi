# UVAPI - C++ RESTful Low-Code Framework

A high-performance, type-safe RESTful framework built on top of UVHTTP, providing modern C++11 abstractions for building HTTP/1.1 and WebSocket servers.

## Features

- **Type-Safe API**: Automatic serialization/deserialization with compile-time type checking
- **High Performance**: Zero-copy optimization, event-driven architecture, efficient memory management
- **Modern C++11**: Template metaprogramming, RAII, perfect forwarding
- **Easy to Use**: Simple, intuitive API for common HTTP operations
- **Multi-Server Support**: Multiple servers can share the same libuv event loop
- **Zero Global Variables**: Test-friendly, supports multiple instances

## Quick Start

```cpp
#include "framework.h"
using namespace uvapi;
using namespace restful;

int main() {
    uv_loop_t* loop = uv_default_loop();
    server::Server server(loop);
    
    server.addRoute("/", HttpMethod::GET, [](const HttpRequest& req) -> HttpResponse {
        HttpResponse resp(200);
        resp.body = "Hello, World!";
        return resp;
    });
    
    server.listen("0.0.0.0", 8080);
    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    return 0;
}
```

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Dependencies

UVAPI requires:
- UVHTTP (as a git submodule)
- libuv (as a git submodule)
- llhttp (as a git submodule)
- mbedtls (as a git submodule)
- cjson (as a git submodule)
- mimalloc (as a git submodule)
- xxhash (as a git submodule)
- uthash (as a git submodule)

## License

MIT License
