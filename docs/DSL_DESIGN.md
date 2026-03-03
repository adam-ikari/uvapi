# DSL 设计哲学

## 声明式原则

声明式 DSL 的核心在于**"描述是什么"**，而不是**"怎么做"**。

关键判断标准：**是否有动作？**

- **命令式**：包含动作，描述"怎么做"
  ```cpp
  param.setValue(1);      // 动作：设置值
  param.setRange(1, 100); // 动作：设置范围
  param.validate();       // 动作：执行验证
  ```

- **声明式**：只描述，没有动作
  ```cpp
  ParamGroup params = {
      range(Int("page", false, 1), 1, 1000),
      range(Int("limit", false, 10), 1, 100)
  };
  ```

## 设计哲学

1. **声明式**：参数声明在前，处理函数在后
2. **自动解析参数**：框架自动解析和类型转换
3. **自动校验**：框架自动验证参数
4. **单线程模型**：避免锁和线程同步，简化设计
5. **类型推导**：使用模板和宏实现编译期类型推导
6. **推荐方式优先**：提供清晰的最佳实践指南

## 声明式 vs 命令式

### 链式调用也可以是声明式

链式调用 ≠ 命令式

判断标准：
- 链式调用 + 无动作 = 声明式
- 链式调用 + 有动作 = 命令式

示例：

**声明式链式调用**：
```cpp
queryParam<int>("page")
    .optional()          // 描述：是可选的
    .defaultValue(1)     // 描述：默认值是1
    .range(1, 1000);     // 描述：范围是[1, 1000]
```

**命令式链式调用**：
```cpp
builder.addParam("page")    // 动作：添加参数
       .setOptional()        // 动作：设置为可选
       .setDefault(1);       // 动作：设置默认值
```

## 参数类型推导

### 编译期类型注册

使用模板参数在编译期注册参数类型：

```cpp
// 路由定义时注册类型
api.get("/users/:id")
    .pathParam<int>("id")        // 注册 id 为 int 类型
    .handle([](const HttpRequest& req) -> HttpResponse {
        // 自动类型推导
        auto id = req.pathParam["id"];
        
        // 检查类型转换是否成功
        if (id.hasError()) {
            return ResponseBuilder::badRequest()
                .json("{\"error\":\"" + id.errorMessage() + "\"}");
        }
        
        return ResponseBuilder::ok().data(user);
    });
```

### 自动类型推导

使用 `operator[]` 实现自动类型推导：

```cpp
// 定义时注册类型
api.get("/users/:id")
    .pathParam<int>("id")        // 注册 id 为 int
    .queryParam<int>("page")     // 注册 page 为 int
    .queryParam<std::string>("name")  // 注册 name 为 string
    .handle([](const HttpRequest& req) -> HttpResponse {
        // 使用 operator[] 自动推导类型
        auto id = req.pathParam["id"];           // 自动推导为 int
        auto page = req.queryParam["page"];      // 自动推导为 int
        auto name = req.queryParam["name"];      // 自动推导为 string
        
        // 检查类型转换是否成功
        if (id.hasError()) {
            return ResponseBuilder::badRequest()
                .json("{\"error\":\"" + id.errorMessage() + "\"}");
        }
        
        return ResponseBuilder::ok().data(users);
    });
```

**推荐用法**：
```cpp
// 1. 使用 operator[] 自动类型推导
auto id = req.pathParam["id"];  // 自动推导为 int

// 2. 检查转换是否成功
if (id.hasError()) {
    // 返回错误响应
    return ResponseBuilder::badRequest()
        .json("{\"error\":\"" + id.errorMessage() + "\"}");
}

// 3. 使用转换后的值
int user_id = id;  // 隐式转换为 int
```

### 单线程模型优势

- **零开销**：无需锁和线程同步
- **简单可靠**：避免竞态条件和死锁
- **性能优化**：直接访问类型注册表
- **易于测试**：无需处理多线程问题

## 推荐方式

### 参数访问

