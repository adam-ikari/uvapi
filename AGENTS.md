# UVAPI 项目上下文

## 项目概述

UVAPI 是一个基于 UVHTTP 构建的高性能、类型安全的 RESTful 低代码框架，采用现代 C++11 标准编写。该项目旨在提供简洁、直观的 API，让开发者能够快速构建 HTTP/1.1 服务器。

### 核心特性

- **类型安全**: 自动序列化/反序列化，编译时类型检查
- **零异常 (Zero Exceptions)**: 不使用异常机制，所有错误通过返回码和错误对象传递
- **声明式 DSL**: 采用流式链式 API (fluent chain API) 定义路由、参数、Schema
- **自动参数解析**: 框架自动从 URL 提取查询参数和路径参数，支持自动类型转换
- **自动校验**: 声明式的验证规则自动执行，支持内置格式验证（email, url, uuid, date, datetime, ipv4）
- **高性能优先**: 零拷贝优化、事件驱动架构、智能缓存、内存优化
- **零全局变量**: 支持多实例，测试友好
- **RAII 优先**: 自动资源管理，避免内存泄漏
- **现代 C++11**: 模板元编程、完美转发、移动语义

### 设计哲学

1. **零异常 (Zero Exceptions)**: 框架完全不使用异常机制
   - 所有错误通过返回码、状态码或错误对象传递
   - 字符串转数字使用 strtol/strtod + errno 检查，而非 std::stoi/stod
   - 确保可预测的行为和更好的性能

2. **声明式 (Declarative)**: DSL 采用声明式设计，而非命令式
   - 使用流式链式 API (fluent chain API) 定义路由、参数、Schema
   - Builder 模式提供清晰的声明式接口
   - 关注"做什么"而非"怎么做"

3. **自动解析参数 (Automatic Parameter Parsing)**: 框架自动解析参数
   - 自动从 URL 提取查询参数和路径参数
   - ParamAccessor 提供类型安全的参数访问
   - 自动类型转换 (string→int/double/bool)
   - 支持默认值设置

4. **自动校验 (Automatic Validation)**: 声明式的验证规则自动执行
   - 通过 DSL 声明验证规则 (required, min, max, pattern, enum)
   - 框架在请求处理时自动执行验证
   - 验证失败自动返回 400 Bad Request 及详细错误信息
   - 支持内置格式验证 (email, url, uuid, date, datetime, ipv4)

5. **零全局变量**: 支持多实例，测试友好
6. **RAII 优先**: 自动资源管理，避免内存泄漏
7. **类型安全**: 编译时类型检查
8. **高性能优先**: 零拷贝、事件驱动

## 技术栈

- **语言**: C++11
- **核心依赖**: UVHTTP（HTTP/1.1 和 WebSocket 服务器库）
- **间接依赖**（通过 UVHTTP）:
  - libuv（异步 I/O）
  - llhttp（HTTP 解析）
  - mbedtls（TLS/SSL 支持）
  - cjson（JSON 库）
  - mimalloc（内存分配器）
  - xxhash（快速哈希算法）
  - uthash（哈希表实现）
- **构建系统**: CMake 3.10+

## 项目结构

