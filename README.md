# UVAPI - C++ RESTful Low-Code Framework

A high-performance, type-safe RESTful framework built on top of UVHTTP, providing modern C++11 abstractions for building HTTP/1.1 servers.

## Features

- **Type-Safe API**: Automatic serialization/deserialization with compile-time type checking
- **High Performance**: Zero-copy optimization, event-driven architecture, efficient memory management
- **Modern C++11**: Template metaprogramming, RAII, perfect forwarding
- **Zero Exceptions**: No exception-based error handling, uses return codes and error objects for better predictability
- **Easy to Use**: Simple, intuitive API for common HTTP operations
- **Multi-Server Support**: Multiple servers can share the same libuv event loop
- **Zero Global Variables**: Test-friendly, supports multiple instances

## Quick Start

### Basic Server

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

### Using Response DSL

```cpp
struct User {
    int64_t id;
    std::string name;
    std::string email;
    
    std::string toJson() const {
        return JSON::Object()
            .set("id", id)
            .set("name", name)
            .set("email", email)
            .toCompactString();
    }
};

auto create_user_handler = [](const HttpRequest& req) -> HttpResponse {
    User user = {1, "Alice", "alice@example.com"};
    
    // 使用声明式 Response DSL
    return ResponseBuilder::created()
        .message("User created successfully")
        .requestId("12345")
        .data(user);  // 自动序列化
};

// 或者使用工厂函数
HttpResponse resp = makeSuccessResponse().data(user);
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

## Documentation

Full documentation is available at: [https://uvapi.dev](https://uvapi.dev)

### Documentation Site

The documentation website provides:
- Quick start guide
- Response DSL tutorial
- API reference
- Examples and best practices
- Design philosophy
- Bilingual support (English and Chinese)

### Local Documentation

To run the documentation site locally:

```bash
cd docs/site
npm install
npm run docs:dev
```

Visit http://localhost:5174/ to view the documentation.

## License

MIT License
