# Design

This section contains design documentation for the UVAPI framework.

## Design Documents

- [DSL Design](./dsl-design.md) - Response DSL design philosophy and principles
- [Architecture](./architecture.md) - Framework architecture and components

## Design Principles

UVAPI follows these core design principles:

1. **Type Safety** - Leverage C++ type system for compile-time error detection
2. **Zero Global Variables** - Support multi-instance and testing
3. **Declarative Style** - Describe "what", not "how"
4. **Zero Overhead** - Minimal runtime overhead
5. **Modern C++** - Use C++11 features effectively
6. **Single-Threaded Model** - Avoid locks and thread synchronization for simplicity
7. **Type Inference** - Use templates and macros for compile-time type deduction
8. **Recommended Way First** - Provide clear best practice guidelines to reduce ambiguity

## See Also

- [Response DSL Guide](../guide/response-dsl.md)
- [API Reference](../api/framework.md)
- [Examples](../examples/)