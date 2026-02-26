# Response DSL API Reference

## Overview

Response DSL provides a declarative interface for building HTTP responses with automatic JSON serialization.

## Class: ResponseBuilder

### Static Constructor Methods

#### ok()

Creates a response builder with status 200.

```cpp
static ResponseBuilder ok();
```

**Returns:** ResponseBuilder instance with status code 200.

**Example:**
```cpp
auto response = ResponseBuilder::ok()
    .message("Success")
    .data(user);
```

#### created()

Creates a response builder with status 201.

```cpp
static ResponseBuilder created();
```

**Returns:** ResponseBuilder instance with status code 201.

**Example:**
```cpp
auto response = ResponseBuilder::created()
    .message("Resource created")
    .data(newResource);
```

#### badRequest()

Creates a response builder with status 400.

```cpp
static ResponseBuilder badRequest();
```

**Returns:** ResponseBuilder instance with status code 400.

**Example:**
```cpp
auto response = ResponseBuilder::badRequest()
    .message("Invalid input")
    .data(errorDetails);
```

#### unauthorized()

Creates a response builder with status 401.

```cpp
static ResponseBuilder unauthorized();
```

**Returns:** ResponseBuilder instance with status code 401.

**Example:**
```cpp
auto response = ResponseBuilder::unauthorized()
    .message("Authentication required");
```

#### forbidden()

Creates a response builder with status 403.

```cpp
static ResponseBuilder forbidden();
```

**Returns:** ResponseBuilder instance with status code 403.

**Example:**
```cpp
auto response = ResponseBuilder::forbidden()
    .message("Access denied");
```

#### notFound()

Creates a response builder with status 404.

```cpp
static ResponseBuilder notFound();
```

**Returns:** ResponseBuilder instance with status code 404.

**Example:**
```cpp
auto response = ResponseBuilder::notFound()
    .message("Resource not found");
```

#### internalServerError()

Creates a response builder with status 500.

```cpp
static ResponseBuilder internalServerError();
```

**Returns:** ResponseBuilder instance with status code 500.

**Example:**
```cpp
auto response = ResponseBuilder::internalServerError()
    .message("Internal server error");
```

### Declarative Methods

#### status()

Sets the HTTP status code.

```cpp
ResponseBuilder& status(int code);
```

**Parameters:**
- `code` - HTTP status code (e.g., 200, 404, 500)

**Returns:** Reference to ResponseBuilder for method chaining.

**Example:**
```cpp
auto response = ResponseBuilder::ok().status(200);
```

#### message()

Sets the response message.

```cpp
ResponseBuilder& message(const std::string& msg);
```

**Parameters:**
- `msg` - Message string

**Returns:** Reference to ResponseBuilder for method chaining.

**Example:**
```cpp
auto response = ResponseBuilder::ok()
    .message("Operation successful");
```

#### header()

Adds a custom HTTP header.

```cpp
ResponseBuilder& header(const std::string& key, const std::string& value);
```

**Parameters:**
- `key` - Header name
- `value` - Header value

**Returns:** Reference to ResponseBuilder for method chaining.

**Example:**
```cpp
auto response = ResponseBuilder::ok()
    .header("X-Custom-Header", "custom-value");
```

#### contentType()

Sets the Content-Type header.

```cpp
ResponseBuilder& contentType(const std::string& type);
```

**Parameters:**
- `type` - Content-Type (e.g., "application/json")

**Returns:** Reference to ResponseBuilder for method chaining.

**Example:**
```cpp
auto response = ResponseBuilder::ok()
    .contentType("application/json");
```

#### cacheControl()

Sets the Cache-Control header.

```cpp
ResponseBuilder& cacheControl(const std::string& control);
```

**Parameters:**
- `control` - Cache-Control directive

**Returns:** Reference to ResponseBuilder for method chaining.

**Example:**
```cpp
auto response = ResponseBuilder::ok()
    .cacheControl("no-cache");
```

#### requestId()

