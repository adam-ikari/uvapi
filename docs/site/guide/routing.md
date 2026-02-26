# 路由指南

本文档详细介绍 UVAPI 的路由系统，包括路由定义、路径参数、路由分组等。

## 基础路由

### 简洁方法

UVAPI 提供简洁的 HTTP 方法快捷方式：

```cpp
#include "framework.h"

using namespace uvapi;
using namespace uvapi::restful;

int main() {
    uv_loop_t* loop = uv_default_loop();
    server::Server server(loop);
    
    // GET 请求
    server.get("/users", [](const HttpRequest& req) -> HttpResponse {
        return ResponseBuilder(200).success();
    });
    
    // POST 请求
    server.post("/users", [](const HttpRequest& req) -> HttpResponse {
        return ResponseBuilder(201).success("Created");
    });
    
    // PUT 请求
    server.put("/users/:id", [](const HttpRequest& req) -> HttpResponse {
        return ResponseBuilder(200).success("Updated");
    });
    
    // DELETE 请求
    server.del("/users/:id", [](const HttpRequest& req) -> HttpResponse {
        return ResponseBuilder(200).success("Deleted");
    });
    
    // PATCH 请求
    server.patch("/users/:id", [](const HttpRequest& req) -> HttpResponse {
        return ResponseBuilder(200).success("Patched");
    });
    
    // HEAD 请求
    server.head("/users", [](const HttpRequest& req) -> HttpResponse {
        return HttpResponse(200);
    });
    
    // OPTIONS 请求
    server.options("/users", [](const HttpRequest& req) -> HttpResponse {
        return HttpResponse(200)
            .header("Allow", "GET, POST, PUT, DELETE");
    });
    
    server.listen("0.0.0.0", 8080);
    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    return 0;
}
```

### 通用方法

使用 `addRoute` 方法注册任意 HTTP 方法：

```cpp
server.addRoute("/users", HttpMethod::GET, handler);
server.addRoute("/users", HttpMethod::POST, handler);
server.addRoute("/users/:id", HttpMethod::PUT, handler);
server.addRoute("/users/:id", HttpMethod::DELETE, handler);
```

## 路径参数

### 基本用法

使用 `:param` 语法定义路径参数：

```cpp
// 单个路径参数
server.get("/users/:id", [](const HttpRequest& req) -> HttpResponse {
    int64_t id = req.path<int64_t>("id");
    return ResponseBuilder(200).success().set("id", id);
});

// 多个路径参数
server.get("/users/:userId/posts/:postId", [](const HttpRequest& req) -> HttpResponse {
    int64_t user_id = req.path<int64_t>("userId");
    int64_t post_id = req.path<int64_t>("postId");
    
    return ResponseBuilder(200).success()
        .set("user_id", user_id)
        .set("post_id", post_id);
});
```

### 带默认值

```cpp
server.get("/items/:id", [](const HttpRequest& req) -> HttpResponse {
    // 如果参数不存在，返回默认值 0
    int64_t id = req.path<int64_t>("id", 0);
    return ResponseBuilder(200).success().set("id", id);
});
```

### 类型转换

支持自动类型转换：

```cpp
// 字符串（默认）
std::string name = req.path<std::string>("name");

// 整数
int num = req.path<int>("num");

// 64位整数
int64_t id = req.path<int64_t>("id");

// 浮点数
double price = req.path<double>("price");

// 布尔值
bool active = req.path<bool>("active");
```

## 查询参数

### 获取查询参数

```cpp
server.get("/search", [](const HttpRequest& req) -> HttpResponse {
    // 获取查询参数
    std::string q = req.query<std::string>("q");
    int page = req.query<int>("page", 1);
    int limit = req.query<int>("limit", 10);
    
    return ResponseBuilder(200).success()
        .set("query", q)
        .set("page", page)
        .set("limit", limit);
});
```

### 访问 URL

```
GET /search?q=uvapi&page=2&limit=20
```

响应：

```json
{
  "code": "0",
  "data": {
    "query": "uvapi",
    "page": 2,
    "limit": 20
  }
}
```

## 声明式路由

使用 `ApiBuilder` 和 `ApiDefinition` 进行声明式路由定义。

### 基本用法

```cpp
#include "declarative_dsl.h"

using namespace uvapi::declarative;

int main() {
    uv_loop_t* loop = uv_default_loop();
    server::Server server(loop);
    
    ApiBuilder api;
    
    // 定义 GET /users/:id
    api.get("/users/:id")
        .pathParam<int64_t>("id", Required<int64_t>())
        .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
            int64_t id = std::stoll(params.at("id"));
            return ResponseBuilder(200).success().set("id", id);
        });
    
    server.listen("0.0.0.0", 8080);
    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    return 0;
}
```

### 参数验证

声明式路由支持自动参数验证：

```cpp
api.get("/users")
    .param<int>("page", OptionalWithDefault<int>(1))
        .range(1, 1000000)         // 页码范围：1 ~ 1000000
    .param<int>("limit", OptionalWithDefault<int>(10))
        .range(1, 100)             // 每页最多 100 条
    .param<std::string>("status", OptionalWithDefault<std::string>(""))
        .oneOf({"active", "inactive", "pending"})  // 枚举值
    .param<std::string>("email", OptionalWithDefault<std::string>(""))
        .pattern("^[A-Za-z0-9+_.-]+@(.+)$")  // 正则验证
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        // 参数已验证通过
        return ResponseBuilder(200).success();
    });
```

