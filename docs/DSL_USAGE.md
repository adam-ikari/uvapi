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