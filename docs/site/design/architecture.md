# Architecture

This document describes the architecture of the UVAPI framework.

## Overview

UVAPI is built on top of UVHTTP, which itself is based on libuv for asynchronous I/O and llhttp for HTTP parsing.

## Core Components

### 1. Framework Layer

The framework layer provides high-level C++ abstractions:

- **Server**: HTTP server implementation
- **Request/Response**: HTTP request/response models
- **ResponseBuilder**: Declarative response building DSL
- **Middleware**: Request/response processing pipeline

### 2. HTTP Layer

Based on UVHTTP:

- **Connection Management**: TCP connection handling
- **Request Parsing**: HTTP/1.1 request parsing
- **Response Building**: HTTP response generation
- **Routing**: URL routing and handler dispatch

### 3. I/O Layer

Based on libuv:

- **Event Loop**: Asynchronous event processing
- **Network I/O**: TCP/UDP socket operations
- **File I/O**: Asynchronous file operations
- **DNS Resolution**: Asynchronous DNS lookups

## Data Flow

```
Client Request
    ↓
libuv Event Loop
    ↓
UVHTTP Connection
    ↓
HTTP Parser (llhttp)
    ↓
UVAPI Server
    ↓
Router
    ↓
Handler Function
    ↓
ResponseBuilder
    ↓
HTTP Response
    ↓
UVHTTP Connection
    ↓
libuv Event Loop
    ↓
Client Response
```

## Key Design Decisions

### 1. Zero Global Variables

All state is passed through parameters or object members:

```cpp
// ❌ Avoid
static ResponseBuilder* instance_;

// ✅ Use
ResponseBuilder makeSuccessResponse() {
    return ResponseBuilder().status(200);
}
```

### 2. Type Safety

Leverage C++ type system:

```cpp
template<typename T>
auto data(const T& instance) -> typename std::enable_if<
    std::is_same<decltype(std::declval<T>().toJson()), std::string>::value,
    ResponseBuilder&
>::type {
    // ...
}
```

### 3. Declarative DSL

Describe "what" not "how":

```cpp
// ✅ Declarative
return ResponseBuilder::ok()
    .message("Success")
    .data(user);

// ❌ Imperative
HttpResponse resp(200);
resp.message = "Success";
resp.data = toJson(user);
```

## Memory Management

### Allocation Strategy

- **Stack allocation** for small objects
- **Smart pointers** for heap objects
- **Move semantics** for efficiency
- **RAII** for resource management

### Object Lifecycle

```
Request Created
    ↓
Processed by Handler
    ↓
Response Built
    ↓
Response Sent
    ↓
Response Destroyed
    ↓
Request Destroyed
```

## Performance Optimization

### 1. Zero-Copy

- Direct buffer usage when possible
- Move semantics instead of copies
- String views for read-only strings

### 2. Compile-Time Optimization

- Template metaprogramming
- Inline functions
- Constexpr where applicable

### 3. Event-Driven

- Non-blocking I/O
- Single-threaded event loop
- Asynchronous operations

## Testing Strategy

### Unit Tests

- Test individual components
- Mock external dependencies
- Test error handling

### Integration Tests

- Test component interactions
- Real HTTP requests/responses
- Performance benchmarks

## Future Enhancements

1. **WebSocket Support**: Enhanced WebSocket API
2. **Middleware System**: Pluggable middleware pipeline
3. **Validation Framework**: Request/response validation
4. **Caching Layer**: Built-in response caching
5. **Metrics Collection**: Automatic metrics gathering
6. **Distributed Tracing**: OpenTelemetry integration

## See Also

- [DSL Design](./dsl-design.md)
- [API Reference](../api/framework.md)
- [Examples](../examples/)