# Welcome to UVAPI

UVAPI is a high-performance, type-safe RESTful framework built on top of UVHTTP, providing modern C++11 abstractions for building HTTP/1.1 and WebSocket servers.

## Quick Start

### Installation

```bash
git clone https://github.com/adami-kari/uvapi.git
cd uvapi
mkdir build && cd build
cmake ..
make
```

### Hello World

```cpp
#include "framework.h"

using namespace uvapi;
using namespace uvapi::restful;

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

## Features

- **Type-Safe API**: Automatic serialization/deserialization with compile-time type checking
- **High Performance**: Zero-copy optimization, event-driven architecture
- **Zero Global Variables**: Test-friendly, supports multiple instances
- **Declarative DSL**: Describe response properties declaratively
- **Automatic Serialization**: Auto-detect and call toJson() methods

## Documentation

- [Quick Start](/guide/quick-start.md) - Get started in 5 minutes
- [Response DSL](/guide/response-dsl.md) - Learn declarative response building
- [API Reference](/api/) - Complete API documentation
- [Examples](/examples/) - Practical usage examples
- [Design](/design/) - Design philosophy and principles