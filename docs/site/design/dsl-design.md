# DSL Design

This document explains the design philosophy and principles behind the Response DSL in UVAPI.

## Overview

The Response DSL (Domain Specific Language) is designed to provide a declarative, type-safe, and intuitive way to build HTTP responses in C++.

## Design Philosophy

### Core Principles

UVAPI follows these core design principles:

1. **Declarative Style** - Describe "what", not "how"
2. **Automatic Parsing** - Framework automatically parses and converts parameters
3. **Automatic Validation** - Framework validates parameters automatically
4. **Single-Threaded Model** - Avoid locks and thread synchronization for simplicity
5. **Type Inference** - Use templates and macros for compile-time type deduction
6. **Recommended Way First** - Provide clear best practice guidelines

### 1. Declarative Style

**Principle:** Describe "what" the response should be, not "how" to build it.

**Imperative Style (Avoid):**
```cpp
HttpResponse resp(201);
resp.setHeader("Content-Type", "application/json");
resp.setHeader("X-Request-ID", "12345");
resp.body = buildJson(user);
```

**Declarative Style (Use):**
```cpp
ResponseBuilder::created()
    .contentType("application/json")
    .requestId("12345")
    .data(user);
```

**Benefits:**
- More readable and maintainable
- Self-documenting code
- Easier to understand at a glance
- Reduces cognitive load

### 2. Zero Global Variables

**Principle:** Avoid static global variables to support multi-instance and testing.

**Anti-Pattern (Avoid):**
```cpp
class ResponseBuilder {
    static ResponseBuilder* instance_;  // ❌ Global variable
};
```

**Pattern (Use):**
```cpp
// Factory functions returning local objects
ResponseBuilder makeSuccessResponse() {
    return ResponseBuilder().status(200);
}
```

**Benefits:**
- Thread-safe
- Supports multiple instances
- Unit test friendly
- No hidden state
- Better for cloud-native scenarios

### 3. Type Safety

**Principle:** Leverage C++ type system for compile-time error detection.

**Implementation:**
```cpp
template<typename T>
auto data(const T& instance) -> typename std::enable_if<
    std::is_same<decltype(std::declval<T>().toJson()), std::string>::value,
    ResponseBuilder&
>::type {
    // Automatic serialization
}
```

**Benefits:**
- Compile-time type checking
- Auto-completion support
- Refactoring safety
- Reduced runtime errors

### 4. Automatic Serialization

**Principle:** Automatically detect and call `toJson()` method on objects.

**Implementation:**
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

// Automatic serialization
auto response = ResponseBuilder::ok().data(user);
```

**Benefits:**
- Minimal boilerplate
- Consistent serialization
- Easy to extend
- Single responsibility principle

### 5. Error Handling

**Principle:** Automatic exception handling with meaningful error responses.

**Implementation:**
```cpp
template<typename T>
ResponseBuilder& data(const T& instance) {
    try {
        std::string json_str = instance.toJson();
        if (json_str.empty()) {
            status_code_ = 500;
            message_ = "Failed to serialize object";
            pending_data_ = "{}";
        } else {
            pending_data_ = json_str;
        }
    } catch (...) {
        status_code_ = 500;
        message_ = "Serialization error";
        pending_data_ = "{}";
    }
    return *this;
}
```

**Benefits:**
- Prevents crashes
- Meaningful error messages
- Graceful degradation
- Debugging support

### 6. Single-Threaded Model

**Principle:** Avoid locks and thread synchronization for simplicity and performance.

**Implementation:**
```cpp
class ParamTypeRegistry {
    // Static member storage (no thread_local needed)
    static std::map<std::string, int> path_param_types_;
    static std::map<std::string, int> query_param_types_;
    
public:
    // Register parameter type at compile time
    template<typename T>
    static void registerPathParam(const std::string& name) {
        path_param_types_[name] = static_cast<int>(DataTypeCode<T>::value);
    }
    