```
uvapi/
├── include/              # 公共头文件
│   ├── framework.h      # 核心框架定义（主头文件）
│   ├── framework_server.h # Server 层定义
│   ├── framework_http.h   # HTTP 类型定义
│   ├── framework_types.h  # 基础类型定义
│   ├── dsl.h              # DSL 定义
│   ├── declarative_dsl.h  # 声明式 DSL
│   ├── schema_dsl.h       # Schema DSL
│   ├── object_pool.h      # 对象池实现
│   ├── response_cache.h   # 响应缓存
│   ├── string_builder.h   # 字符串构建器
│   ├── middleware.h       # 中间件接口
│   ├── rate_limiter.h     # 限流器
│   ├── metrics.h          # 性能指标
│   ├── health_check.h     # 健康检查
│   ├── multipart.h        # 多部分表单处理
│   └── uvhttp_middleware_adapter.h # UVHTTP 中间件适配器
├── src/                  # 源代码实现
│   ├── framework_uvhttp.cpp # 框架实现（基于 UVHTTP）
│   └── multipart.cpp     # 多部分表单处理
├── examples/             # 示例程序
│   ├── complete_api_example.cpp      # 完整 API 示例
│   ├── benchmark_server.cpp          # 性能测试服务器
│   ├── auth_server.cpp               # 认证服务器示例
│   ├── crud_server.cpp               # CRUD 服务器示例
│   ├── dsl_example.cpp               # DSL 使用示例
│   ├── schema_example.cpp            # Schema 使用示例
│   ├── schema_dsl_example.cpp        # Schema DSL 示例
│   ├── declarative_dsl_example.cpp   # 声明式 DSL 示例
│   ├── upload_example.cpp            # 文件上传示例
│   ├── static_files_example.cpp      # 静态文件服务示例
│   ├── uvhttp_middleware_example.cpp # UVHTTP 中间件示例
│   ├── validation_example.cpp        # 验证示例
│   └── cache_usage_example.cpp       # 缓存使用示例
├── test/                 # 测试
│   └── unit/             # 单元测试
│       ├── test_object_pool.cpp          # 对象池测试
│       ├── test_declarative_dsl.cpp      # 声明式 DSL 测试
│       └── test_schema_dsl.cpp           # Schema DSL 测试
├── docs/                 # 文档
│   ├── DSL_DESIGN.md              # DSL 设计哲学
│   ├── DSL_FEATURES_ASSESSMENT.md # DSL 特性评估
│   ├── DSL_USAGE.md               # DSL 使用指南
│   ├── MEMORY_OPTIMIZATION.md     # 内存优化文档
│   └── PRODUCTION_READINESS.md    # 生产就绪性检查清单
├── public/               # 静态文件（用于测试）
│   ├── index.html
│   ├── css/
│   ├── js/
│   └── images/
├── config/               # 配置文件
│   ├── production-examples.yaml
│   └── prometheus.yml
├── deps/                 # 第三方依赖（子模块）
│   └── uvhttp/           # UVHTTP 库
├── build/                # 构建输出目录
├── CMakeLists.txt        # 主构建配置
├── setup.sh              # 初始化脚本
├── BUILD.md              # 构建文档
├── README.md             # 项目说明
├── VALIDATION_IMPLEMENTATION.md # 验证实现总结
└── AGENTS.md             # 本文件（用于 iFlow CLI 上下文）
```

## 构建和运行

### 初始化依赖

```bash
# 使用提供的初始化脚本
./setup.sh

# 或手动初始化
git submodule update --init --recursive

# 构建 UVHTTP
cd deps/uvhttp
mkdir -p build && cd build
cmake .. -DUVHTTP_ALLOCATOR_TYPE=1
make -j$(nproc)
cd ../..
```

### 构建项目

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目（默认选项）
cmake ..

# 编译
make -j$(nproc)
```

### 构建选项

```bash
# 选择内存分配器类型
cmake -DUVHTTP_ALLOCATOR_TYPE=0 ..  # 系统分配器（默认）
cmake -DUVHTTP_ALLOCATOR_TYPE=1 ..  # mimalloc 分配器（推荐）

# Debug 模式（禁用优化）
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release 模式（-O2 优化，默认）
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### 运行示例

```bash
# 编译示例程序
cd build
make

# 运行完整 API 示例
./complete_api_example

# 运行性能测试服务器
./benchmark_server

# 运行静态文件服务示例
./static_files_example
```

## 开发约定

### 代码风格

- **标准**: 使用 C++11 标准
- **缩进**: 4 个空格
- **大括号**: K&R 风格
- **编译警告**: 零警告原则，启用 `-Werror`（待完善）
- **命名约定**:
  - 类名: PascalCase（如 `HttpRequest`、`HttpResponse`）
  - 函数名: camelCase（如 `getUsers`、`setBody`）
  - 变量名: camelCase（如 `user_id`、`page_num`）
  - 常量: UPPER_CASE（如 `MAX_CONNECTIONS`）
  - 命名空间: lowercase（如 `uvapi`、`restful`、`server`）

