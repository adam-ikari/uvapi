# JSON API Reference

## JSON::Object

Class for building JSON objects.

### Methods

#### JSON::Object()

Default constructor.

```cpp
JSON::Object();
```

#### set()

Sets a key-value pair in the object.

```cpp
JSON::Object& set(const std::string& key, const std::string& value);
JSON::Object& set(const std::string& key, int64_t value);
JSON::Object& set(const std::string& key, double value);
JSON::Object& set(const std::string& key, bool value);
```

**Parameters:**
- `key` - The key string
- `value` - The value (string, number, or boolean)

**Returns:** Reference to JSON::Object for method chaining.

**Example:**
```cpp
JSON::Object obj;
obj.set("name", "Alice");
obj.set("age", 30);
```

#### toString()

Converts the object to a formatted JSON string.

```cpp
std::string toString() const;
```

**Returns:** Formatted JSON string with whitespace.

**Example:**
```cpp
std::string json = obj.toString();
// Result: {\n  "name": "Alice",\n  "age": 30\n}
```

#### toCompactString()

Converts the object to a compact JSON string without whitespace.

```cpp
std::string toCompactString() const;
```

**Returns:** Compact JSON string without whitespace.

**Example:**
```cpp
std::string json = obj.toCompactString();
// Result: {"name":"Alice","age":30}
```

## JSON::Array

Class for building JSON arrays.

### Methods

#### JSON::Array()

Default constructor.

```cpp
JSON::Array();
```

#### add()

Adds a value to the array.

```cpp
JSON::Array& add(const std::string& value);
JSON::Array& add(int64_t value);
JSON::Array& add(double value);
JSON::Array& add(bool value);
```

**Parameters:**
- `value` - The value to add (string, number, or boolean)

**Returns:** Reference to JSON::Array for method chaining.

**Example:**
```cpp
JSON::Array arr;
arr.add("Alice");
arr.add(30);
```

#### toString()

Converts the array to a formatted JSON string.

```cpp
std::string toString() const;
```

**Returns:** Formatted JSON string with whitespace.

**Example:**
```cpp
std::string json = arr.toString();
// Result: [\n  "Alice",\n  30\n]
```

#### toCompactString()

Converts the array to a compact JSON string without whitespace.

```cpp
std::string toCompactString() const;
```

**Returns:** Compact JSON string without whitespace.

**Example:**
```cpp
std::string json = arr.toCompactString();
// Result: ["Alice",30]
```

## Usage Examples

### Simple Object

```cpp
std::string json = JSON::Object()
    .set("name", "Alice")
    .set("age", 30)
    .set("active", true)
    .toCompactString();
```

### Simple Array

```cpp
std::string json = JSON::Array()
    .add("Alice")
    .add("Bob")
    .add("Charlie")
    .toCompactString();
```

### Nested Structures

```cpp
std::string json = JSON::Object()
    .set("user", JSON::Object()
        .set("id", 1)
        .set("name", "Alice")
        .toCompactString())
    .set("tags", JSON::Array()
        .add("admin")
        .add("active")
        .toCompactString())
    .toCompactString();
```

## Integration with Response DSL

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

auto handler = [](const HttpRequest& req) -> HttpResponse {
    User user = {1, "Alice"};
    
    return ResponseBuilder::ok()
        .message("User retrieved")
        .data(user);  // Automatically calls toJson()
};
```

## See Also

- [JSON Usage Guide](../guide/json-usage.md)
- [Response DSL API](./response-dsl.md)
- [Framework API](./framework.md)