    // Get parameter type
    static int getPathParamType(const std::string& name) {
        auto it = path_param_types_.find(name);
        return it != path_param_types_.end() ? it->second : -1;
    }
};
```

**Benefits:**
- Zero overhead (no locks)
- Simple and reliable (no race conditions)
- Easy to test (no threading issues)
- Better performance (direct access)

### 7. Parameter Type Inference

**Principle:** Use template metaprogramming to deduce parameter types at compile time.

**Implementation:**
```cpp
// Register type when defining route
api.get("/users/:id")
    .pathParam<int>("id")        // Register id as int type
    .handle([](const HttpRequest& req) -> HttpResponse {
        // Compile-time type knowledge
        int id = req.pathParam.get<int>("id");
        return ResponseBuilder::ok().data(user);
    });
```

**Automatic Type Inference Macros:**
```cpp
// Register types when defining
api.get("/users/:id")
    .pathParam<int>("id")        // Register id as int
    .queryParam<int>("page")     // Register page as int
    .queryParam<std::string>("name")  // Register name as string
    .handle([](const HttpRequest& req) -> HttpResponse {
        // Automatic type deduction
        int id = PATH_PARAM(req, id);           // Auto-deduced as int
        int page = QUERY_PARAM(req, page);      // Auto-deduced as int
        std::string name = QUERY_PARAM(req, name);  // Auto-deduced as string
        
        return ResponseBuilder::ok().data(users);
    });
