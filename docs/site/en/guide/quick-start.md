# Quick Start

This tutorial will help you quickly get started with the UVAPI framework, from installation to creating your first API.

## Installation

### Prerequisites

- C++11 or higher compiler
- CMake 3.10 or higher
- Git

### Clone Repository

```bash
git clone https://github.com/adam-ikari/uvapi.git
cd uvapi
```

### Build Project

```bash
mkdir build && cd build
cmake ..
make
```

## Create Your First API

### Basic Server

Create `main.cpp`:

```cpp
#include "framework.h"

using namespace uvapi;
using namespace uvapi::restful;

int main() {
    uv_loop_t* loop = uv_default_loop();
    server::Server server(loop);
    
    server.addRoute("/hello", HttpMethod::GET, [](const HttpRequest& req) -> HttpResponse {
        HttpResponse resp(200);
        resp.body = "Hello, UVAPI!";
        return resp;
    });
    
    server.listen("0.0.0.0", 8080);
    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    return 0;
}
```

### Compile and Run

```bash
# Compile
g++ -std=c++11 main.cpp -o server -I../include -L../build -luvhttp -luv -llhttp -lcjson -lmbedtls -lmbedx509 -lmbedcrypto -lpthread -ldl

# Run
./server
```

Visit http://localhost:8080 to see the result (port may vary).

## Using Response DSL

### Define Data Model

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
```

### Create Response

```cpp
auto create_user_handler = [](const HttpRequest& req) -> HttpResponse {
    User user = {1, "Alice", "alice@example.com"};
    
    // Use declarative Response DSL
    return ResponseBuilder::created()
        .message("User created successfully")
        .requestId("12345")
        .data(user);
};
```

### Response Format

```json
{
  "code": "201",
  "message": "User created successfully",
  "data": {
    "id": 1,
    "name": "Alice",
    "email": "alice@example.com"
  }
}
```

## Next Steps

- [Response DSL Guide](./response-dsl.md) - Learn how to use Response DSL
- [JSON Usage Guide](./json-usage.md) - Learn JSON serialization
- [API Reference](./api/framework.md) - View complete API documentation
- [Examples](../examples/) - View more examples