# Response DSL 指南

Response DSL 提供声明式的方式构建 HTTP 响应，支持在 handler 函数外部声明响应结构。

## 设计原则

1. **零全局变量** - 使用工厂函数返回局部对象
2. **声明式风格** - 描述响应属性，而非执行动作
3. **类型安全** - 编译期类型检查，自动序列化
4. **错误处理** - 捕获异常，返回错误响应
5. **隐式转换** - 自动转换为 HttpResponse

## 基础用法

### 快速构造方法

```cpp
ResponseBuilder::ok()                    // 200 成功
ResponseBuilder::created()              // 201 创建成功
ResponseBuilder::badRequest()           // 400 错误请求
ResponseBuilder::unauthorized()          // 401 未授权
ResponseBuilder::forbidden()             // 403 禁止访问
ResponseBuilder::notFound()             // 404 未找到
ResponseBuilder::internalServerError()  // 500 内部错误
```

### 声明式方法

```cpp
ResponseBuilder::created()
    .status(201)                          // 描述状态码是 201
    .message("User created")               // 描述消息是 "User created"
    .header("X-Custom", "value")          // 描述头部
    .contentType("application/json")     // 描述 Content-Type
    .cacheControl("no-cache")             // 描述 Cache-Control
    .requestId("12345")                   // 描述请求 ID
    .traceId("abc-def-ghi")               // 描述追踪 ID
```

### 数据序列化

```cpp
.data(user)                            // 描述数据是 user（自动序列化）
.data(users)                           // 描述数据是 users 数组
.data("{\"key\":\"value\"}")           // 描述数据是 JSON 字符串
```

## 工厂函数

为了避免使用 static 全局变量，框架提供了工厂函数：

```cpp
makeSuccessResponse()    // 成功响应模板
makeCreatedResponse()    // 创建用户响应模板
makeErrorResponse()      // 错误响应模板
makeNotFoundResponse()  // 未找到响应模板
makeListResponse()      // 列表响应模板
```

## 自动序列化

ResponseBuilder 自动检测对象的 `toJson()` 方法：

```cpp
struct User {
    int64_t id;
    std::string name;
    
    std::string toJson() const {
        return JSON::Object()
            .set("id", id)
            .set("name", name)
            .toCompactString();
    }
};

// 自动序列化
HttpResponse resp = ResponseBuilder::ok().data(user);
```

## 隐式转换

支持隐式转换为 HttpResponse，无需显式调用 `.toHttpResponse()`：

```cpp
// 隐式转换（推荐）
HttpResponse resp = ResponseBuilder::ok().data(user);

// 显式转换（可选）
HttpResponse resp = ResponseBuilder::ok()
    .message("Success")
    .data(user)
    .toHttpResponse();
```

## 完整示例

### 示例 1: 创建用户

```cpp
auto create_user_handler = [](const HttpRequest& req) -> HttpResponse {
    User user = {1, "Alice", "alice@example.com"};
    
    return ResponseBuilder::created()
        .message("User created successfully")
        .requestId("12345")
        .data(user);
};
```

### 示例 2: 获取用户列表

```cpp
auto list_users_handler = [](const HttpRequest& req) -> HttpResponse {
    std::vector<User> users = {
        {1, "Alice", "alice@example.com"},
        {2, "Bob", "bob@example.com"}
    };
    
    return makeListResponse()
        .data(users);
};
```

### 示例 3: 错误响应

```cpp
auto error_handler = [](const HttpRequest& req) -> HttpResponse {
    return makeErrorResponse()
        .data("{\"error\":\"Invalid input\"}");
};
```

## 错误处理

ResponseBuilder 自动捕获 `toJson()` 可能的异常：

```cpp
struct BadUser {
    std::string toJson() const {
        return "";  // 返回空字符串（错误情况）
    }
};

BadUser user;
HttpResponse resp = ResponseBuilder::ok().data(user);

// 自动捕获异常，返回错误响应
// resp.status_code == 500
// resp.body == "{\"code\":\"500\",\"message\":\"Serialization error\",\"data\":\"{}}"
```

## 最佳实践

1. **使用工厂函数** - 避免使用 static 全局变量
2. **使用声明式风格** - 描述响应属性，而非执行动作
3. **利用隐式转换** - 减少代码，提高可读性
4. **实现 toJson() 方法** - 让数据模型支持自动序列化
5. **链式调用** - 提高代码可读性
6. **错误处理** - 在 toJson() 中处理异常，返回错误响应

## 相关文档

- [JSON 使用指南](./json-usage.md) - 了解 JSON 序列化
- [API 参考](../api/response-dsl.md) - 查看 Response DSL API
- [示例代码](../examples/response-dsl.md) - 查看完整示例