### 核心设计原则

#### 1. 零异常 (Zero Exceptions)

框架完全不使用异常机制，所有错误通过返回码、状态码或错误对象传递：

```cpp
// 字符串转整数（使用 strtol + errno，避免 std::stoi 抛出异常）
template<typename T>
inline T safeStrToInt(const std::string& str, T default_value) {
    if (str.empty()) return default_value;
    errno = 0;
    char* endptr = nullptr;
    long long result = strtoll(str.c_str(), &endptr, 10);
    if (endptr == str.c_str() || *endptr != '\0') return default_value;
    if (errno == ERANGE) return default_value;
    return static_cast<T>(result);
}
```

#### 2. 声明式 DSL

DSL 采用声明式设计，关注"做什么"而非"怎么做"：

```cpp
// 声明式：描述参数属性，无动作
auto param = queryParam<int>("page")
    .optional()          // 描述：是可选的
    .defaultValue(1)     // 描述：默认值是1
    .range(1, 1000);     // 描述：范围是[1, 1000]

// 声明式 Schema 定义
REQUIRED_STRING(username, username)
    .length(3, 20)
    .pattern("^[a-zA-Z0-9]+$");

REQUIRED_EMAIL(email, email)
    .pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");

REQUIRED_STRING(status, status)
    .enumValues({"active", "inactive", "pending"});
```

#### 3. 自动参数解析

框架自动解析 URL 参数和路径参数，支持类型自动转换：

```cpp
// 查询参数（自动类型推导）
auto page = req.queryParam["page"];        // 自动推导为 int
auto limit = req.queryParam["limit"];      // 自动推导为 int
auto status = req.queryParam["status"];    // 自动推导为 string
auto search = req.queryParam["search"];    // 自动推导为 string

// 路径参数（自动类型推导）
auto id = req.pathParam["id"];             // 自动推导为 int
```

#### 4. 自动校验

声明式的验证规则自动执行，验证失败自动返回 400 错误：

```cpp
// Schema 定义
class UserSchema : public DslBodySchema<User> {
public:
    void define() override {
        // username: 3-20 字符，字母和数字
        string("username", FIELD_OFFSET(User, username))
            .required()
            .length(3, 20)
            .pattern("^[a-zA-Z0-9]+$");

        // email: 必须是有效邮箱格式
        email("email", FIELD_OFFSET(User, email))
            .required();

        // status: 必须是枚举值之一
        string("status", FIELD_OFFSET(User, status))
            .required()
            .enumValues({"active", "inactive", "pending"});
    }
};
```

#### 5. RAII 资源管理

使用智能指针自动管理资源，避免内存泄漏：

```cpp
// UVHTTP 资源的 RAII 包装器
struct UvhttpServerDeleter {
    void operator()(uvhttp_server_t* ptr) const {
        if (ptr) uvhttp_server_free(ptr);
    }
};
using UvhttpServerPtr = std::shared_ptr<uvhttp_server_t>;

// Server 类自动管理所有资源
class Server {
private:
    UvhttpServerPtr server_;
    UvhttpRouterPtr router_;
    UvhttpContextPtr uvhttp_ctx_;
    UvhttpConfigPtr config_;
    UvhttpTlsContextPtr tls_config_;
    
public:
    // 析构函数自动清理所有资源
    ~Server() {
        // 清理静态文件上下文
        if (static_ctx_) {
            uvhttp_static_free(static_ctx_);
            static_ctx_ = nullptr;
        }
        // 其他资源由 RAII 包装类自动管理
    }
};
```

#### 6. 移动语义和零拷贝

使用移动语义避免不必要的拷贝，提升性能：

```cpp
// HttpResponse 移动构造函数
HttpResponse(HttpResponse&& other) noexcept
    : status_code(other.status_code)
    , headers(std::move(other.headers))
    , body(std::move(other.body)) {}

// 零拷贝优化示例
HttpResponse resp(200);
std::string body = buildResponse();
resp.setBody(std::move(body));  // 使用移动语义，避免拷贝
```

