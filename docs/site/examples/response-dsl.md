# Response DSL Examples

This page provides practical examples of using the Response DSL in various scenarios.

## Basic Examples

### Example 1: Simple Success Response

```cpp
auto success_handler = [](const HttpRequest& req) -> HttpResponse {
    return ResponseBuilder::ok()
        .message("Operation successful");
};
```

**Response:**
```json
{
  "code": "200",
  "message": "Operation successful"
}
```

### Example 2: Created Resource

```cpp
struct User {
    int64_t id;
    std::string name;
    std::string email;
    
    std::string toJson() const {
        return JSON::Object()
            .set("id", id)
            .set("name", name)
            .set("email", email)
            .toCompactString();
    }
};

auto create_user_handler = [](const HttpRequest& req) -> HttpResponse {
    User user = {1, "Alice", "alice@example.com"};
    
    return ResponseBuilder::created()
        .message("User created successfully")
        .requestId("12345")
        .data(user);
};
```

**Response:**
```json
{
  "code": "201",
  "message": "User created successfully",
  "data": {
    "id": 1,
    "name": "Alice",
    "email": "alice@example.com"
  }
}
```

### Example 3: Error Response

```cpp
auto error_handler = [](const HttpRequest& req) -> HttpResponse {
    return ResponseBuilder::badRequest()
        .message("Invalid input parameters")
        .data("{\"errors\":{\"email\":\"Invalid email format\"}}");
};
```

**Response:**
```json
{
  "code": "400",
  "message": "Invalid input parameters",
  "data": "{\"errors\":{\"email\":\"Invalid email format\"}}"
}
```

## Advanced Examples

### Example 4: List Response with Pagination

```cpp
struct UserList {
    std::vector<User> users;
    int64_t total;
    int page;
    int page_size;
    
    std::string toJson() const {
        JSON::Array users_array;
        for (const auto& user : users) {
            users_array.add(user.toJson());
        }
        
        return JSON::Object()
            .set("users", users_array.toCompactString())
            .set("total", total)
            .set("page", page)
            .set("page_size", page_size)
            .toCompactString();
    }
};

auto list_users_handler = [](const HttpRequest& req) -> HttpResponse {
    std::vector<User> users = {
        {1, "Alice", "alice@example.com"},
        {2, "Bob", "bob@example.com"}
    };
    
    UserList list = {users, 2, 1, 10};
    
    return makeListResponse()
        .message("Users retrieved successfully")
        .requestId("67890")
        .data(list);
};
```

**Response:**
```json
{
  "code": "200",
  "message": "Users retrieved successfully",
  "data": {
    "users": [
      {"id": 1, "name": "Alice", "email": "alice@example.com"},
      {"id": 2, "name": "Bob", "email": "bob@example.com"}
    ],
    "total": 2,
    "page": 1,
    "page_size": 10
  }
}
```

### Example 5: Custom Headers

```cpp
auto custom_headers_handler = [](const HttpRequest& req) -> HttpResponse {
    return ResponseBuilder::ok()
        .message("Custom headers example")
        .header("X-Custom-Header", "custom-value")
        .header("X-Request-ID", "12345")
        .header("X-Trace-ID", "trace-abc")
        .contentType("application/json")
        .cacheControl("no-cache, no-store")
        .data("{\"custom\": true}");
};
```

### Example 6: Factory Function Usage

```cpp
auto factory_handler = [](const HttpRequest& req) -> HttpResponse {
    // Using factory functions for common response patterns
    if (req.method == HttpMethod::POST) {
        return makeCreatedResponse()
            .message("Resource created")
            .data(newResource);
    } else if (req.method == HttpMethod::GET) {
        return makeSuccessResponse()
            .message("Resource retrieved")
            .data(resource);
    } else if (req.method == HttpMethod::DELETE) {
        return makeSuccessResponse()
            .message("Resource deleted");
    }
    
    return makeErrorResponse()
        .message("Method not allowed");
};
```

## Error Handling Examples

### Example 7: Serialization Error Handling

```cpp
struct FailingModel {
    std::string toJson() const {
        throw std::runtime_error("Serialization failed");
    }
};

auto failing_handler = [](const HttpRequest& req) -> HttpResponse {
    FailingModel model;
    
    // ResponseBuilder catches the exception automatically
    return ResponseBuilder::ok()
        .message("This will fail")
        .data(model);
};
```

**Response (auto-generated error):**
```json
{
  "code": "500",
  "message": "Serialization error",
  "data": "{}"
}
```

### Example 8: Empty JSON Handling

```cpp
struct EmptyModel {
    std::string toJson() const {
        return "";  // Returns empty string
    }
};

auto empty_handler = [](const HttpRequest& req) -> HttpResponse {
    EmptyModel model;
    
    // Empty JSON returns error response
    return ResponseBuilder::ok()
        .message("Empty model")
        .data(model);
};
```

**Response (auto-generated error):**
```json
{
  "code": "500",
  "message": "Failed to serialize object",
  "data": "{}"
}
```

## Complete CRUD Example

### Example 9: User Management API

```cpp
// Data models
struct User {
    int64_t id;
    std::string name;
    std::string email;
    
    std::string toJson() const {
        return JSON::Object()
            .set("id", id)
            .set("name", name)
            .set("email", email)
            .toCompactString();
    }
};

// Create user
auto create_user = [](const HttpRequest& req) -> HttpResponse {
    User user = {1, "Alice", "alice@example.com"};
    return ResponseBuilder::created()
        .message("User created")
        .requestId(generateRequestId())
        .data(user);
};

// Get user
auto get_user = [](const HttpRequest& req) -> HttpResponse {
    int64_t user_id = std::stoll(req.path.substr(6)); // /users/123
    User user = {user_id, "Alice", "alice@example.com"};
    
    return ResponseBuilder::ok()
        .message("User retrieved")
        .data(user);
};

// Update user
auto update_user = [](const HttpRequest& req) -> HttpResponse {
    User user = {1, "Alice Updated", "alice.updated@example.com"};
    
    return ResponseBuilder::ok()
        .message("User updated")
        .data(user);
};

// Delete user
auto delete_user = [](const HttpRequest& req) -> HttpResponse {
    return ResponseBuilder::ok()
        .message("User deleted");
};

// List users
auto list_users = [](const HttpRequest& req) -> HttpResponse {
    std::vector<User> users = {
        {1, "Alice", "alice@example.com"},
        {2, "Bob", "bob@example.com"}
    };
    
    JSON::Array users_array;
    for (const auto& user : users) {
        users_array.add(user.toJson());
    }
    
    auto list_data = JSON::Object()
        .set("users", users_array.toCompactString())
        .set("total", users.size())
        .toCompactString();
    
    return makeListResponse()
        .message("Users retrieved")
        .data(list_data);
};
```

## Best Practices

1. **Use factory functions** for common response patterns
2. **Implement toJson() method** for all data models
3. **Chain methods** for better readability
4. **Handle exceptions** in toJson() methods
5. **Use declarative style** - describe "what", not "how"
6. **Add request IDs** for tracing and debugging
7. **Set appropriate content types** for different data formats
8. **Use implicit conversion** to reduce code

## See Also

- [Response DSL Guide](../guide/response-dsl.md)
- [API Reference](../api/response-dsl.md)
- [JSON Usage Guide](../guide/json-usage.md)