Sets the X-Request-ID header.

```cpp
ResponseBuilder& requestId(const std::string& id);
```

**Parameters:**
- `id` - Request ID

**Returns:** Reference to ResponseBuilder for method chaining.

**Example:**
```cpp
auto response = ResponseBuilder::ok()
    .requestId("req-12345");
```

#### traceId()

Sets the X-Trace-ID header.

```cpp
ResponseBuilder& traceId(const std::string& id);
```

**Parameters:**
- `id` - Trace ID

**Returns:** Reference to ResponseBuilder for method chaining.

**Example:**
```cpp
auto response = ResponseBuilder::ok()
    .traceId("trace-abc-def");
```

### Data Serialization

#### data()

Serializes data to JSON and sets it as response data.

```cpp
template<typename T>
ResponseBuilder& data(const T& instance);
```

**Parameters:**
- `instance` - Object with `toJson()` method or JSON string

**Returns:** Reference to ResponseBuilder for method chaining.

**Behavior:**
- Automatically detects `toJson()` method
- Catches serialization exceptions
- Returns error response on failure

**Example:**
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

User user = {1, "Alice"};
auto response = ResponseBuilder::ok()
    .data(user);
```

### Build Methods

#### build()

Builds and returns an HttpResponse.

```cpp
HttpResponse build() const;
```

**Returns:** HttpResponse instance.

**Example:**
```cpp
HttpResponse resp = ResponseBuilder::ok()
    .message("Success")
    .build();
```

#### operator HttpResponse()

Implicit conversion operator to HttpResponse.

```cpp
operator HttpResponse() const;
```

**Returns:** HttpResponse instance.

**Example:**
```cpp
// Implicit conversion (recommended)
HttpResponse resp = ResponseBuilder::ok().data(user);

// Explicit conversion (optional)
HttpResponse resp = ResponseBuilder::ok()
    .message("Success")
    .data(user)
    .toHttpResponse();
```

## Factory Functions

### makeSuccessResponse()

Creates a success response template.

```cpp
ResponseBuilder makeSuccessResponse();
```

**Returns:** ResponseBuilder configured for success responses.

### makeCreatedResponse()

Creates a created response template.

```cpp
ResponseBuilder makeCreatedResponse();
```

**Returns:** ResponseBuilder configured for created responses.

### makeErrorResponse()

Creates an error response template.

```cpp
ResponseBuilder makeErrorResponse();
```

**Returns:** ResponseBuilder configured for error responses.

### makeNotFoundResponse()

Creates a not found response template.

```cpp
ResponseBuilder makeNotFoundResponse();
```

**Returns:** ResponseBuilder configured for not found responses.

### makeListResponse()

Creates a list response template.

```cpp
ResponseBuilder makeListResponse();
```

**Returns:** ResponseBuilder configured for list responses.

## Error Handling

ResponseBuilder automatically handles serialization errors:

```cpp
struct BadModel {
    std::string toJson() const {
        return "";  // Returns empty string (error case)
    }
};

BadModel model;
auto response = ResponseBuilder::ok().data(model);

// Automatically catches exception
// response.status_code == 500
// response.body == "{\"code\":\"500\",\"message\":\"Serialization error\",\"data\":\"{}}"
```

## Complete Example

```cpp
auto create_user_handler = [](const HttpRequest& req) -> HttpResponse {
    User user = {1, "Alice", "alice@example.com"};
    
    return ResponseBuilder::created()
        .message("User created successfully")
        .requestId("12345")
        .traceId("abc-def-ghi")
        .contentType("application/json")
        .cacheControl("no-cache")
        .data(user);
};
```

## Response Format

Standard response format:

```json
{
  "code": "200",
  "message": "Success",
  "data": {
    "id": 1,
    "name": "Alice"
  }
}
```

## See Also

- [Response DSL Guide](../guide/response-dsl.md)
- [Framework API](./framework.md)
- [JSON Usage](../guide/json-usage.md)