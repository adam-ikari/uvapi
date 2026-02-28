# Request DSL

Request DSL 提供声明式的方式来定义请求参数和请求体，框架会自动解析和验证。

## 概述

Request DSL 包括：
- **参数声明**：声明查询参数、路径参数
- **请求体 Schema**：定义请求体的结构和验证规则
- **自动解析**：框架自动解析参数到类型安全的访问器
- **自动验证**：声明验证规则，框架自动执行

## 参数声明

### 基础用法

使用 `ApiBuilder` 和 `ApiDefinition` 声明参数：

```cpp
#include "declarative_dsl.h"

using namespace uvapi::declarative;

ApiBuilder api;

// GET /users?page=1&limit=10
api.get("/users")
    .param<int>("page", OptionalWithDefault<int>(1))
    .param<int>("limit", OptionalWithDefault<int>(10))
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        int page = std::stoi(params.at("page"));
        int limit = std::stoi(params.at("limit"));
        
        return ResponseBuilder(200).success()
            .set("page", page)
            .set("limit", limit);
    });
```

### 必需参数

```cpp
api.get("/users/:id")
    .pathParam<int64_t>("id", Required<int64_t>())
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        int64_t id = std::stoll(params.at("id"));
        return ResponseBuilder(200).success().set("id", id);
    });
```

### 可选参数

```cpp
api.get("/search")
    .param<std::string>("q", OptionalWithDefault<std::string>(""))
    .param<std::string>("category", OptionalWithDefault<std::string>("all"))
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        return ResponseBuilder(200).success()
            .set("query", params.at("q"))
            .set("category", params.at("category"));
    });
```

### 参数验证

```cpp
api.get("/users")
    .param<int>("page", OptionalWithDefault<int>(1))
        .range(1, 1000000)         // 页码范围
    .param<int>("limit", OptionalWithDefault<int>(10))
        .range(1, 100)             // 每页最多 100 条
    .param<std::string>("status", OptionalWithDefault<std::string>(""))
        .oneOf({"active", "inactive", "pending"})  // 枚举值
    .param<std::string>("email", OptionalWithDefault<std::string>(""))
        .pattern("^[A-Za-z0-9+_.-]+@(.+)$")  // 正则表达式
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        // 参数已验证通过
        return ResponseBuilder(200).success();
    });
```

### 验证规则

| 方法 | 说明 | 示例 |
|------|------|------|
| `range(min, max)` | 数值范围 | `.range(1, 100)` |
| `length(min, max)` | 字符串长度 | `.length(1, 100)` |
| `pattern(regex)` | 正则表达式 | `.pattern("^[a-z]+$")` |
| `oneOf(values)` | 枚举值 | `.oneOf({"a", "b", "c"})` |

## 便捷方法

### 分页参数

```cpp
api.get("/users")
    .pagination(PageParam().page(1).limit(20))
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        int page = std::stoi(params.at("page"));
        int limit = std::stoi(params.at("limit"));
        return ResponseBuilder(200).success();
    });
```

### 搜索参数

```cpp
api.get("/users")
    .search(SearchParam(""))
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        std::string search = params.at("search");
        return ResponseBuilder(200).success().set("search", search);
    });
```

### 排序参数

```cpp
api.get("/users")
    .sort(SortParam("created_at", "desc", {"id", "name", "created_at"}))
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        std::string sort = params.at("sort");
        std::string order = params.at("order");
        return ResponseBuilder(200).success();
    });
```

### 范围参数

```cpp
api.get("/products")
    .range("min_price", "max_price", RangeParam().min(0).max(1000000))
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        int min_price = std::stoi(params.at("min_price"));
        int max_price = std::stoi(params.at("max_price"));
        return ResponseBuilder(200).success();
    });
```

## 请求体 Schema

### 定义 Schema

使用 `DslBodySchema` 定义请求体结构：

```cpp
struct CreateUserRequest {
    std::string name;
    std::string email;
    int age;
    
    // 定义 Schema
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
                .required()
                .range(1, 150));
        }
    };
};
```

### 字段类型

| 方法 | 类型 | 说明 |
|------|------|------|
| `string(name, offset)` | std::string | 字符串 |
| `integer(name, offset)` | int | 整数 |
| `integer64(name, offset)` | int64_t | 64位整数 |
| `number(name, offset)` | double | 浮点数 |
| `floating(name, offset)` | float | 单精度浮点 |
| `boolean(name, offset)` | bool | 布尔值 |
| `object(name, offset)` | Object | 嵌套对象 |
| `array(name, offset)` | Array | 数组 |

### 字段验证

