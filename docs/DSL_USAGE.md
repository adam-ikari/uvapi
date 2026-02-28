# DSL 使用方法

## 基本语法

声明式 DSL 使用初始化列表语法，整体式 API 声明。

```cpp
#include "declarative_dsl.h"

using namespace uvapi::declarative;

ApiBuilder api;

api.get("/api/users")
    .param("page", Required<int>())
    .param("limit", OptionalWithDefault<int>(10))
    .handle([](const HttpRequest& req) -> HttpResponse {
        return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
    });
```

## 参数定义

### 必需参数

```cpp
Required<int>()              // 必需整数
Required<std::string>()      // 必需字符串
Required<bool>()              // 必需布尔
Required<double>()            // 必需双精度浮点
```

### 可选参数（带默认值）

```cpp
OptionalWithDefault<int>(10)            // 可选整数，默认值 10
OptionalWithDefault<std::string>("active")  // 可选字符串，默认值 "active"
OptionalWithDefault<bool>(true)            // 可选布尔，默认值 true
OptionalWithDefault<double>(0.0)           // 可选双精度浮点，默认值 0.0
```

## 验证规则

### 范围验证

```cpp
ParamDef.range(min, max)
```

适用于整数和浮点数参数。

示例：
```cpp
Required<int>().range(1, 1000)
OptionalWithDefault<double>(0.0).range(0.0, 1000000.0)
```

### 长度验证

```cpp
ParamDef.length(min, max)
```

适用于字符串参数。

示例：
```cpp
Required<std::string>().length(3, 20)
```

### 正则验证

```cpp
ParamDef.pattern(regex)
```

适用于字符串参数。

示例：
```cpp
Required<std::string>().pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
```

### 枚举验证

```cpp
ParamDef.oneOf({value1, value2, ...})
```

适用于所有类型参数。

示例：
```cpp
OptionalWithDefault<std::string>("active").oneOf({"active", "inactive", "pending"})
OptionalWithDefault<int>(1).oneOf({1, 2, 3})
```

## 完整示例

### 用户列表 API

```cpp
api.get("/api/users")
    .param("page", Required<int>()).range(1, 1000)
    .param("limit", OptionalWithDefault<int>(10)).range(1, 100)
    .param("status", OptionalWithDefault<std::string>("active")).oneOf({"active", "inactive", "pending"})
    .param("search", OptionalWithDefault<std::string>(""))
    .handle([](const HttpRequest& req) -> HttpResponse {
        return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
    });
```

### 用户创建 API

```cpp
api.post("/api/users")
    .param("username", Required<std::string>()).length(3, 20)
    .param("email", Required<std::string>()).pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
    .param("age", OptionalWithDefault<int>(18)).range(18, 120)
    .param("active", OptionalWithDefault<bool>(true))
    .handle([](const HttpRequest& req) -> HttpResponse {
        return HttpResponse(201).json("{\"code\":201,\"message\":\"Created\"}");
    });
```

### 用户详情 API

```cpp
api.get("/api/users/:id")
    .pathParam("id", Required<int>()).range(1, INT_MAX)
    .handle([](const HttpRequest& req) -> HttpResponse {
        return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
    });
```

## 组合规则

1. **可选参数必须设置默认值**
   ```cpp
   OptionalWithDefault<int>(10)  // 可选，默认值 10
   ```

2. **必需参数不能设置默认值**
   ```cpp
   Required<int>()  // 必需，无默认值
   ```

3. **验证规则可以组合**
   ```cpp
   Required<std::string>().length(3, 20).pattern("^[a-z]+$")
   ```

4. **验证规则应用到最后一个参数**
   ```cpp
   .param("username", Required<std::string>())
   .param("email", Required<std::string>())
   .length(3, 20)  // 应用到 email
   ```

## 参数类型说明

| 类型 | Required | OptionalWithDefault |
|------|----------|---------------------|
| 整数 | `Required<int>()` | `OptionalWithDefault<int>(10)` |
| 字符串 | `Required<std::string>()` | `OptionalWithDefault<std::string>("active")` |
| 布尔 | `Required<bool>()` | `OptionalWithDefault<bool>(true)` |
| 双精度浮点 | `Required<double>()` | `OptionalWithDefault<double>(0.0)` |

## 验证规则说明

| 规则 | 方法 | 适用类型 |
|------|------|----------|
| 范围 | `.range(min, max)` | 整数、浮点数 |
| 长度 | `.length(min, max)` | 字符串 |
| 正则 | `.pattern(regex)` | 字符串 |
| 枚举 | `.oneOf({values})` | 所有类型 |

## Handler 参数访问（类型自动推导）

框架根据 DSL 中声明的参数类型，自动推导返回类型。所有参数访问返回 `optional<T>`。

### 查询参数

