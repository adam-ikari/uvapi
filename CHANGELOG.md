# Changelog

All notable changes to UVAPI will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.1] - 2026-03-03

### Changed
- **Example Updates**: Updated `declarative_dsl_auto_parse.cpp` to use new parameter access API
- **Error Handling in Examples**: Added error checking with `hasError()` and `errorMessage()` in all example handlers
- **Documentation Sync**: All examples now demonstrate proper error handling with operator[] syntax

## [1.1.0] - 2026-03-03

### Added
- **Error Reporting Mechanism**: Added `hasError()` and `errorMessage()` methods to `ParamValue` class for detailed error information
- **Explicit Type Conversion**: Added `as<T>()` method for explicit type conversion with error checking
- **Comprehensive Tests**: Added 45 unit tests for `ParamValue` and `ParamAccessor` classes
- **Thread Safety Documentation**: Added detailed thread safety documentation for `ParamValue` class
- **Version Information**: Added version tracking system with `version.h` header

### Changed
- **Parameter Access API**: Improved parameter access using `operator[]` with automatic type deduction
- **Boolean Parsing**: Restricted boolean parsing to only accept "true" and "false" (case-insensitive)
- **Error Messages**: Enhanced error messages with detailed information about conversion failures
- **Code Quality**: Fixed const correctness violations by using `mutable` keyword for error state members
- **Documentation**: Updated all documentation to reflect new parameter access API and error handling

### Fixed
- **Const Correctness**: Fixed const correctness violation in `ParamValue` class by making error state members `mutable`
- **C++11 Compatibility**: Replaced C++17 `if constexpr` with C++11 compatible SFINAE using `std::enable_if`
- **Range Checking**: Fixed integer and floating point range checking logic for proper overflow/underflow detection
- **Type Conversion**: Fixed silent type conversion failures by adding explicit error reporting

### Security
- **Input Validation**: Improved input validation with detailed error messages
- **Type Safety**: Enhanced type safety with explicit error checking for all type conversions
- **Silent Failures**: Eliminated silent type conversion failures that could lead to security vulnerabilities

### Performance
- **Zero Overhead**: Maintained zero runtime overhead for successful type conversions
- **Compile-Time Optimization**: Template specialization ensures compile-time type checking
- **No Dynamic Dispatch**: Eliminated virtual calls for better performance

### Breaking Changes
- **Deprecated APIs**: Deprecated `get<T>()` method and `PATH_PARAM`/`QUERY_PARAM` macros
- **Boolean Parsing**: "1", "yes", "on" are no longer accepted as boolean values
- **API Simplification**: Removed redundant parameter access methods, keeping only `operator[]` syntax

### Migration Guide
- Replace `req.pathParam.get<int>("id")` with `req.pathParam["id"]`
- Add error checking: `if (param.hasError()) { /* handle error */ }`
- Update boolean values: use "true"/"false" instead of "1"/"yes"/"on"

### Statistics
- Files changed: 10
- Lines added: 1092
- Lines removed: 151
- Tests added: 45
- Code coverage: ParamValue class 100% covered

## [1.0.0] - Initial Release

### Features
- Type-safe RESTful API framework built on UVHTTP
- Automatic serialization/deserialization with compile-time type checking
- High-performance zero-copy optimization
- Modern C++11 template metaprogramming
- Zero exception error handling
- Multi-server support with shared event loop
- Zero global variables for testability
- Declarative DSL for API definition
- ResponseBuilder for HTTP response construction
- Schema validation for request bodies
- WebSocket support (optional)
- Static file serving (optional)
- Rate limiting (optional)

[1.1.0]: https://github.com/adam-ikari/uvapi/releases/tag/v1.1.0
[1.0.0]: https://github.com/adam-ikari/uvapi/releases/tag/v1.0.0