```cpp
field(string("name", offsetof(User, name))
    .required()                    // 必填
    .optional()                    // 可选
    .length(1, 100)                // 长度范围
    .pattern("^[a-zA-Z]+$")        // 正则表达式
    .oneOf({"admin", "user"})      // 枚举值
    .range(0, 100));               // 数值范围
```

### 使用 Schema

**重要：只推荐使用 `req.parseBody<T>()` 方式，框架会自动处理解析和验证。**

```cpp
#include "framework.h"

// 在 handler 中使用 req.parseBody<T>()
server.post("/users", [](const HttpRequest& req) -> HttpResponse {
    // 唯一推荐的方式：自动解析和验证请求体
    auto create_req = req.parseBody<CreateUserRequest>();
    
    if (!create_req.hasValue()) {
        return ResponseBuilder(400).error("Invalid request body");
    }
    
    // 使用解析后的数据
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
6. **零开销**：编译期优化，运行时无额外开销

## 参数访问

### 推荐方式：使用 operator[] 自动类型推导

```cpp
// 路径参数 - 自动类型推导
auto id = req.pathParam["id"];           // 自动推导为 int64_t
auto slug = req.pathParam["slug"];       // 自动推导为 string

// 查询参数 - 自动类型推导
auto page = req.queryParam["page"];      // 自动推导为 int
auto limit = req.queryParam["limit"];    // 自动推导为 int
auto status = req.queryParam["status"];  // 自动推导为 string
auto active = req.queryParam["active"];  // 自动推导为 bool
```

**优势**：
- 简洁：不需要重复指定类型
- 自动推导：根据变量声明自动选择正确的类型
- 类型安全：编译期类型检查
- 符合 DSL 哲学：描述"是什么"，而非"怎么做"

### 兼容方式：使用模板参数

```cpp
// 字符串 → 整数
int page = req.queryParam.get<int>("page", 1);

// 字符串 → 64位整数
int64_t id = req.pathParam.get<int64_t>("id");

// 字符串 → 浮点数
double price = req.queryParam.get<double>("price", 0.0);

// 字符串 → 布尔值
bool active = req.queryParam.get<bool>("active", false);
```

**使用场景**：
- 需要指定默认值时
- 需要明确的类型转换时

## 错误处理

### 验证失败自动响应

当参数验证失败时，框架自动返回 400 错误：

```json
{
  "code": 400,
  "message": "Parameter 'page' must be between 1 and 1000000",
  "field": "page"
}
```

### 自定义错误处理

```cpp
api.get("/users")
    .param<int>("page", OptionalWithDefault<int>(1))
        .range(1, 100)
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
        // 如果到达这里，说明验证通过
        return ResponseBuilder(200).success();
    });
```

## 完整示例

```cpp
#include "framework.h"
#include "declarative_dsl.h"

using namespace uvapi;
using namespace uvapi::restful;
using namespace uvapi::declarative;

// 请求体 Schema
struct CreateUserRequest {
    std::string name;
    std::string email;
    int age;
    
    class Schema : public DslBodySchema<CreateUserRequest> {
    public:
        void define() override {
            field(string("name", offsetof(CreateUserRequest, name))
                .required().length(1, 100));
            field(string("email", offsetof(CreateUserRequest, email))
                .required().pattern("^[A-Za-z0-9+_.-]+@(.+)$"));
            field(integer("age", offsetof(CreateUserRequest, age))
                .required().range(1, 150));
        }
    };
};

int main() {
    uv_loop_t* loop = uv_default_loop();
    server::Server server(loop);
    
    // 使用声明式 API
    ApiBuilder api;
    
    // GET /users/:id - 路径参数
    api.get("/users/:id")
        .pathParam<int64_t>("id", Required<int64_t>())
        .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
            int64_t id = std::stoll(params.at("id"));
            return ResponseBuilder(200).success().set("id", id);
        });
    
    // GET /users - 查询参数
    api.get("/users")
        .pagination(PageParam().page(1).limit(20))
        .search(SearchParam(""))
        .sort(SortParam("created_at", "desc"))
        .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) {
            return ResponseBuilder(200).success()
                .set("page", std::stoi(params.at("page")))
                .set("limit", std::stoi(params.at("limit")))
                .set("search", params.at("search"))
                .set("sort", params.at("sort"))
                .set("order", params.at("order"));
        });
    
    // POST /users - 请求体
    api.post("/users")
        .body<CreateUserRequest>(CreateUserRequest::Schema())
        .handle([](const HttpRequest& req) {
            return ResponseBuilder(201).success("User created");
        });
    
    server.listen("0.0.0.0", 8080);
    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    return 0;
}
```

## Next Steps

- [Response DSL Guide](./response-dsl.md) - Learn response building
- [JSON Usage Guide](./json-usage.md) - Learn JSON serialization
- [API Reference](../api/framework.md) - View complete API documentation
