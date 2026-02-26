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

```cpp
#include "framework.h"

server.post("/users", [](const HttpRequest& req) -> HttpResponse {
    // 解析请求体
    CreateUserRequest create_req;
    CreateUserRequest::Schema schema;
    
    if (!schema.fromJson(req.body, &create_req)) {
        return ResponseBuilder(400).error("Invalid request body");
    }
    
    // 验证
    cJSON* json = cJSON_Parse(req.body.c_str());
    std::string validation_error = schema.validate(json);
    cJSON_Delete(json);
    
    if (!validation_error.empty()) {
        return ResponseBuilder(400).error(validation_error);
    }
    
    // 使用数据
    return ResponseBuilder(201)
        .success("User created")
        .set("name", create_req.name)
        .set("email", create_req.email);
});
```

### 自动解析

结合声明式 API 自动解析请求体：

```cpp
api.post("/users")
    .body<CreateUserRequest>(CreateUserRequest::Schema())
    .handle([](const HttpRequest& req) {
        // 请求体已自动解析和验证
        return ResponseBuilder(201).success("User created");
    });
```

## 类型转换

Request DSL 支持自动类型转换：

```cpp
// 字符串 → 整数
int page = req.query<int>("page", 1);

// 字符串 → 64位整数
int64_t id = req.path<int64_t>("id");

// 字符串 → 浮点数
double price = req.query<double>("price", 0.0);

// 字符串 → 布尔值
bool active = req.query<bool>("active", false);
```

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
