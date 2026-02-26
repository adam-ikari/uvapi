# Examples

Welcome to the UVAPI examples collection. This page contains practical examples of using the framework.

## Available Examples

### Response DSL Examples

- [Response DSL Examples](./response-dsl.md) - Comprehensive examples of using Response DSL

### JSON Usage Examples

Coming soon...

### Complete API Examples

Coming soon...

### Static Files Examples

Coming soon...

### Middleware Examples

Coming soon...

## Running Examples

Each example can be compiled and run using the following steps:

```bash
# Navigate to the example directory
cd examples/

# Compile
g++ -std=c++11 example.cpp -o example -I../include -L../build -luvhttp -luv -llhttp -lcjson -lmbedtls -lmbedx509 -lmbedcrypto -lpthread -ldl

# Run
./example
```

## Contributing Examples

To contribute a new example:

1. Create a new `.cpp` file in the `examples/` directory
2. Follow the existing example structure
3. Add documentation and comments
4. Update this index page

## See Also

- [Quick Start](../guide/quick-start.md)
- [Response DSL Guide](../guide/response-dsl.md)
- [API Reference](../api/framework.md)