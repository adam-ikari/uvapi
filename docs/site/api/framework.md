# Framework API Reference

## Namespace

```cpp
namespace uvapi {
namespace restful {
```

## Core Classes

### Server

Basic server class for handling HTTP requests.

```cpp
namespace server {
class Server {
public:
    Server(uv_loop_t* loop);
    ~Server();
    
    void addRoute(const std::string& path, HttpMethod method, 
                  std::function<HttpResponse(const HttpRequest&)> handler);
    void listen(const std::string& host, int port);
    
    void stop();
    bool isRunning() const;
};
}
```

### HttpRequest

Represents an incoming HTTP request.

```cpp
struct HttpRequest {
    HttpMethod method;
    std::string path;
    std::string query;
    std::map<std::string, std::string> headers;
    std::string body;
    
    // Query parameter access
    std::string getQuery(const std::string& key) const;
    std::string getHeader(const std::string& key) const;
};
```

### HttpResponse

Represents an HTTP response.

```cpp
struct HttpResponse {
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;
    
    HttpResponse(int code = 200);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
};
```

### ResponseBuilder

Declarative response builder for constructing HTTP responses.

```cpp
class ResponseBuilder {
public:
    // Static constructor methods
    static ResponseBuilder ok();
    static ResponseBuilder created();
    static ResponseBuilder badRequest();
    static ResponseBuilder unauthorized();
    static ResponseBuilder forbidden();
    static ResponseBuilder notFound();
    static ResponseBuilder internalServerError();
    
    // Declarative methods
    ResponseBuilder& status(int code);
    ResponseBuilder& message(const std::string& msg);
    ResponseBuilder& header(const std::string& key, const std::string& value);
    ResponseBuilder& contentType(const std::string& type);
    ResponseBuilder& cacheControl(const std::string& control);
    ResponseBuilder& requestId(const std::string& id);
    ResponseBuilder& traceId(const std::string& id);
    
    // Data serialization
    template<typename T>
    ResponseBuilder& data(const T& instance);
    
    // Build response
    HttpResponse build() const;
    
    // Implicit conversion
    operator HttpResponse() const;
};
```

## Enums

### HttpMethod

HTTP methods supported by the framework.

```cpp
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS
};
```

## Factory Functions

```cpp
// Success response template
ResponseBuilder makeSuccessResponse();

// Created response template
ResponseBuilder makeCreatedResponse();

// Error response template
ResponseBuilder makeErrorResponse();

// Not found response template
ResponseBuilder makeNotFoundResponse();

// List response template
ResponseBuilder makeListResponse();
```

## JSON Support

### JSON::Object

Object for building JSON objects.

```cpp
class JSON::Object {
public:
    JSON::Object();
    
    JSON::Object& set(const std::string& key, const std::string& value);
    JSON::Object& set(const std::string& key, int64_t value);
    JSON::Object& set(const std::string& key, double value);
    JSON::Object& set(const std::string& key, bool value);
    
    std::string toString() const;
    std::string toCompactString() const;
};
```

### JSON::Array

Array for building JSON arrays.

```cpp
class JSON::Array {
public:
    JSON::Array();
    
    JSON::Array& add(const std::string& value);
    JSON::Array& add(int64_t value);
    JSON::Array& add(double value);
    JSON::Array& add(bool value);
    
    std::string toString() const;
    std::string toCompactString() const;
};
```

## Middleware

### Middleware

Base class for request/response middleware.

```cpp
class Middleware {
public:
    virtual ~Middleware() = default;
    virtual bool process(HttpRequest& req, HttpResponse& resp) = 0;
};
```

### RateLimiter

Rate limiting middleware.

```cpp
class RateLimiter : public Middleware {
public:
    RateLimiter(int max_requests, int window_seconds);
    
    bool process(HttpRequest& req, HttpResponse& resp) override;
};
```

## Examples

### Basic Server

```cpp
uv_loop_t* loop = uv_default_loop();
server::Server server(loop);

server.addRoute("/hello", HttpMethod::GET, [](const HttpRequest& req) -> HttpResponse {
    HttpResponse resp(200);
    resp.body = "Hello, World!";
    return resp;
});

server.listen("0.0.0.0", 8080);
uv_run(loop, UV_RUN_DEFAULT);
```

### Using Response DSL

```cpp
auto handler = [](const HttpRequest& req) -> HttpResponse {
    User user = {1, "Alice"};
    
    return ResponseBuilder::ok()
        .message("Success")
        .requestId("12345")
        .data(user);
};
```

### JSON Serialization

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
```

## See Also

- [Response DSL Guide](../guide/response-dsl.md)
- [Quick Start](../guide/quick-start.md)
- [Examples](../examples/)