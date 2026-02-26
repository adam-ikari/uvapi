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