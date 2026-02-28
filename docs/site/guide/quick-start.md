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
    
    server.get("/hello", [](const HttpRequest& req) -> HttpResponse {
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

访问 http://localhost:8080/hello 查看结果。

## Request 对象

`HttpRequest` 对象包含了请求的所有信息：

### 属性

```cpp
struct HttpRequest {
    HttpMethod method;              // HTTP 方法
    std::string url_path;           // URL 路径
    std::map<std::string, std::string> headers;      // 请求头
    std::map<std::string, std::string> query_params; // 查询参数
    std::map<std::string, std::string> path_params;  // 路径参数
    std::string body;               // 请求体
    int64_t user_id;                // 用户 ID（认证后设置）
};
```

### 获取路径参数

```cpp
server.get("/users/:id", [](const HttpRequest& req) -> HttpResponse {
    // 推荐方式：使用 operator[] 自动类型推导
    auto id = req.pathParam["id"];  // 自动推导为 int64_t
    
    // 兼容方式：使用模板参数
    optional<int64_t> user_id = req.pathParam.get<int64_t>("id");
    
    return ResponseBuilder(200)
        .success()
        .set("user_id", id);
});
```

### 获取查询参数

```cpp
server.get("/users", [](const HttpRequest& req) -> HttpResponse {
    // 推荐方式：使用 operator[] 自动类型推导
    auto page = req.queryParam["page"];       // 自动推导为 int
    auto limit = req.queryParam["limit"];     // 自动推导为 int
    auto status = req.queryParam["status"];   // 自动推导为 string
    
    // 兼容方式：使用模板参数
    optional<std::string> name = req.queryParam.get<std::string>("name");
    
    return ResponseBuilder(200)
        .success()
        .set("page", page)
        .set("limit", limit)
        .set("status", status);
});
```

### 获取请求头

```cpp
server.get("/profile", [](const HttpRequest& req) -> HttpResponse {
    std::string auth = req.getHeader("Authorization");
    
    return ResponseBuilder(200)
        .success()
        .set("auth", auth);
});
```

### 解析请求体

**重要：只推荐使用 `req.parseBody<T>()` 方式，框架会自动处理解析和验证。**

```cpp
struct CreateUserRequest {
    std::string name;
    std::string email;
    int age;
    
    class Schema : public uvapi::DslBodySchema<CreateUserRequest> {
    public:
        void define() override {
            field(string("name", offsetof(CreateUserRequest, name))
                .required()
                .length(1, 100));
            
            field(string("email", offsetof(CreateUserRequest, email))
                .required()
                .pattern("^[A-Za-z0-9+_.-]+@(.+)$"));
            
            field(integer("age", offsetof(CreateUserRequest, age))
                .optional()
                .range(1, 150));
        }
    };
};

server.post("/users", [](const HttpRequest& req) -> HttpResponse {
    // 唯一推荐的方式：自动解析和验证请求体
    auto create_req = req.parseBody<CreateUserRequest>();
    
    if (!create_req.hasValue()) {
        return ResponseBuilder(400).error("Invalid request body");
    }
    
    return ResponseBuilder(201)
        .success("User created")
        .set("name", create_req.value().name)
        .set("email", create_req.value().email);
});
```

**为什么只推荐这种方式？**

1. **清晰简洁**：一种方式，无歧义
2. **自动验证**：自动调用 Schema 验证
3. **类型安全**：编译期类型检查
4. **统一风格**：所有 API 使用相同模式
5. **最佳实践**：避免多种方式带来的混乱

## 参数声明

使用声明式 DSL 声明参数，框架会自动解析和验证。

### 必需参数

```cpp
#include "declarative_dsl.h"

using namespace uvapi::declarative;

ApiBuilder api;

api.get("/users/:id")
    .pathParam<int64_t>("id", Required<int64_t>())
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        int64_t id = std::stoll(params.at("id"));
        return ResponseBuilder(200).success().set("id", id);
    });
```

### 可选参数（带默认值）

```cpp
api.get("/users")
    .param<int>("page", OptionalWithDefault<int>(1))
    .param<int>("limit", OptionalWithDefault<int>(10))
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        int page = std::stoi(params.at("page"));
        int limit = std::stoi(params.at("limit"));
        
        return ResponseBuilder(200)
            .success()
            .set("page", page)
            .set("limit", limit);
    });
```

### 参数验证

```cpp
api.get("/users")
    .param<int>("page", OptionalWithDefault<int>(1))
        .range(1, 1000000)         // 范围验证
    .param<int>("limit", OptionalWithDefault<int>(10))
        .range(1, 100)             // 限制最大 100
    .param<std::string>("status", OptionalWithDefault<std::string>(""))
        .oneOf({"active", "inactive", "pending"})  // 枚举值
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        // 参数已验证通过
        return ResponseBuilder(200).success();
    });
```

### 便捷方法

```cpp
// 分页参数
api.get("/users")
    .pagination(PageParam().page(1).limit(20))
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        return ResponseBuilder(200).success();
    });

// 搜索参数
api.get("/users")
    .search(SearchParam(""))
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        std::string search = params.at("search");
        return ResponseBuilder(200).success().set("search", search);
    });

// 排序参数
api.get("/users")
    .sort(SortParam("created_at", "desc", {"id", "name", "created_at"}))
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        std::string sort = params.at("sort");
        std::string order = params.at("order");
        return ResponseBuilder(200).success();
    });
```

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

## HTTP 方法

UVAPI 支持所有标准 HTTP 方法：

```cpp
server.get("/users", list_users);       // 查询列表
server.post("/users", create_user);     // 创建资源
server.put("/users/:id", update_user);  // 更新资源
server.del("/users/:id", delete_user);  // 删除资源
server.patch("/users/:id", patch_user); // 部分更新
server.head("/users", head_users);      // 只返回头
server.options("/users", options_users); // CORS 预检
```

## Next Steps

- [Response DSL Guide](./response-dsl.md) - Learn how to use Response DSL
- [JSON Usage Guide](./json-usage.md) - Learn JSON serialization
- [API Reference](../api/framework.md) - View complete API documentation
- [Examples](../examples/) - View more examples