### 响应处理标准模式

所有响应处理应遵循以下模式：

```cpp
HttpResponse handler(const HttpRequest& req) {
    // 1. 解析参数（框架自动完成）
    auto id = req.pathParam["id"];
    auto name = req.queryParam["name"];
    
    // 2. 验证参数（框架自动完成 Schema 验证）
    
    // 3. 处理业务逻辑
    // ...
    
    // 4. 构建响应
    return HttpResponse(200)
        .setHeader("Content-Type", "application/json")
        .setBody("{\"success\":true}");
}
```

### 内存管理

- **RAII 优先**: 使用智能指针（`std::shared_ptr`）自动管理资源
- **移动语义**: 优先使用移动语义传递大对象，避免拷贝
- **零拷贝**: 对于大对象（如响应体），使用右值引用避免拷贝
- **对象池**: 对于频繁分配和释放的对象，使用对象池（`ObjectPool`）
- **避免全局变量**: 支持多实例，测试友好

### 错误处理

- **Zero Exceptions**: 不使用异常机制
- **返回码**: 使用返回码、状态码或错误对象传递错误
- **ValidationResult**: 使用 `ValidationResult` 表示验证结果
- **HttpResponse**: 使用 `HttpResponse` 表示 HTTP 响应（包含状态码和错误信息）
- **详细错误信息**: 错误信息清晰明确，包含字段名和错误原因

## 性能优化

### 已实现的优化

1. **移动语义**: 为 `HttpResponse`、`RouteDefinition` 等类添加移动构造函数和移动赋值运算符
2. **零拷贝优化**: 在响应构建中使用 `std::move` 避免字符串拷贝
3. **对象池**: 实现通用对象池系统（`ObjectPool`、`MemoryPoolManager`、`PoolAllocator`）
4. **智能缓存**: 集成 UVHTTP 的 LRU 缓存和缓存预热机制
5. **零拷贝文件传输**: 支持大文件 sendfile 优化
6. **事件驱动**: 基于 libuv 的事件驱动架构

### 性能指标

- **测试环境**: Linux 6.14.11-2-pve, GCC, -O2 优化, mimalloc 分配器
- **测试命令**: `wrk -t4 -c100 -d30s http://localhost:8080/`
- **结果**:
  - 总请求数: 212,251
  - 每秒请求数: 7,023.90 RPS
  - 平均延迟: 11.13ms
  - 错误率: 0.0042% (9 个超时错误)

## 生产就绪性

### 已完成 ✅

- 代码风格统一
- 命名规范一致
- 模块化设计
- Zero Exceptions 原则
- 输入验证机制
- 事件驱动架构
- 零拷贝优化
- 类型安全
- 单元测试框架
- 文档完整（API、设计、使用）

### 待改进 ⚠️

- 零编译警告（需要检查和修复）
- 内存泄漏检测（需要添加 valgrind/ASan）
- 性能基准测试（需要建立基线）
- 压力测试（需要完善）
- 错误日志记录（需要加强）
- 监控指标（需要添加）
- 健康检查端点（需要实现）
- CI/CD 集成（需要完善）
- Docker 支持（需要添加）
- 生产配置示例（需要提供）

### 生产使用建议

#### 可以使用 ✅
- 简单的 RESTful API 服务
- 低到中等负载的应用（< 10,000 RPS）
- 开发和测试环境
- 概念验证（POC）项目

#### 慎重使用 ⚠️
- 高负载应用（> 10,000 RPS）
- 需要严格 SLA 的生产环境
- 金融级应用
- 医疗级应用

## 功能模块

### 1. DSL (Domain Specific Language)

框架提供声明式 DSL 用于定义 API、参数和 Schema：

- **DSL 基础**: `include/dsl.h` - 基础 DSL 定义
- **声明式 DSL**: `include/declarative_dsl.h` - 声明式 DSL 实现
- **Schema DSL**: `include/schema_dsl.h` - Schema 定义 DSL