```cpp
api.get("/api/users")
    .param("page", Required<int>())
    .param("limit", OptionalWithDefault<int>(10))
    .param("status", OptionalWithDefault<std::string>("active"))
    .handle([](const HttpRequest& req) -> HttpResponse {
        // 类型自动推导，无需手动指定 <int>
        auto page = req.query("page");      // 返回 optional<int>
        auto limit = req.query("limit");    // 返回 optional<int>
        auto status = req.query("status");  // 返回 optional<string>
        
        // 使用 optional<T>
        int page_num = page.value_or(1);
        int limit_num = limit.value_or(10);
        std::string status_filter = status.value_or("active");
        
        return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
    });
```

### 路径参数

```cpp
api.get("/api/users/:id")
    .pathParam("id", Required<int>())
    .handle([](const HttpRequest& req) -> HttpResponse {
        // 类型自动推导
        auto id = req.path("id");  // 返回 optional<int>
        
        if (!id.hasValue()) {
            return HttpResponse(400).json("{\"code\":400,\"message\":\"Invalid user ID\"}");
        }
        
        int user_id = id.value();
        return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
    });
```

### 类型推导规则

| DSL 声明 | Handler 访问 | 返回类型 |
|----------|--------------|----------|
| `Required<int>()` | `req.query("page")` | `optional<int>` |
| `OptionalWithDefault<int>(10)` | `req.query("limit")` | `optional<int>` |
| `Required<std::string>()` | `req.query("username")` | `optional<string>` |
| `OptionalWithDefault<std::string>("active")` | `req.query("status")` | `optional<string>` |
| `Required<bool>()` | `req.query("active")` | `optional<bool>` |
| `OptionalWithDefault<bool>(true)` | `req.query("enabled")` | `optional<bool>` |
| `Required<double>()` | `req.query("price")` | `optional<double>` |
| `OptionalWithDefault<double>(0.0)` | `req.query("rate")` | `optional<double>` |

### 默认值自动应用

框架会自动应用 DSL 中声明的默认值，`optional<T>` 在有默认值时一定有值。

```cpp
api.get("/api/users")
    .param("limit", OptionalWithDefault<int>(10))  // 默认值 10
    .handle([](const HttpRequest& req) -> HttpResponse {
        // 即使请求中不传 limit 参数，limit 也会有值
        auto limit = req.query("limit");  // 返回 optional<int>，值一定是 10
        int limit_num = limit.value();     // 安全使用，不需要 value_or
        return HttpResponse(200).json("{\"code\":200}");
    });
```

## Request Body Schema 验证

### 定义 Schema（可复用）

Schema 用于验证 Request Body，支持在多个 API 中复用（如创建和更新操作）。

```cpp
auto userSchema = Schema<User>()
    .field("username", Required<std::string>()).length(3, 20)
    .field("email", Required<std::string>()).pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
    .field("age", OptionalWithDefault<int>(18)).range(18, 120)
    .field("active", OptionalWithDefault<bool>(true));
```

### 在 API 中使用 Schema

```cpp
// 创建用户 API
api.post("/api/users")
    .body(userSchema)
    .handle([](const HttpRequest& req) -> HttpResponse {
        auto user = req.body<User>();  // 类型自动推导
        return HttpResponse(201).json(user.toJson());
    });

// 更新用户 API（复用相同 Schema）
api.put("/api/users/:id")
    .pathParam("id", Required<int>())
    .body(userSchema)  // 复用相同的 Schema
    .handle([](const HttpRequest& req) -> HttpResponse {
        auto id = req.pathParam.get<int>("id");
        auto user = req.body<User>();
        return HttpResponse(200).json(user.toJson());
    });
```

### Schema 字段定义

#### 必需字段

```cpp
.field("username", Required<std::string>())
.field("email", Required<std::string>())
```

#### 可选字段（带默认值）

```cpp
.field("age", OptionalWithDefault<int>(18))
.field("active", OptionalWithDefault<bool>(true))
```

### Schema 验证规则

#### 范围验证

```cpp
.field("age", OptionalWithDefault<int>(18)).range(18, 120)
```

#### 长度验证

```cpp
.field("username", Required<std::string>()).length(3, 20)
```

#### 正则验证

```cpp
.field("email", Required<std::string>()).pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
```

#### 枚举验证

```cpp
.field("status", OptionalWithDefault<std::string>("active")).oneOf({"active", "inactive", "pending"})
```

### Schema 核心特点

| 特点 | 说明 |
|------|------|
| 可复用 | 在多个 API 中共享（创建、更新等） |
| 类型自动推导 | `req.body<User>()` 自动返回正确的类型 |
| 校验规则 | 支持 `.length()`、`.pattern()`、`.range()`、`.oneOf()` |
| 与参数声明一致 | 使用相同的 `Required<T>` 和 `OptionalWithDefault<T>` |

