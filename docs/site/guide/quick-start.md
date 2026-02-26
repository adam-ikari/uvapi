# 快速开始

本教程将帮助您快速上手 UVAPI 框架，从安装到创建第一个 API。

## 安装

### 前置要求

- C++11 或更高版本的编译器
- CMake 3.10 或更高版本
- Git

### 克隆仓库

```bash
git clone https://github.com/adam-ikari/uvapi.git
cd uvapi
```

### 构建项目

```bash
mkdir build && cd build
cmake ..
make
```

## 创建第一个 API

### 基础服务器

创建 `main.cpp`：

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

### 编译和运行

```bash
# 编译
g++ -std=c++11 main.cpp -o server -I../include -L../build -luvhttp -luv -llhttp -lcjson -lmbedtls -lmbedx509 -lmbedcrypto -lpthread -ldl

# 运行
./server
```

访问 http://localhost:8080 查看结果（端口可能有所不同）。

## 使用 Response DSL

### 定义数据模型

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

### 创建响应

```cpp
auto create_user_handler = [](const HttpRequest& req) -> HttpResponse {
    User user = {1, "Alice", "alice@example.com"};
    
    // 使用声明式 Response DSL
    return ResponseBuilder::created()
        .message("User created successfully")
        .requestId("12345")
        .data(user);
};
```

### 响应格式

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
- [API Reference](../api/framework.md) - View complete API documentation
- [Examples](../examples/) - View more examples