### 2. Server 层

`server::Server` 类提供 HTTP 服务器功能：

```cpp
uv_loop_t* loop = uv_default_loop();
server::Server server(loop);

// 添加路由
server.addRoute("/api/users", HttpMethod::GET, getUsersHandler);
server.addRoute("/api/users/:id", HttpMethod::GET, getUserHandler);

// 启动服务器
server.listen("0.0.0.0", 8080);
uv_run(loop, UV_RUN_DEFAULT);
```

### 3. HTTP 类型

框架提供类型安全的 HTTP 请求和响应类型：

- `HttpRequest`: HTTP 请求（包含方法、路径、查询参数、路径参数、请求头、请求体）
- `HttpResponse`: HTTP 响应（包含状态码、响应头、响应体）
- `HttpMethod`: HTTP 方法枚举（GET、POST、PUT、DELETE、PATCH、HEAD、OPTIONS）

### 4. 参数解析

框架自动解析 URL 参数和路径参数：

```cpp
// 查询参数
auto page = req.queryParam.get<int>("page");
auto limit = req.queryParam.get<int>("limit");

// 路径参数
auto id = req.pathParam["id"];
```

### 5. Schema 和验证

框架提供声明式 Schema 定义和自动验证：

```cpp
class UserSchema : public DslBodySchema<User> {
public:
    void define() override {
        REQUIRED_STRING(username, username).length(3, 20);
        REQUIRED_EMAIL(email, email);
        REQUIRED_STRING(status, status).enumValues({"active", "inactive"});
    }
};
```

### 6. 中间件

框架提供中间件接口，支持请求预处理和响应后处理：

```cpp
class Middleware {
public:
    virtual HttpResponse process(const HttpRequest& req, 
                                  std::function<HttpResponse(const HttpRequest&)> next) = 0;
};
```

### 7. 静态文件服务

支持静态文件服务，集成 UVHTTP 的静态文件功能：

```cpp
server.enableStaticFiles("/path/to/files", "/static", true);  // 启用缓存
```

### 8. TLS/SSL 支持

支持 HTTPS 服务器：

```cpp
TlsConfig tls_config;
tls_config.enabled = true;
tls_config.cert_file = "/path/to/cert.pem";
tls_config.key_file = "/path/to/key.pem";

server.enableTls(tls_config);
server.listenHttps("0.0.0.0", 8443);
```

### 9. 对象池

提供通用对象池系统，用于管理频繁分配和释放的对象：

```cpp
auto& manager = MemoryPoolManager::instance();
auto pool = manager.getPool<HttpResponse>(100);
auto response = manager.acquire<HttpResponse>();
```

### 10. 响应缓存

提供响应缓存功能，减少重复计算：

```cpp
ResponseCache cache(1000, 3600);  // 最多 1000 个条目，1 小时过期

HttpResponse cachedResp;
if (cache.get("cache_key", cachedResp)) {
    return cachedResp;
}

// 计算响应
HttpResponse resp = computeResponse();
cache.set("cache_key", resp, 3600);
return resp;
```

## 重要文件说明

### 核心文件

- **include/framework.h**: 核心框架定义（主头文件，包含所有公共接口）
- **src/framework_uvhttp.cpp**: 框架实现（基于 UVHTTP）
- **include/dsl.h**: DSL 基础定义
- **include/declarative_dsl.h**: 声明式 DSL 实现
- **include/schema_dsl.h**: Schema 定义 DSL

### 示例文件

- **examples/complete_api_example.cpp**: 完整的 RESTful API 示例
- **examples/benchmark_server.cpp**: 性能测试服务器
- **examples/validation_example.cpp**: 验证功能示例

### 文档文件

- **docs/DSL_DESIGN.md**: DSL 设计哲学
- **docs/MEMORY_OPTIMIZATION.md**: 内存优化文档
- **docs/PRODUCTION_READINESS.md**: 生产就绪性检查清单
- **BUILD.md**: 构建文档
- **README.md**: 项目说明

