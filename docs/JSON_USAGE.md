# JSON 库使用指南

UVAPI 框架基于 cJSON 库提供了一个简洁、易用的 JSON 构建和解析接口。

## 特性

- **类型安全**：支持 C++11 基本类型
- **链式调用**：流式 API，易于构建复杂 JSON
- **自动内存管理**：使用 RAII 和智能指针，无需手动释放
- **紧凑输出**：支持格式化和紧凑两种输出格式

## JSON::Object - 对象构建器

### 基本使用

```cpp
using namespace uvapi;
using namespace restful;

// 构造基本对象
std::string json = JSON::Object()
    .set("name", "John Doe")
    .set("age", 30)
    .set("active", true)
    .setNull("deleted")
    .toString();
```

输出：
```json
{
  "name": "John Doe",
  "age": 30,
  "active": true,
  "deleted": null
}
```

### 支持的类型

```cpp
.set("key", "string")          // std::string
.set("key", "cstring")         // const char*
.set("key", 42)                // int
.set("key", 123456789LL)       // int64_t
.set("key", 3.14)              // double
.set("key", true)              // bool
.setNull("key")                // null
```

### 嵌套对象

```cpp
std::string json = JSON::Object()
    .set("code", 200)
    .set("message", "Success")
    .set("data", JSON::Object()
        .set("id", 123)
        .set("name", "John Doe")
        .toString())
    .toString();
```

## JSON::Array - 数组构建器

### 基本使用

```cpp
std::string json = JSON::Array()
    .append("Apple")
    .append("Banana")
    .append(42)
    .append(true)
    .append(3.14)
    .toString();
```

输出：
```json
["Apple", "Banana", 42, true, 3.14]
```

### 对象数组

```cpp
std::string json = JSON::Array()
    .append(JSON::Object()
        .set("id", 1)
        .set("name", "Alice")
        .toString())
    .append(JSON::Object()
        .set("id", 2)
        .set("name", "Bob")
        .toString())
    .toString();
```

### 嵌套数组

```cpp
std::string json = JSON::Object()
    .set("tags", JSON::Array()
        .append("user")
        .append("admin")
        .toString())
    .toString();
```

## 快速构造响应

### 成功响应

```cpp
std::string response = JSON::success("操作成功");
```

输出：
```json
{
  "code": "0",
  "message": "操作成功"
}
```

### 错误响应

```cpp
std::string response = JSON::error("参数错误");
```

输出：
```json
{
  "code": "-1",
  "message": "参数错误"
}
```

### 数据响应

```cpp
std::string response = JSON::data(R"({"total":100,"items":[]})");
```

输出：
```json
{
  "code": "0",
  "message": "Success",
  "data": {"total":100,"items":[]}
}
```

## 在 HTTP 响应中使用

```cpp
server.addRoute("/api/users/:id", HttpMethod::GET, 
    [](const HttpRequest& req) -> HttpResponse {
        (void)req;
        
        std::string body = JSON::Object()
            .set("code", 200)
            .set("message", "Success")
            .set("data", JSON::Object()
                .set("id", 123)
                .set("username", "johndoe")
                .set("email", "john@example.com")
                .toString())
            .toString();
        
        return HttpResponse(200).json(body);
    });
```

## 在声明式 DSL 中使用

```cpp
api.get("/api/users")
    .param("page", OptionalWithDefault<int>(1))
    .param("limit", OptionalWithDefault<int>(20))
    .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) -> HttpResponse {
        (void)req;
        
        std::string body = JSON::Object()
            .set("code", 200)
            .set("message", "Success")
            .set("data", JSON::Object()
                .set("page", std::stoi(params.at("page")))
                .set("limit", std::stoi(params.at("limit")))
                .set("total", 100)
                .set("users", JSON::Array()
                    .append(JSON::Object()
                        .set("id", 1)
                        .set("name", "Alice")
                        .toString())
                    .append(JSON::Object()
                        .set("id", 2)
                        .set("name", "Bob")
                        .toString())
                    .toString())
                .toString())
            .toString();
        
        return HttpResponse(200).json(body);
    });
```

## 紧凑格式

用于网络传输，减少数据量：

```cpp
std::string compact = JSON::Object()
    .set("code", 200)
    .set("message", "Success")
    .toCompactString();
```

输出：
```json
{"code":200,"message":"Success"}
```

## 注意事项

1. **内存管理**：JSON 类使用智能指针自动管理内存，无需手动调用 cJSON_Delete
2. **类型安全**：确保使用正确的类型，避免隐式转换
3. **空值检查**：始终检查 isValid() 方法，特别是在嵌套对象中
4. **字符串转义**：JSON 库会自动处理字符串转义

## 示例程序

完整示例：`examples/json_usage_example.cpp`

编译和运行：
```bash
cd build
cmake ..
make json_usage_example
./json_usage_example
```

## 技术细节

- **底层库**：cJSON (轻量级 C JSON 库)
- **内存管理**：std::shared_ptr + 自定义删除器
- **线程安全**：不保证线程安全，每个线程应使用独立的 JSON 对象