### 完整示例：用户 CRUD API

```cpp
// 定义 Schema（可复用）
auto userSchema = Schema<User>()
    .field("username", Required<std::string>()).length(3, 20)
    .field("email", Required<std::string>()).pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
    .field("age", OptionalWithDefault<int>(18)).range(18, 120)
    .field("active", OptionalWithDefault<bool>(true));

// 创建用户
api.post("/api/users")
    .body(userSchema)
    .handle([](const HttpRequest& req) -> HttpResponse {
        auto user = req.body<User>();
        if (!user.hasValue()) {
            return HttpResponse(400).json("{\"code\":400,\"message\":\"Invalid request body\"}");
        }
        return HttpResponse(201).json(user.value().toJson());
    });

// 更新用户
api.put("/api/users/:id")
    .pathParam("id", Required<int>())
    .body(userSchema)  // 复用相同 Schema
    .handle([](const HttpRequest& req) -> HttpResponse {
        auto id = req.pathParam.get<int>("id");
        auto user = req.body<User>();
        return HttpResponse(200).json(user.value().toJson());
    });
```

### 示例请求

```bash
# 创建用户
curl -X POST http://localhost:8080/api/users \
     -H "Content-Type: application/json" \
     -d '{"username":"alice","email":"alice@example.com","age":25}'

# 更新用户
curl -X PUT http://localhost:8080/api/users/1 \
     -H "Content-Type: application/json" \
     -d '{"username":"alice","email":"alice@newdomain.com","age":26}'
```

## Response DSL

Response DSL 提供声明式的方式构建 HTTP 响应，支持在 handler 函数外部声明响应结构。

### 设计原则

1. **零全局变量** - 使用工厂函数返回局部对象，避免 static 全局变量
2. **声明式风格** - 描述响应属性（what），而非执行动作（how）
3. **类型安全** - 编译期类型检查，自动序列化
4. **错误处理** - 捕获 toJson() 异常，返回错误响应
5. **隐式转换** - 自动转换为 HttpResponse，无需显式调用 .toHttpResponse()

### 基本用法

```cpp
#include "framework.h"

using namespace uvapi;
using namespace uvapi::restful;

// 定义数据模型
struct User {
    int64_t id;
    std::string username;
    std::string email;
    
    std::string toJson() const {
        return JSON::Object()
            .set("id", id)
            .set("username", username)
            .set("email", email)
            .toCompactString();
    }
};

// Handler 中使用
auto create_user_handler = [](const HttpRequest& req) -> HttpResponse {
    User user = {1, "alice", "alice@example.com"};
    
    // 隐式转换（自动转换为 HttpResponse）
    return ResponseBuilder::created()
        .message("User created successfully")
        .requestId("12345")
        .data(user);
};
```

### 声明式方法

#### 快速构造方法

```cpp
ResponseBuilder::ok()                    // 200 成功
ResponseBuilder::created()              // 201 创建成功
ResponseBuilder::badRequest()           // 400 错误请求
ResponseBuilder::unauthorized()          // 401 未授权
ResponseBuilder::forbidden()             // 403 禁止访问
ResponseBuilder::notFound()             // 404 未找到
ResponseBuilder::internalServerError()  // 500 内部错误
```

#### 声明式方法（描述响应属性）

```cpp
.status(201)                          // 描述状态码是 201
.message("User created")               // 描述消息是 "User created"
.header("X-Custom", "value")          // 描述头部是 "X-Custom: value"
.contentType("application/json")     // 描述 Content-Type 是 "application/json"
.cacheControl("no-cache")             // 描述 Cache-Control 是 "no-cache"
.requestId("12345")                   // 描述请求 ID 是 "12345"
.traceId("abc-def-ghi")               // 描述追踪 ID 是 "abc-def-ghi"
```

#### 数据序列化

```cpp
.data(user)                            // 描述数据是 user（自动调用 toJson()）
.data(users)                           // 描述数据是 users 数组
.data("{\"key\":\"value\"}")           // 描述数据是 JSON 字符串
```

### 工厂函数

为了避免使用 static 全局变量，框架提供了工厂函数：

```cpp
makeSuccessResponse()    // 成功响应模板
makeCreatedResponse()    // 创建用户响应模板
makeErrorResponse()      // 错误响应模板
makeNotFoundResponse()  // 未找到响应模板
makeListResponse()      // 列表响应模板
```

使用示例：

```cpp
// 使用工厂函数
HttpResponse resp = makeSuccessResponse().data(user);

// 工厂函数已经配置了常用的头部（Cache-Control, Content-Type 等）
```

### 隐式转换

ResponseBuilder 支持隐式转换为 HttpResponse，无需显式调用 `.toHttpResponse()`：