```

**Macro Expansion Principle:**
```cpp
#define PATH_PARAM(req, name) \
    ({ \
        int type = uvapi::ParamTypeRegistry::getPathParamType(#name); \
        if (type == static_cast<int>(uvapi::ParamDataType::STRING)) req.pathParam.get<std::string>(#name); \
        else if (type == static_cast<int>(uvapi::ParamDataType::INT)) req.pathParam.get<int>(#name); \
        else if (type == static_cast<int>(uvapi::ParamDataType::INT64)) req.pathParam.get<int64_t>(#name); \
        else if (type == static_cast<int>(uvapi::ParamDataType::DOUBLE)) req.pathParam.get<double>(#name); \
        else if (type == static_cast<int>(uvapi::ParamDataType::FLOAT)) req.pathParam.get<float>(#name); \
        else if (type == static_cast<int>(uvapi::ParamDataType::BOOL)) req.pathParam.get<bool>(#name); \
        else std::nullopt; \
    })
```

### 8. Recommended Practices

**Principle:** Provide clear best practices to reduce ambiguity and improve consistency.

#### Parameter Access

**Recommended Way (Template Parameter):**
```cpp
int id = req.pathParam.get<int>("id");
int page = req.queryParam.get<int>("page");
```

**Convenient Way (Auto Type Inference):**
```cpp
int id = PATH_PARAM(req, id);
int page = QUERY_PARAM(req, page);
```

#### Request Body Parsing

**Only Recommended Way:**
```cpp
User user = req.parseBody<User>();
```

**Why Only One Way?**
- Clear and concise: One way, no ambiguity
- Automatic validation: Automatically calls Schema validation
- Type safe: Compile-time type checking
- Unified style: All APIs use the same pattern

#### Schema Definition

**Recommended Pattern:**
```cpp
// Define reusable Schema
auto userSchema = Schema<User>()
    .field("username", Required<std::string>()).length(3, 20)
    .field("email", Required<std::string>()).pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
    .field("age", OptionalWithDefault<int>(18)).range(18, 120);

// Use in route
api.post("/api/users")
    .body(userSchema)
    .handle([](const HttpRequest& req) -> HttpResponse {
        User user = req.parseBody<User>();  // Automatic validation
        return ResponseBuilder::created().data(user);
    });
```

### 9. Method Chaining

**Principle:** All methods return `ResponseBuilder&` for fluent API.

**Implementation:**
```cpp
ResponseBuilder& status(int code) {
    status_code_ = code;
    return *this;
}

ResponseBuilder& message(const std::string& msg) {
    message_ = msg;
    return *this;
}

// Chained calls
auto response = ResponseBuilder::ok()
    .message("Success")
    .requestId("12345")
    .data(user);
```

**Benefits:**
- Fluent API
- Better readability
- Concise code
- Intuitive usage

### 7. Implicit Conversion

**Principle:** Support implicit conversion to HttpResponse.

**Implementation:**
```cpp
operator HttpResponse() const {
    return build();
}

// Implicit conversion (recommended)
HttpResponse resp = ResponseBuilder::ok().data(user);

// Explicit conversion (optional)
HttpResponse resp = ResponseBuilder::ok().data(user).build();
```

**Benefits:**
- Reduces boilerplate
- Cleaner code
- Automatic conversion
- Type safety maintained

## Architecture

### Class Structure

```
ResponseBuilder
├── Private Members
│   ├── status_code_ (int)
│   ├── message_ (string)
│   ├── headers_ (map<string, string>)
│   └── pending_data_ (string)
│
├── Static Constructors
│   ├── ok()
│   ├── created()
│   ├── badRequest()
│   ├── unauthorized()
│   ├── forbidden()
│   ├── notFound()
│   └── internalServerError()
│
├── Declarative Methods
│   ├── status()
│   ├── message()
│   ├── header()
│   ├── contentType()
│   ├── cacheControl()
│   ├── requestId()
│   └── traceId()
│
├── Data Methods
│   └── data<T>() (template)
│
├── Build Methods
│   ├── build()
│   └── operator HttpResponse()
│
└── Factory Functions
    ├── makeSuccessResponse()
    ├── makeCreatedResponse()
    ├── makeErrorResponse()
    ├── makeNotFoundResponse()
    └── makeListResponse()
```

### Data Flow

```
Request Handler
    ↓
ResponseBuilder::ok()
    ↓
.status() → Update status_code_
    ↓
.message() → Update message_
    ↓
.data(user) → Call user.toJson()
                ↓
            Try-catch block
                ↓
            Update pending_data_
                ↓
            Update status on error
    ↓
Implicit conversion (operator HttpResponse())
    ↓
build() → Create HttpResponse
    ↓
Return HttpResponse
```

## Performance Considerations

### 1. Zero-Copy String Operations

```cpp
// Move semantics for efficiency
ResponseBuilder& message(const std::string& msg) {
    message_ = msg;  // Copy or move based on call site
    return *this;
}

// Use move semantics when possible
auto response = ResponseBuilder::ok()
    .message(std::string("Success"));  // Move
```

### 2. Compile-Time Optimization

```cpp
// Template specialization for different types
template<typename T>
auto data(const T& instance) -> typename std::enable_if<
    has_toJson<T>::value,
    ResponseBuilder&
>::type {
    // Type-specific implementation
}

// Optimized at compile time
```

### 3. Inline Methods

```cpp
// Inline for performance
inline ResponseBuilder& status(int code) {
    status_code_ = code;
    return *this;
}
```

## Testing Strategy

### Unit Tests

```cpp
TEST(ResponseBuilder, BasicBuilding) {
    HttpResponse resp = ResponseBuilder::ok()
        .message("Test")
        .build();
    
    ASSERT_EQ(resp.status_code, 200);
    ASSERT_TRUE(resp.body.find("Test") != std::string::npos);
}

TEST(ResponseBuilder, ChainedCalls) {
    HttpResponse resp = ResponseBuilder::ok()
        .status(200)
        .message("Test")
        .requestId("123")
        .build();
    
    ASSERT_EQ(resp.status_code, 200);
}
```

### Integration Tests

```cpp
TEST(ResponseBuilder, ImplicitConversion) {
    HttpResponse resp = ResponseBuilder::ok().data(user);
    
    ASSERT_EQ(resp.status_code, 200);
    ASSERT_TRUE(resp.body.find("Alice") != std::string::npos);
}
```

## Best Practices

1. **Use Factory Functions** - `makeSuccessResponse()` instead of `ResponseBuilder().status(200)`
2. **Implement toJson()** - For all data models
3. **Chain Methods** - For better readability
4. **Handle Exceptions** - In toJson() methods
5. **Use Declarative Style** - Describe "what", not "how"
6. **Avoid Global State** - Use factory functions
7. **Leverage Type System** - Let compiler catch errors
8. **Document Errors** - Provide meaningful error messages

## Future Enhancements

1. **Validation Framework** - Add request validation DSL
2. **Pagination Support** - Built-in pagination helpers
3. **Response Caching** - Automatic cache control
4. **Rate Limiting** - Built-in rate limiting
5. **Metrics Integration** - Automatic metrics collection
6. **Tracing Support** - Distributed tracing integration

## References

- [Response DSL Guide](../guide/response-dsl.md)
- [API Reference](../api/response-dsl.md)
- [Examples](../examples/response-dsl.md)