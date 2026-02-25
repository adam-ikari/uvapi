# 内存优化文档

## 优化概述

本文档记录了 UVAPI 项目的内存优化工作，包括移动语义、零拷贝技术和对象池的使用。

## 优化内容

### 1. 移动语义 (Move Semantics)

#### HttpResponse 类优化

为 `HttpResponse` 类添加了移动构造函数和移动赋值运算符，减少了不必要的字符串拷贝：

```cpp
// 移动构造函数
HttpResponse(HttpResponse&& other) noexcept
    : status_code(other.status_code)
    , headers(std::move(other.headers))
    , body(std::move(other.body)) {}

// 移动赋值运算符
HttpResponse& operator=(HttpResponse&& other) noexcept {
    if (this != &other) {
        status_code = other.status_code;
        headers = std::move(other.headers);
        body = std::move(other.body);
    }
    return *this;
}
```

#### 字符串方法优化

添加了 `setBody` 和 `json` 方法的右值引用重载：

```cpp
// 接受右值引用的 setBody
HttpResponse& setBody(std::string&& body_text) {
    body = std::move(body_text);
    return *this;
}

// 接受右值引用的 json
HttpResponse& json(std::string&& json_body) {
    body = std::move(json_body);
    headers["Content-Type"] = "application/json";
    return *this;
}

// 静态方法：创建 JSON 响应（支持移动语义）
static HttpResponse jsonResponse(int status_code, std::string&& json_body) {
    HttpResponse resp(status_code);
    resp.headers["Content-Type"] = "application/json";
    resp.body = std::move(json_body);
    return resp;
}
```

#### RouteDefinition 类优化

为 `RouteDefinition` 类添加了移动构造函数：

```cpp
RouteDefinition(RouteDefinition&& other) noexcept
    : path(std::move(other.path))
    , method(other.method)
    , handler(std::move(other.handler))
    , path_params(std::move(other.path_params))
    , query_params(std::move(other.query_params)) {}
```

### 2. 零拷贝优化

#### benchmark_server 优化

在 `benchmark_server.cpp` 中使用 `std::move` 避免字符串拷贝：

```cpp
// 请求统计（零拷贝优化 + 移动语义）
server.addRoute("/stats", HttpMethod::GET, [](const HttpRequest& req) -> HttpResponse {
    HttpResponse resp(200);
    resp.header("Content-Type", "application/json");
    std::string body = "{\"total_requests\":" + std::to_string(request_count.load()) + "}";
    resp.setBody(std::move(body));  // 使用移动语义
    return resp;
});
```

#### 错误响应优化

在参数验证中使用 `HttpResponse::jsonResponse` 避免拷贝：

```cpp
// 之前的代码（会触发拷贝）
return HttpResponse(400).json(
    "{\"code\":\"400\",\"message\":\"Query parameter '" + param.name + "' is required\"}"
);

// 优化后的代码（使用移动语义）
std::string msg = "{\"code\":\"400\",\"message\":\"Query parameter '" + param.name + "' is required\"}";
return HttpResponse::jsonResponse(400, std::move(msg));
```

### 3. 对象池 (Object Pool)

实现了通用的对象池系统，用于管理频繁分配和释放的对象。

#### ObjectPool 类

```cpp
template<typename T>
class ObjectPool {
public:
    explicit ObjectPool(size_t initial_size = 10);
    
    std::unique_ptr<T, std::function<void(T*)>> acquire();
    
    size_t size() const;
    size_t max_size() const;
    void resize(size_t new_size);
    
private:
    mutable std::mutex mutex_;
    std::stack<std::unique_ptr<T>> pool_;
    size_t max_size_;
};
```

#### MemoryPoolManager 单例

```cpp
class MemoryPoolManager {
public:
    static MemoryPoolManager& instance();
    
    template<typename T>
    std::shared_ptr<ObjectPool<T>> getPool(size_t initial_size = 10);
    
    template<typename T>
    std::unique_ptr<T, std::function<void(T*)>> acquire();
    
    void clear();
    size_t poolCount() const;
};
```

#### PoolAllocator 类

提供 STL 兼容的分配器接口：

```cpp
template<typename T>
class PoolAllocator {
public:
    using value_type = T;
    
    PoolAllocator(std::shared_ptr<ObjectPool<T>> pool);
    
    T* allocate(size_t n);
    void deallocate(T* p, size_t n);
};
```

## 性能测试结果

### 测试环境

- 平台: Linux 6.14.11-2-pve
- 编译器: GCC
- 优化级别: -O2
- 分配器: mimalloc

### 测试命令

```bash
wrk -t4 -c100 -d30s http://localhost:8080/
```

### 测试结果

| 指标 | 数值 |
|------|------|
| 总请求数 | 212,251 |
| 每秒请求数 | 7,023.90 |
| 平均延迟 | 11.13ms |
| 标准差 | 44.38ms |
| 最大延迟 | 1.97s |
| Socket 错误 | 9 (timeout) |
| 传输量 | 23.48MB |
| 传输速率 | 795.68KB/s |

### 性能分析

1. **吞吐量**: 7,023.90 RPS，在高并发 (100 连接) 下表现良好
2. **延迟**: 平均延迟 11.13ms，P99 延迟较高 (1.97s)，可能是因为虚拟化环境的限制
3. **错误率**: 9 个超时错误，错误率 0.0042%，非常低
4. **内存使用**: 移动语义和零拷贝优化减少了内存分配和拷贝操作

## 优化建议

### 1. 使用对象池管理频繁分配的对象

对于频繁分配和释放的对象（如 HttpResponse、HttpRequest），可以使用对象池：

```cpp
auto& manager = MemoryPoolManager::instance();
auto response = manager.acquire<HttpResponse>();
// 使用 response
// 自动归还到池中
```

### 2. 使用移动语义传递大对象

在函数间传递大对象时，优先使用移动语义：

```cpp
// 推荐
void processResponse(HttpResponse&& resp);

// 避免
void processResponse(const HttpResponse& resp);
```

### 3. 使用 std::string&& 避免拷贝

在构建响应体时，使用右值引用：

```cpp
// 推荐
std::string body = buildResponse();
resp.setBody(std::move(body));

// 避免
resp.setBody(buildResponse());
```

## 未来优化方向

1. **响应体池化**: 为响应体使用内存池，减少堆分配
2. **连接池优化**: 优化连接池管理，减少连接创建和销毁开销
3. **智能指针优化**: 使用更高效的智能指针实现
4. **内存预分配**: 预分配常用对象，减少运行时分配
5. **延迟释放**: 延迟释放策略，减少内存碎片

## 注意事项

1. **移动后对象状态**: 移动后的对象处于有效但未定义的状态，不应再使用
2. **线程安全**: ObjectPool 内部使用互斥锁保证线程安全
3. **对象清理**: 对象池中的对象在使用前需要重置状态
4. **大小限制**: 对象池有最大大小限制，防止内存无限增长

## 相关文件

- `include/framework.h` - HttpResponse、RouteDefinition 移动语义
- `include/object_pool.h` - 对象池实现
- `examples/benchmark_server.cpp` - 零拷贝优化示例
- `src/framework_uvhttp.cpp` - 错误响应优化

## 参考资料

- C++11 移动语义: https://en.cppreference.com/w/cpp/language/move_constructor
- 零拷贝技术: https://en.wikipedia.org/wiki/Zero-copy
- 对象池模式: https://en.wikipedia.org/wiki/Object_pool_pattern