```cpp
// 隐式转换（推荐）
HttpResponse resp = ResponseBuilder::ok().data(user);

// 显式转换（可选，提高可读性）
HttpResponse resp = ResponseBuilder::ok()
    .message("Success")
    .data(user)
    .toHttpResponse();
```

### 链式调用

ResponseBuilder 支持链式调用，所有方法都返回 `ResponseBuilder&`：

```cpp
HttpResponse resp = ResponseBuilder::created()
    .message("User created successfully")
    .requestId("12345")
    .cacheControl("no-cache")
    .data(user);  // 自动转换为 HttpResponse
```

### 自动序列化

ResponseBuilder 自动检测对象的 `toJson()` 方法并调用：

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

// 等价于手动调用
HttpResponse resp = ResponseBuilder::ok().data(user.toJson());
```

### 错误处理

ResponseBuilder 自动捕获 `toJson()` 可能的异常：

```cpp
struct BadUser {
    std::string toJson() const {
        return "";  // 返回空字符串（错误情况）
        // 或者抛出异常
    }
};

BadUser user;
HttpResponse resp = ResponseBuilder::ok().data(user);

// 自动捕获异常，返回错误响应
// resp.status_code == 500
// resp.body == "{\"code\":\"500\",\"message\":\"Serialization error\",\"data\":\"{}}"
```

### 完整示例

#### 示例 1: 创建用户

```cpp
auto create_user_handler = [](const HttpRequest& req) -> HttpResponse {
    User user = {1, "alice", "alice@example.com"};
    
    return ResponseBuilder::created()
        .message("User created successfully")
        .requestId("12345")
        .data(user);
};
```

响应：
```json
{
  "code": "201",
  "message": "User created successfully",
  "data": {
    "id": 1,
    "username": "alice",
    "email": "alice@example.com"
  }
}
```

#### 示例 2: 获取用户列表

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

响应：
```json
{
  "code": "200",
  "message": "List retrieved successfully",
  "data": [
    {
      "id": 1,
      "username": "Alice",
      "email": "alice@example.com"
    },
    {
      "id": 2,
      "username": "Bob",
      "email": "bob@example.com"
    }
  ]
}
```

#### 示例 3: 错误响应

```cpp
auto error_handler = [](const HttpRequest& req) -> HttpResponse {
    return makeErrorResponse()
        .data("{\"error\":\"Invalid input\"}");
};
```

响应：
```json
{
  "code": "400",
  "message": "Bad Request",
  "data": {
    "error": "Invalid input"
  }
}
```

### 设计哲学

#### 核心原则

UVAPI 遵循以下核心设计原则：

1. **声明式**：参数声明在前，处理函数在后
2. **自动解析**：框架自动解析和类型转换参数
3. **自动校验**：框架自动验证参数
4. **单线程模型**：避免锁和线程同步，简化设计
5. **类型推导**：使用模板和宏实现编译期类型推导
6. **推荐方式优先**：提供清晰的最佳实践指南

#### 声明式 vs 命令式

**命令式（描述"怎么做"）**：
```cpp
response.setCode(200);          // 设置状态码为 200
response.setMessage("Success"); // 设置消息为 Success
response.setData(user);         // 设置数据为 user
```

**声明式（描述"是什么"）**：
```cpp
.status(200)                    // 状态码是 200
.message("Success")             // 消息是 Success
.data(user)                     // 数据是 user
```

Response DSL 使用纯粹的声明式风格。

#### 零全局变量

**错误的方式（使用 static 全局变量）**：
```cpp
static ResponseBuilder success_response = ResponseBuilder::ok()
    .header("Cache-Control", "no-cache");
```

**正确的方式（使用工厂函数）**：
```cpp
ResponseBuilder makeSuccessResponse() {
    return ResponseBuilder::ok()
        .header("Cache-Control", "no-cache");
}
```

### 最佳实践

1. **使用工厂函数** - 避免使用 static 全局变量
2. **使用声明式风格** - 描述响应属性，而非执行动作
3. **利用隐式转换** - 减少代码，提高可读性
4. **实现 toJson() 方法** - 让数据模型支持自动序列化
5. **链式调用** - 提高代码可读性
6. **错误处理** - 在 toJson() 中处理异常，返回错误响应
7. **使用模板参数访问参数** - 使用 `req.pathParam.get<T>("id")` 获取参数
8. **使用自动类型推导宏** - 使用 `PATH_PARAM` 和 `QUERY_PARAM` 宏简化代码
9. **只使用 req.parseBody<T>()** - Request Body 解析只使用这一种方式
10. **定义可复用的 Schema** - 创建可复用的 Schema 定义，在多个 API 中共享

### 测试

运行 ResponseBuilder 测试：

```bash
cd build
./test_response_builder
```

所有测试应该通过。