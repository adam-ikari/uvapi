# JSON Usage Guide

This guide explains how to use JSON serialization in UVAPI.

## JSON::Object

`JSON::Object` provides a fluent interface for building JSON objects.

### Basic Usage

```cpp
JSON::Object obj;
obj.set("name", "Alice");
obj.set("age", 30);
obj.set("active", true);

std::string json = obj.toCompactString();
// Result: {"name":"Alice","age":30,"active":true}
```

### Chained Calls

```cpp
std::string json = JSON::Object()
    .set("id", 1)
    .set("name", "Alice")
    .set("email", "alice@example.com")
    .toCompactString();
```

### Pretty Print

```cpp
std::string json = JSON::Object()
    .set("id", 1)
    .set("name", "Alice")
    .toString();
// Result with formatting
```

## JSON::Array

`JSON::Array` provides a fluent interface for building JSON arrays.

### Basic Usage

```cpp
JSON::Array arr;
arr.add("Alice");
arr.add("Bob");
arr.add("Charlie");

std::string json = arr.toCompactString();
// Result: ["Alice","Bob","Charlie"]
```

### Chained Calls

```cpp
std::string json = JSON::Array()
    .add("Alice")
    .add("Bob")
    .add("Charlie")
    .toCompactString();
```

### Mixed Types

```cpp
std::string json = JSON::Array()
    .add("Alice")
    .add(30)
    .add(true)
    .toCompactString();
// Result: ["Alice",30,true]
```

## Integration with Response DSL

Response DSL automatically calls `toJson()` method on objects:

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

auto handler = [](const HttpRequest& req) -> HttpResponse {
    User user = {1, "Alice", "alice@example.com"};
    
    return ResponseBuilder::ok()
        .message("User retrieved")
        .data(user);  // Calls user.toJson() automatically
};
```

## Complex Structures

### Nested Objects

```cpp
std::string json = JSON::Object()
    .set("user", JSON::Object()
        .set("id", 1)
        .set("name", "Alice")
        .toCompactString())
    .set("metadata", JSON::Object()
        .set("created_at", "2026-02-26")
        .set("updated_at", "2026-02-26")
        .toCompactString())
    .toCompactString();
```

### Arrays of Objects

```cpp
JSON::Array users;
users.add(JSON::Object()
    .set("id", 1)
    .set("name", "Alice")
    .toCompactString());
users.add(JSON::Object()
    .set("id", 2)
    .set("name", "Bob")
    .toCompactString());

std::string json = JSON::Object()
    .set("users", users.toCompactString())
    .set("total", 2)
    .toCompactString();
```

## Best Practices

1. **Use toCompactString() for HTTP responses** - Minimizes bandwidth
2. **Implement toJson() for all data models** - Consistent serialization
3. **Chain calls for readability** - Fluent API
4. **Handle exceptions** - Catch and handle serialization errors
5. **Use appropriate data types** - Numbers, strings, booleans

## See Also

- [Response DSL Guide](./response-dsl.md)
- [API Reference](../api/framework.md)
- [Examples](../examples/)