### 配置文件

- **CMakeLists.txt**: 主构建配置
- **setup.sh**: 初始化脚本

## 重要提示

### UVHTTP 子模块

UVAPI 依赖 UVHTTP 作为子模块，需要先初始化子模块：

```bash
git submodule update --init --recursive
cd deps/uvhttp
mkdir -p build && cd build
cmake .. -DUVHTTP_ALLOCATOR_TYPE=1
make -j$(nproc)
cd ../..
```

### 内存分配器

框架默认使用 mimalloc 分配器，可通过编译宏选择：

```bash
cmake -DUVHTTP_ALLOCATOR_TYPE=0 ..  # 系统分配器
cmake -DUVHTTP_ALLOCATOR_TYPE=1 ..  # mimalloc 分配器（推荐）
```

### 零异常原则

框架完全不使用异常机制，所有错误通过返回码、状态码或错误对象传递。字符串转数字使用 `strtol/strtod + errno` 而非 `std::stoi/stod`。

### 声明式 DSL

DSL 采用声明式设计，关注"做什么"而非"怎么做"。链式调用不一定是命令式，判断标准是有无动作：

```cpp
// 声明式：描述参数属性，无动作
auto param = queryParam<int>("page").optional().defaultValue(1).range(1, 1000);

// 命令式：包含动作
builder.addParam("page").setOptional().setDefault(1);
```

### 测试

项目使用单元测试框架（基于 Google Test），测试文件位于 `test/unit/` 目录：

```bash
cd build
make
./uvapi_unit_tests
```

## 相关链接

- 项目主页: https://github.com/adam-ikari/uvapi
- UVHTTP: https://github.com/adam-ikari/uvhttp
- 许可证: MIT License

## 常见任务

### 添加新路由

```cpp
server.addRoute("/api/users", HttpMethod::GET, [](const HttpRequest& req) -> HttpResponse {
    auto page = req.queryParam["page"];  // 默认值由 DSL 声明
    auto limit = req.queryParam["limit"];  // 默认值由 DSL 声明
    // 处理逻辑...
    return HttpResponse(200)
        .setHeader("Content-Type", "application/json")
        .setBody("{\"users\":[]}");
});
```

### 添加新 Schema

```cpp
struct User {
    std::string username;
    std::string email;
    int age;
};

class UserSchema : public DslBodySchema<User> {
public:
    void define() override {
        REQUIRED_STRING(username, username).length(3, 20);
        REQUIRED_EMAIL(email, email);
        OPTIONAL_INT(age, age).range(18, 120);
    }
};
```

### 添加中间件

```cpp
class LoggingMiddleware : public Middleware {
public:
    HttpResponse process(const HttpRequest& req, 
                          std::function<HttpResponse(const HttpRequest&)> next) {
        std::cout << "Request: " << req.url_path << std::endl;
        return next(req);
    }
};

// 使用中间件
server.useMiddleware(std::make_shared<LoggingMiddleware>());
```

### 性能测试

```bash
# 启动测试服务器
./benchmark_server

# 使用 wrk 进行性能测试
wrk -t4 -c100 -d30s http://localhost:8080/

# 使用 ab 进行性能测试
ab -n 10000 -c 100 http://localhost:8080/
```

## 设计改进（2026-02）

### 单线程模型设计

采用单线程模型，避免锁和线程同步，简化设计：

- **零开销**：无需锁和线程同步
- **简单可靠**：避免竞态条件和死锁
- **易于测试**：无需处理多线程问题
- **性能优化**：直接访问类型注册表

实现方式：
```cpp
class ParamTypeRegistry {
    // 静态成员存储（无需 thread_local）
    static std::map<std::string, int> path_param_types_;
    static std::map<std::string, int> query_param_types_;
    
public:
    // 编译期注册参数类型
    template<typename T>
    static void registerPathParam(const std::string& name) {
        path_param_types_[name] = static_cast<int>(DataTypeCode<T>::value);
    }
};
```

### 参数类型推导

使用模板元编程和宏实现编译期类型推导：