**推荐方式（operator[] 自动类型推导）**：
```cpp
auto id = req.pathParam["id"];      // 自动推导为 int
auto page = req.queryParam["page"];  // 自动推导为 int

// 检查类型转换是否成功
if (id.hasError()) {
    return ResponseBuilder::badRequest()
        .json("{\"error\":\"" + id.errorMessage() + "\"}");
}
```

**使用 optional 显式类型检查**：
```cpp
auto id = req.pathParam["id"].as<int>();
if (id.hasValue()) {
    int user_id = id.value();
    // 使用 user_id
}
```

### Request Body 解析

**唯一推荐方式**：
```cpp
User user = req.parseBody<User>();
```

**为什么只推荐一种方式**：
- 清晰简洁：一种方式，无歧义
- 自动验证：自动调用 Schema 验证
- 类型安全：编译期类型检查
- 统一风格：所有 API 使用相同模式

### Schema 定义

**推荐模式**：
```cpp
// 定义可复用的 Schema
auto userSchema = Schema<User>()
    .field("username", Required<std::string>()).length(3, 20)
    .field("email", Required<std::string>()).pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
    .field("age", OptionalWithDefault<int>(18)).range(18, 120);

// 在路由中使用
api.post("/api/users")
    .body(userSchema)
    .handle([](const HttpRequest& req) -> HttpResponse {
        User user = req.parseBody<User>();  // 自动验证
        return ResponseBuilder::created().data(user);
    });
```

## Response DSL 设计

Response DSL 遵循相同的声明式原则，用于构建 HTTP 响应。

### 声明式风格

**描述响应具有的属性**：

```cpp
ResponseBuilder::created()
    .status(201)                    // 描述：状态码是 201
    .message("User created")        // 描述：消息是 "User created"
    .requestId("12345")            // 描述：请求 ID 是 "12345"
    .data(user);                   // 描述：数据是 user
```

**读法**：
- `status(201)` → "状态码是 201"
- `message("User created")` → "消息是 User created"
- `requestId("12345")` → "请求 ID 是 12345"
- `data(user)` → "数据是 user"

### 零全局变量

使用工厂函数返回局部对象，避免 static 全局变量：

```cpp
// 错误：使用 static 全局变量
static ResponseBuilder success_response = ResponseBuilder::ok()
    .cacheControl("no-cache");

// 正确：使用工厂函数
ResponseBuilder makeSuccessResponse() {
    return ResponseBuilder::ok()
        .cacheControl("no-cache");
}
```

### 隐式转换

支持隐式转换为 HttpResponse，无需显式调用 `.toHttpResponse()`：

```cpp
// 隐式转换（推荐）
HttpResponse resp = ResponseBuilder::ok().data(user);

// 显式转换（可选）
HttpResponse resp = ResponseBuilder::ok().data(user).toHttpResponse();
```

### 自动序列化

自动检测并调用对象的 `toJson()` 方法：

```cpp
struct User {
    std::string name;
    
    std::string toJson() const {
        return JSON::Object().set("name", name).toCompactString();
    }
};

// 自动序列化
HttpResponse resp = ResponseBuilder::ok().data(user);
```

### 设计优势

1. **职责分离**：ResponseBuilder 负责 DSL，Response 负责数据
2. **零冗余**：移除 40+ 个重复 API，代码减少 45%
3. **类型安全**：编译期类型检查，自动序列化
4. **错误处理**：自动捕获异常，返回错误响应
5. **性能优化**：链式调用，减少拷贝

## 优势

1. **清晰性**：一眼看出参数的属性
2. **可组合性**：可以自由组合描述
3. **无副作用**：描述不会产生副作用
4. **易于测试**：描述可以独立测试
5. **符合直觉**：「声明在前，处理在后」
6. **类型安全**：编译期类型检查，自动类型推导
7. **单线程高效**：零锁开销，简单可靠
8. **统一风格**：推荐方式优先，减少歧义
9. **性能优化**：模板元编程，编译期优化
10. **易于维护**：清晰的 API 设计，最佳实践指南