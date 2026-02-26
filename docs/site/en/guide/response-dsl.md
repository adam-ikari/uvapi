# Response DSL Guide

Response DSL provides a declarative way to build HTTP responses, supporting response structure declaration outside handler functions.

## Design Principles

1. **Zero Global Variables** - Use factory functions returning local objects
2. **Declarative Style** - Describe response properties, not execute actions
3. **Type Safety** - Compile-time type checking, automatic serialization
4. **Error Handling** - Catch exceptions, return error responses
5. **Implicit Conversion** - Automatically convert to HttpResponse

## Basic Usage

### Quick Constructor Methods

```cpp
ResponseBuilder::ok()                    // 200 Success
ResponseBuilder::created()              // 201 Created
ResponseBuilder::badRequest()           // 400 Bad Request
ResponseBuilder::unauthorized()          // 401 Unauthorized
ResponseBuilder::forbidden()             // 403 Forbidden
ResponseBuilder::notFound()             // 404 Not Found
ResponseBuilder::internalServerError()  // 500 Internal Server Error
```

### Declarative Methods

```cpp
ResponseBuilder::created()
    .status(201)                          // Describe status is 201
    .message("User created")               // Describe message is "User created"
    .header("X-Custom", "value")          // Describe header
    .contentType("application/json")     // Describe Content-Type
    .cacheControl("no-cache")             // Describe Cache-Control
    .requestId("12345")                   // Describe request ID
    .traceId("abc-def-ghi")               // Describe trace ID
```

### Data Serialization

```cpp
.data(user)                            // Describe data is user (auto-serialize)
.data(users)                           // Describe data is users array
.data("{\"key\":\"value\"}")           // Describe data is JSON string
```

## Factory Functions

To avoid using static global variables, the framework provides factory functions:

```cpp
makeSuccessResponse()    // Success response template
makeCreatedResponse()    // Create user response template
makeErrorResponse()      // Error response template
makeNotFoundResponse()  // Not found response template
makeListResponse()      // List response template
```

## Automatic Serialization

ResponseBuilder automatically detects the `toJson()` method of objects:

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
HttpResponse resp = ResponseBuilder::ok().data(user);
```

## Implicit Conversion

Supports implicit conversion to HttpResponse, no need to explicitly call `.toHttpResponse()`:

```cpp
// Implicit conversion (recommended)
HttpResponse resp = ResponseBuilder::ok().data(user);

// Explicit conversion (optional)
HttpResponse resp = ResponseBuilder::ok()
    .message("Success")
    .data(user)
    .toHttpResponse();
```

## Complete Examples

### Example 1: Create User

```cpp
auto create_user_handler = [](const HttpRequest& req) -> HttpResponse {
    User user = {1, "Alice", "alice@example.com"};
    
    return ResponseBuilder::created()
        .message("User created successfully")
        .requestId("12345")
        .data(user);
};
```

### Example 2: Get User List

```cpp
auto list_users_handler = [](const HttpRequest& req) -> HttpResponse {
    std::vector<User> users = {
        {1, "Alice", "alice@example.com"},
        {2, "Bob", "bob@example.com"}
    };
    
    return makeListResponse()
        .data(users);
};
```

### Example 3: Error Response

```cpp
auto error_handler = [](const HttpRequest& req) -> HttpResponse {
    return makeErrorResponse()
        .data("{\"error\":\"Invalid input\"}");
};
```

## Error Handling

ResponseBuilder automatically catches possible exceptions from `toJson()`:

```cpp
struct BadUser {
    std::string toJson() const {
        return "";  // Return empty string (error case)
    }
};

BadUser user;
HttpResponse resp = ResponseBuilder::ok().data(user);

// Automatically catches exception, returns error response
// resp.status_code == 500
// resp.body == "{\"code\":\"500\",\"message\":\"Serialization error\",\"data\":\"{}}"
```

## Best Practices

1. **Use Factory Functions** - Avoid using static global variables
2. **Use Declarative Style** - Describe response properties, not execute actions
3. **Leverage Implicit Conversion** - Reduce code, improve readability
4. **Implement toJson() Method** - Let data models support automatic serialization
5. **Chain Calls** - Improve code readability
6. **Error Handling** - Handle exceptions in toJson(), return error responses

## Related Documentation

- [JSON Usage Guide](./json-usage.md) - Learn JSON serialization
- [API Reference](../api/response-dsl.md) - View Response DSL API
- [Examples](../examples/response-dsl.md) - View complete examples