**模板参数方式（推荐）**：
```cpp
int id = req.pathParam.get<int>("id");
int page = req.queryParam.get<int>("page");
```

**参数访问（推荐方式）**：
```cpp
int id = req.pathParam["id"];        // 自动推导为 int
int page = req.queryParam["page"];   // 自动推导为 int
std::string name = req.queryParam["name"];  // 自动推导为 string
```

### Request Body 解析简化

**唯一推荐方式**：
```cpp
User user = req.parseBody<User>();
```

**为什么只推荐一种方式**：
1. 清晰简洁：一种方式，无歧义
2. 自动验证：自动调用 Schema 验证
3. 类型安全：编译期类型检查
4. 统一风格：所有 API 使用相同模式
5. 最佳实践：避免多种方式带来的混乱

### 设计哲学更新

新增核心设计原则：

1. **声明式**：参数声明在前，处理函数在后
2. **自动解析**：框架自动解析和类型转换参数
3. **自动校验**：框架自动验证参数
4. **单线程模型**：避免锁和线程同步，简化设计
5. **类型推导**：使用模板和宏实现编译期类型推导
6. **推荐方式优先**：提供清晰的最佳实践指南，减少歧义

### 文档更新

更新了以下文档以反映最新的设计改进：

1. **docs/DSL_DESIGN.md** - 更新设计哲学，添加单线程模型、参数类型推导等内容
2. **docs/site/design/dsl-design.md** - 更新英文版本的设计哲学
3. **docs/site/design/index.md** - 更新设计原则
4. **docs/DSL_USAGE.md** - 更新最佳实践
5. **docs/site/guide/quick-start.md** - 更新快速开始指南，强调只使用 req.parseBody<T>()
6. **docs/site/guide/request-dsl.md** - 更新请求 DSL 指南，强调推荐方式

## 总结

UVAPI 是一个高性能、类型安全的 RESTful 低代码框架，采用现代 C++11 标准，基于 UVHTTP 构建。框架的核心设计原则包括零异常、声明式 DSL、自动参数解析、自动校验、RAII 资源管理、移动语义和零拷贝优化。框架已实现核心功能，包括 DSL、Server 层、HTTP 类型、参数解析、Schema 和验证、中间件、静态文件服务、TLS/SSL 支持、对象池和响应缓存等。框架的性能指标为 7,023.90 RPS（高并发 100 连接），适用于简单到中等负载的 RESTful API 服务。对于高负载、高可用、高安全要求的生产环境，还需要进一步完善测试、监控、文档等方面的内容。

最新的设计改进包括：
- 采用单线程模型，避免锁和线程同步
- 实现真正的自动类型推导：使用 `operator[]` 语法
  - `auto id = req.pathParam["id"]` - 无需指定类型，自动推导
  - `auto page = req.queryParam["page"]` - 无需指定类型，自动推导
- 简化 Request Body 解析，只推荐使用 `req.parseBody<T>()` 方法
- 更新设计哲学，强调推荐方式优先，减少歧义
- 更新所有相关文档，确保与实现保持一致

### 参数访问方式（2026-02-28）

**推荐方式：operator[]（真正的自动类型推导）**
```cpp
// 路径参数
auto id = req.pathParam["id"];        // 自动推导为 int64_t
auto slug = req.pathParam["slug"];    // 自动推导为 string

// 查询参数
auto page = req.queryParam["page"];   // 自动推导为 int
auto status = req.queryParam["status"];  // 自动推导为 string
auto active = req.queryParam["active"];  // 自动推导为 bool
```

**实现细节**：
- `ParamValue` 类使用模板转换运算符 `operator U()` 支持自动类型转换
- 转换时使用 `strtol/strtod + errno` 进行安全的类型转换
- 支持的类型：int, int64_t, double, float, bool, string
- 编译期类型检查，零运行时开销

**优势**：
- 真正的自动类型推导，无需重复指定类型
- 代码最简洁，减少样板代码
- 符合 DSL 设计哲学：描述"是什么"，而非"怎么做"