### 验证失败响应

当参数验证失败时，框架自动返回 400 错误：

```json
{
  "code": 400,
  "message": "Value is too large",
  "field": "limit"
}
```

## RESTful API 示例

完整的 RESTful API 示例：

```cpp
#include "framework.h"
#include "declarative_dsl.h"

using namespace uvapi;
using namespace uvapi::restful;
using namespace uvapi::declarative;

// 用户数据模型
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

// 模拟数据库
std::map<int64_t, User> users = {
    {1, {1, "Alice", "alice@example.com"}},
    {2, {2, "Bob", "bob@example.com"}}
};

int main() {
    uv_loop_t* loop = uv_default_loop();
    server::Server server(loop);
    
    ApiBuilder api;
    
    // GET /users - 获取用户列表
    api.get("/users")
        .pagination(PageParam().page(1).limit(10))
        .search(SearchParam(""))
        .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
            int page = std::stoi(params.at("page"));
            int limit = std::stoi(params.at("limit"));
            std::string search = params.at("search");
            
            // 构建响应
            ResponseBuilder builder(200);
            builder.success().set("page", page).set("limit", limit);
            
            // 添加用户列表
            builder.add("users", JSON::Object()
                .set("id", 1)
                .set("name", "Alice")
                .set("email", "alice@example.com")
                .toJson());
            
            return builder;
        });
    
    // GET /users/:id - 获取单个用户
    api.get("/users/:id")
        .pathParam<int64_t>("id", Required<int64_t>())
        .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
            int64_t id = std::stoll(params.at("id"));
            
            auto it = users.find(id);
            if (it == users.end()) {
                return ResponseBuilder(404).error("User not found");
            }
            
            return ResponseBuilder(200).success().data(it->second);
        });
    
    // POST /users - 创建用户
    api.post("/users")
        .handle([](const HttpRequest& req) {
            // 解析请求体
            cJSON* json = cJSON_Parse(req.body.c_str());
            if (!json) {
                return ResponseBuilder(400).error("Invalid JSON");
            }
            
            cJSON* name_json = cJSON_GetObjectItem(json, "name");
            cJSON* email_json = cJSON_GetObjectItem(json, "email");
            
            if (!name_json || !email_json) {
                cJSON_Delete(json);
                return ResponseBuilder(400).error("Missing required fields");
            }
            
            // 创建用户
            int64_t new_id = users.size() + 1;
            User new_user = {new_id, name_json->valuestring, email_json->valuestring};
            users[new_id] = new_user;
            
            cJSON_Delete(json);
            
            return ResponseBuilder(201)
                .success("User created")
                .data(new_user);
        });
    
    // PUT /users/:id - 更新用户
    api.put("/users/:id")
        .pathParam<int64_t>("id", Required<int64_t>())
        .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
            int64_t id = std::stoll(params.at("id"));
            
            auto it = users.find(id);
            if (it == users.end()) {
                return ResponseBuilder(404).error("User not found");
            }
            
            // 解析请求体
            cJSON* json = cJSON_Parse(req.body.c_str());
            if (!json) {
                return ResponseBuilder(400).error("Invalid JSON");
            }
            
            cJSON* name_json = cJSON_GetObjectItem(json, "name");
            cJSON* email_json = cJSON_GetObjectItem(json, "email");
            
            if (name_json) it->second.name = name_json->valuestring;
            if (email_json) it->second.email = email_json->valuestring;
            
            cJSON_Delete(json);
            
            return ResponseBuilder(200).success("User updated").data(it->second);
        });
    
    // DELETE /users/:id - 删除用户
    api.del("/users/:id")
        .pathParam<int64_t>("id", Required<int64_t>())
        .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
            int64_t id = std::stoll(params.at("id"));
            
            auto it = users.find(id);
            if (it == users.end()) {
                return ResponseBuilder(404).error("User not found");
            }
            
            users.erase(it);
            
            return ResponseBuilder(200).success("User deleted");
        });
    
    server.listen("0.0.0.0", 8080);
    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    return 0;
}
```

## 路由模式

### 精确匹配

```cpp
server.get("/users", handler);        // 只匹配 /users
server.get("/users/profile", handler); // 只匹配 /users/profile
```

### 路径参数

```cpp
server.get("/users/:id", handler);              // 单个参数
server.get("/users/:userId/posts/:postId", handler); // 多个参数
```

### HTTP 方法区分

相同路径，不同方法：

```cpp
server.get("/users/:id", get_user);    // 获取用户
server.put("/users/:id", update_user); // 更新用户
server.del("/users/:id", delete_user); // 删除用户
```

## Next Steps

- [Request DSL Guide](./request-dsl.md) - Learn request parameter handling
- [Response DSL Guide](./response-dsl.md) - Learn response building
- [API Reference](../api/framework.md) - View complete API documentation
