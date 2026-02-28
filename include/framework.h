/**
 * @file framework.h
 * @brief RESTful 低代码框架核心定义
 * 
 * 采用 server 和 api 两层架构，事件循环注入模式
 * 支持未来扩展其他 API 规范（如 GraphQL）
 */

#ifndef RESTFUL_FRAMEWORK_H
#define RESTFUL_FRAMEWORK_H

#include <string>
#include <map>
#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>
#include <sstream>
#include <algorithm>
#include <type_traits>
#include <limits>
#include <iostream>
#include <regex>
#include <ctime>
#include <iomanip>

// Forward declarations
class ICustomTypeHandler;

// ========== C++11 Optional 实现 ==========
namespace uvapi {

/**
 * @brief UVAPI 框架核心命名空间
 * 
 * ## 设计哲学
 * 
 * 1. **零异常 (Zero Exceptions)**: 框架完全不使用异常机制
 *    - 所有错误通过返回码、状态码或错误对象传递
 *    - 使用 ValidationResult、HttpResponse 等类型表示错误
 *    - 字符串转数字使用 strtol/strtod + errno 检查，而非 std::stoi/stod
 *    - 确保可预测的行为和更好的性能
 * 
 * 2. **声明式 (Declarative)**: DSL 采用声明式设计，而非命令式
 *    - 使用流式链式 API (fluent chain API) 定义路由、参数、Schema
 *    - Builder 模式提供清晰的声明式接口
 *    - 类型安全成员指针自动计算偏移量，无需手动 offsetof
 *    - 关注"做什么"而非"怎么做"
 * 
 * 3. **自动解析参数 (Automatic Parameter Parsing)**: 框架自动解析参数
 *    - 自动从 URL 提取查询参数和路径参数
 *    - ParamAccessor 提供类型安全的参数访问
 *    - 自动类型转换 (string→int/double/bool)
 *    - 支持默认值设置
 * 
 * 4. **自动校验 (Automatic Validation)**: 声明式的验证规则自动执行
 *    - 通过 DSL 声明验证规则 (required, min, max, pattern, enum)
 *    - 框架在请求处理时自动执行验证
 *    - 验证失败自动返回 400 Bad Request 及详细错误信息
 *    - 支持内置格式验证 (email, url, uuid, date, datetime, ipv4)
 * 
 * 5. **零全局变量**: 支持多实例，测试友好
 * 
 * 6. **RAII 优先**: 自动资源管理，避免内存泄漏
 * 
 * 7. **类型安全**: 编译时类型检查
 * 
 * 8. **高性能优先**: 零拷贝、事件驱动
 */

template<typename T>
class optional {
private:
    bool initialized_;
    alignas(T) unsigned char storage_[sizeof(T)];
    
    T* ptr() { return reinterpret_cast<T*>(storage_); }
    const T* ptr() const { return reinterpret_cast<const T*>(storage_); }
    
public:
    // 默认构造函数
    optional() : initialized_(false) {}
    
    // 有值构造函数（左值引用）
    explicit optional(const T& value) : initialized_(true) {
        new (ptr()) T(value);
    }
    
    // 有值构造函数（右值引用）
    optional(T&& value) : initialized_(true) {
        new (ptr()) T(std::move(value));
    }
    
    // 模板构造函数（支持隐式转换）
    template<typename U>
    explicit optional(U&& value, typename std::enable_if<std::is_convertible<U, T>::value, int>::type = 0)
        : initialized_(true) {
        new (ptr()) T(std::forward<U>(value));
    }
    
    // 拷贝构造函数
    optional(const optional& other) : initialized_(other.initialized_) {
        if (initialized_) {
            new (ptr()) T(*other.ptr());
        }
    }
    
    // 移动构造函数
    optional(optional&& other) : initialized_(other.initialized_) {
        if (initialized_) {
            new (ptr()) T(std::move(*other.ptr()));
        }
        other.initialized_ = false;
    }
    
    // 析构函数
    ~optional() {
        if (initialized_) {
            ptr()->~T();
            initialized_ = false;
        }
    }
    
    // 拷贝赋值运算符
    optional& operator=(const optional& other) {
        if (this != &other) {
            if (initialized_) {
                ptr()->~T();
            }
            initialized_ = other.initialized_;
            if (initialized_) {
                new (ptr()) T(*other.ptr());
            }
        }
        return *this;
    }
    
    // 移动赋值运算符
    optional& operator=(optional&& other) {
        if (this != &other) {
            if (initialized_) {
                ptr()->~T();
            }
            initialized_ = other.initialized_;
            if (initialized_) {
                new (ptr()) T(std::move(*other.ptr()));
            }
            other.initialized_ = false;
        }
        return *this;
    }
    
    // 赋值运算符（左值引用）
    optional& operator=(const T& value) {
        if (initialized_) {
            *ptr() = value;
        } else {
            new (ptr()) T(value);
            initialized_ = true;
        }
        return *this;
    }
    
    // 赋值运算符（右值引用）
    optional& operator=(T&& value) {
        if (initialized_) {
            *ptr() = std::move(value);
        } else {
            new (ptr()) T(std::move(value));
            initialized_ = true;
        }
        return *this;
    }
    
    // 检查是否有值
    bool has_value() const {
        return initialized_;
    }
    
    // 别名方法，符合项目命名规范（camelCase）
    bool hasValue() const {
        return initialized_;
    }
    
    // 获取值（异常版本）
    T& value() {
        if (!initialized_) {
            throw std::runtime_error("bad_optional_access");
        }
        return *ptr();
    }
    
    const T& value() const {
        if (!initialized_) {
            throw std::runtime_error("bad_optional_access");
        }
        return *ptr();
    }
    
    // 获取值或默认值（零异常版本）
    T value_or(const T& default_value) const {
        if (initialized_) {
            return *ptr();
        }
        return default_value;
    }
    
    // 操作符 ->
    T* operator->() {
        if (!initialized_) {
            throw std::runtime_error("bad_optional_access");
        }
        return ptr();
    }
    
    const T* operator->() const {
        if (!initialized_) {
            throw std::runtime_error("bad_optional_access");
        }
        return ptr();
    }
    
    // 操作符 *
    T& operator*() {
        if (!initialized_) {
            throw std::runtime_error("bad_optional_access");
        }
        return *ptr();
    }
    
    const T& operator*() const {
        if (!initialized_) {
            throw std::runtime_error("bad_optional_access");
        }
        return *ptr();
    }
    
    // 重置
    void reset() {
        if (initialized_) {
            ptr()->~T();
            initialized_ = false;
        }
    }
    
    // 隐式转换为 bool
    explicit operator bool() const {
        return initialized_;
    }
    
    // 交换
    void swap(optional& other) {
        if (initialized_ && other.initialized_) {
            std::swap(*ptr(), *other.ptr());
        } else if (initialized_ && !other.initialized_) {
            new (other.ptr()) T(std::move(*ptr()));
            ptr()->~T();
            other.initialized_ = true;
            initialized_ = false;
        } else if (!initialized_ && other.initialized_) {
            new (ptr()) T(std::move(*other.ptr()));
            other.ptr()->~T();
            initialized_ = true;
            other.initialized_ = false;
        }
    }
};

// 特化 void 类型
template<>
class optional<void> {
private:
    bool initialized_;
    
public:
    optional() : initialized_(false) {}
    optional(bool has_value) : initialized_(has_value) {}
    
    bool has_value() const { return initialized_; }
    
    // 别名方法，符合项目命名规范（camelCase）
    bool hasValue() const { return initialized_; }
    
    void reset() { initialized_ = false; }
    explicit operator bool() const { return initialized_; }
    
    void swap(optional& other) {
        std::swap(initialized_, other.initialized_);
    }
};

// 辅助函数：创建 optional
template<typename T>
optional<T> make_optional(T&& value) {
    return optional<T>(std::forward<T>(value));
}

// 比较运算符
template<typename T>
bool operator==(const optional<T>& lhs, const optional<T>& rhs) {
    if (!lhs.has_value() && !rhs.has_value()) return true;
    if (!lhs.has_value() || !rhs.has_value()) return false;
    return lhs.value() == rhs.value();
}

template<typename T>
bool operator!=(const optional<T>& lhs, const std::nullptr_t) {
    return lhs.has_value();
}

template<typename T>
bool operator==(const optional<T>& lhs, const T& rhs) {
    return lhs.has_value() && lhs.value() == rhs;
}

template<typename T>
bool operator==(const T& lhs, const optional<T>& rhs) {
    return rhs.has_value() && rhs.value() == lhs;
}

} // namespace uvapi

// libuv 头文件
extern "C" {
#include <uv.h>
}

// cJSON 头文件
extern "C" {
#include <cJSON.h>
}

// uvhttp 头文件
extern "C" {
#include <uvhttp.h>
#include <uvhttp_request.h>
#include <uvhttp_context.h>
#include <uvhttp_router.h>
#include <uvhttp_config.h>
}

namespace uvapi {
// ========== RAII 资源管理包装类（使用 std::shared_ptr）==========

// cJSON 自定义删除器
struct CJsonDeleter {
    void operator()(cJSON* ptr) const {
        if (ptr) {
            cJSON_Delete(ptr);
        }
    }
};

// cJSON shared_ptr 类型
using CJsonPtr = std::shared_ptr<cJSON>;

// 辅助函数：创建 CJsonPtr
inline CJsonPtr makeCJson(cJSON* ptr) {
    return CJsonPtr(ptr, CJsonDeleter());
}

// uvhttp_server_t 自定义删除器
struct UvhttpServerDeleter {
    void operator()(uvhttp_server_t* ptr) const {
        if (ptr) {
            uvhttp_server_free(ptr);
        }
    }
};

// uvhttp_server_t shared_ptr 类型
using UvhttpServerPtr = std::shared_ptr<uvhttp_server_t>;

// 辅助函数：创建 UvhttpServerPtr
inline UvhttpServerPtr makeUvhttpServer(uvhttp_server_t* ptr) {
    return UvhttpServerPtr(ptr, UvhttpServerDeleter());
}

// uvhttp_router_t 自定义删除器
struct UvhttpRouterDeleter {
    void operator()(uvhttp_router_t* ptr) const {
        if (ptr) {
            uvhttp_router_free(ptr);
        }
    }
};

// uvhttp_router_t shared_ptr 类型
using UvhttpRouterPtr = std::shared_ptr<uvhttp_router_t>;

// 辅助函数：创建 UvhttpRouterPtr
inline UvhttpRouterPtr makeUvhttpRouter(uvhttp_router_t* ptr) {
    return UvhttpRouterPtr(ptr, UvhttpRouterDeleter());
}

// uvhttp_config_t 自定义删除器
struct UvhttpConfigDeleter {
    void operator()(uvhttp_config_t* ptr) const {
        if (ptr) {
            uvhttp_config_free(ptr);
        }
    }
};

// uvhttp_config_t shared_ptr 类型
using UvhttpConfigPtr = std::shared_ptr<uvhttp_config_t>;

// 辅助函数：创建 UvhttpConfigPtr
inline UvhttpConfigPtr makeUvhttpConfig(uvhttp_config_t* ptr) {
    return UvhttpConfigPtr(ptr, UvhttpConfigDeleter());
}

// uvhttp_context_t 自定义删除器
struct UvhttpContextDeleter {
    void operator()(uvhttp_context_t* ptr) const {
        if (ptr) {
            uvhttp_context_destroy(ptr);
        }
    }
};

// uvhttp_context_t shared_ptr 类型
using UvhttpContextPtr = std::shared_ptr<uvhttp_context_t>;

// 辅助函数：创建 UvhttpContextPtr
inline UvhttpContextPtr makeUvhttpContext(uvhttp_context_t* ptr) {
    return UvhttpContextPtr(ptr, UvhttpContextDeleter());
}

// uvhttp_tls_context_t 自定义删除器
struct UvhttpTlsContextDeleter {
    void operator()(uvhttp_tls_context_t* ptr) const {
        if (ptr) {
            uvhttp_tls_context_free(ptr);
        }
    }
};

// uvhttp_tls_context_t shared_ptr 类型
using UvhttpTlsContextPtr = std::shared_ptr<uvhttp_tls_context_t>;

// 辅助函数：创建 UvhttpTlsContextPtr
inline UvhttpTlsContextPtr makeUvhttpTlsContext(uvhttp_tls_context_t* ptr) {
    return UvhttpTlsContextPtr(ptr, UvhttpTlsContextDeleter());
}

// char* 自定义删除器（用于 cJSON_Print 返回的字符串）
struct CJsonStringDeleter {
    void operator()(char* ptr) const {
        if (ptr) {
            free(ptr);
        }
    }
};

// char* shared_ptr 类型
using CJsonStringPtr = std::shared_ptr<char>;

// 辅助函数：创建 CJsonStringPtr
inline CJsonStringPtr makeCJsonString(char* ptr) {
    return CJsonStringPtr(ptr, CJsonStringDeleter());
}


struct ValidationResult {
    bool success_;
    std::string error_message;
    
    // 默认构造：成功
    ValidationResult() : success_(true), error_message("") {}
    
    // 从字符串隐式转换：失败
    ValidationResult(const std::string& error) : success_(false), error_message(error) {}
    
    // 从 const char* 隐式转换：失败
    ValidationResult(const char* error) : success_(false), error_message(error ? error : "") {}
    
    // 从 bool 隐式转换
    ValidationResult(bool success) : success_(success), error_message("") {}
    
    // 静态工厂方法（保留以兼容现有代码）
    static ValidationResult ok() {
        return ValidationResult{true};
    }
    
    static ValidationResult fail(const std::string& message) {
        return ValidationResult{message};
    }
    
    // 转换为 bool（用于 if 条件判断）
    explicit operator bool() const {
        return success_;
    }
};

// ========== Body 序列化和反序列化 ==========

// 字段类型枚举
enum class FieldType {
    STRING,    // 字符串
    INT8,      // 8位有符号整数
    INT16,     // 16位有符号整数
    INT,       // 32位有符号整数（INT32）
    INT64,     // 64位有符号整数
    UINT8,     // 8位无符号整数
    UINT16,    // 16位无符号整数
    UINT32,    // 32位无符号整数
    UINT64,    // 64位无符号整数
    FP32,      // 32位浮点数（单精度）
    FLOAT,     // 32位浮点数（别名）
    DOUBLE,    // 64位浮点数（双精度）
    BOOL,      // 布尔值
    ARRAY,     // 数组
    OBJECT,    // 对象
    DATE,      // 日期（YYYY-MM-DD）
    DATETIME,  // 日期时间（YYYY-MM-DD HH:MM:SS）
    EMAIL,     // 邮箱
    URL,       // URL
    UUID,      // UUID
    CUSTOM     // 自定义类型
};

// 字段验证规则
struct FieldValidation {
    bool required;
    int min_length;
    int max_length;
    bool has_min_length;  // 是否设置了最小长度
    bool has_max_length;  // 是否设置了最大长度
    double min_value;
    double max_value;
    bool has_min_value;  // 是否设置了最小值
    bool has_max_value;  // 是否设置了最大值
    std::string pattern;
    std::vector<std::string> enum_values;
    bool has_pattern;  // 是否设置了正则表达式
    bool has_enum;  // 是否设置了枚举值
    bool use_optional;  // 是否使用 optional 容器

    FieldValidation()
        : required(false), min_length(0), max_length(0), has_min_length(false), has_max_length(false),
          min_value(0), max_value(0), has_min_value(false), has_max_value(false),
          has_pattern(false), has_enum(false),
          use_optional(false) {}
};

// 应用验证规则
inline std::string applyValidation(const cJSON* json, const FieldValidation& validation, const std::string& field_name) {
    if (!json) return "";
    
    // 字符串长度验证
    if (cJSON_IsString(json)) {
        const char* str = json->valuestring;
        size_t len = strlen(str);
        std::string value_str(str);

        if (validation.has_min_length && len < static_cast<size_t>(validation.min_length)) {
            return "Field '" + field_name + "' must be at least " + std::to_string(validation.min_length) + " characters";
        }

        if (validation.has_max_length && len > static_cast<size_t>(validation.max_length)) {
            return "Field '" + field_name + "' must be at most " + std::to_string(validation.max_length) + " characters";
        }

        // 正则表达式验证
        if (validation.has_pattern) {
            try {
                std::regex regex_pattern(validation.pattern);
                if (!std::regex_match(value_str, regex_pattern)) {
                    return "Field '" + field_name + "' does not match the required pattern";
                }
            } catch (const std::regex_error& e) {
                // 正则表达式编译错误，返回详细的错误信息
                return "Field '" + field_name + "' has invalid regex pattern: " + validation.pattern + " (error code: " + std::to_string(e.code()) + ")";
            }
        }

        // 枚举值验证
        if (validation.has_enum) {
            bool found = false;
            for (const auto& enum_val : validation.enum_values) {
                if (value_str == enum_val) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                // 构建允许的值列表
                std::string allowed_values;
                for (size_t i = 0; i < validation.enum_values.size(); ++i) {
                    if (i > 0) allowed_values += ", ";
                    allowed_values += "'" + validation.enum_values[i] + "'";
                }
                return "Field '" + field_name + "' must be one of: " + allowed_values;
            }
        }
    }
    
    // 数值范围验证
    if (cJSON_IsNumber(json)) {
        double value = json->valuedouble;
        
        // 检查最小值
        if (validation.has_min_value && value < validation.min_value) {
            return "Field '" + field_name + "' must be at least " + std::to_string(validation.min_value);
        }
        
        // 检查最大值
        if (validation.has_max_value && value > validation.max_value) {
            return "Field '" + field_name + "' must be at most " + std::to_string(validation.max_value);
        }
    }
    
    return "";
}

// 前置声明
class BodySchemaBase;

// 字段定义
struct FieldDefinition {
    std::string name;
    FieldType type;
    size_t offset;
    FieldValidation validation;
    bool is_optional;  // 是否使用 optional 容器
    
    // 嵌套对象/数组支持
    BodySchemaBase* nested_schema;  // 嵌套对象的 schema
    BodySchemaBase* item_schema;    // 数组元素的 schema
    FieldType element_type;         // 数组元素的类型（仅对 ARRAY 有效）
    ICustomTypeHandler* custom_handler;  // 自定义类型处理器（仅对 CUSTOM 有效）
    
    FieldDefinition(const std::string& n, FieldType t, size_t o)
        : name(n), type(t), offset(o), is_optional(false), 
          nested_schema(nullptr), item_schema(nullptr), 
          element_type(FieldType::STRING), custom_handler(nullptr) {}
};

// Body Schema 基类
class BodySchemaBase {
public:
    virtual ~BodySchemaBase() {}
    virtual std::vector<FieldDefinition> fields() const = 0;
    virtual std::string toJson(void* instance) const = 0;
    virtual bool fromJson(const std::string& json, void* instance) const = 0;
    virtual std::string validate(const cJSON* json) const = 0;
    
    // 整体校验方法（可选实现，如果不实现则使用默认实现）
    virtual std::string validateBody(void* /*instance*/) const {
        // 默认实现：无额外校验，返回空字符串表示通过
        return "";
    }
    
    // 直接验证对象（性能优化：避免序列化再解析）
    virtual std::string validateObject(void* /*instance*/) const {
        // 默认实现：使用序列化+解析方式（子类应该重写此方法以提升性能）
        return "";
    }
};

// ========== DSL 风格的 Body Schema ==========
// 提供更简洁、直观的 API 来定义 Body Schema

// 字段类型定义器
class TypeDefiner {
private:
    FieldType type_;
    
public:
    TypeDefiner(FieldType type) : type_(type) {}
    
    operator FieldType() const { return type_; }
};

// 类型定义常量
namespace types {
    inline TypeDefiner string() { return TypeDefiner(FieldType::STRING); }
    inline TypeDefiner integer() { return TypeDefiner(FieldType::INT); }
    inline TypeDefiner integer64() { return TypeDefiner(FieldType::INT64); }
    inline TypeDefiner number() { return TypeDefiner(FieldType::DOUBLE); }
    inline TypeDefiner floating() { return TypeDefiner(FieldType::FLOAT); }
    inline TypeDefiner boolean() { return TypeDefiner(FieldType::BOOL); }
    inline TypeDefiner array() { return TypeDefiner(FieldType::ARRAY); }
    inline TypeDefiner object() { return TypeDefiner(FieldType::OBJECT); }
}

// 字段构建器 - 提供流畅的 DSL API
class FieldBuilder {
private:
    std::string name_;
    FieldType type_;
    size_t offset_;
    FieldValidation validation_;
    
public:
    FieldBuilder(const std::string& name, FieldType type, size_t offset)
        : name_(name), type_(type), offset_(offset) {}
    
    // 必填字段
    FieldBuilder& required() {
        validation_.required = true;
        return *this;
    }
    
    // 可选字段
    FieldBuilder& optional() {
        validation_.required = false;
        return *this;
    }
    
    // 字符串长度限制
    FieldBuilder& length(int min_len, int max_len) {
        validation_.min_length = min_len;
        validation_.max_length = max_len;
        validation_.has_min_length = true;
        validation_.has_max_length = true;
        return *this;
    }
    
    // 最小长度
    FieldBuilder& minLength(int min_len) {
        validation_.min_length = min_len;
        validation_.has_min_length = true;
        return *this;
    }
    
    // 最大长度
    FieldBuilder& maxLength(int max_len) {
        validation_.max_length = max_len;
        validation_.has_max_length = true;
        return *this;
    }
    
    // 数值范围
    FieldBuilder& range(double min_val, double max_val) {
        validation_.min_value = min_val;
        validation_.max_value = max_val;
        validation_.has_min_value = true;
        validation_.has_max_value = true;
        return *this;
    }
    
    // 最小值
    FieldBuilder& min(double min_val) {
        validation_.min_value = min_val;
        validation_.has_min_value = true;
        return *this;
    }
    
    // 最大值
    FieldBuilder& max(double max_val) {
        validation_.max_value = max_val;
        validation_.has_max_value = true;
        return *this;
    }
    
    // 正则表达式
    FieldBuilder& pattern(const std::string& regex) {
        validation_.pattern = regex;
        validation_.has_pattern = true;
        return *this;
    }
    
    // 枚举值
    FieldBuilder& enumValues(const std::vector<std::string>& values) {
        validation_.enum_values = values;
        validation_.has_enum = true;
        return *this;
    }
    
    // 枚举值（可变参数）
    FieldBuilder& oneOf(const std::string& v1, const std::string& v2 = "", 
                        const std::string& v3 = "", const std::string& v4 = "") {
        std::vector<std::string> values;
        if (!v1.empty()) values.push_back(v1);
        if (!v2.empty()) values.push_back(v2);
        if (!v3.empty()) values.push_back(v3);
        if (!v4.empty()) values.push_back(v4);
        validation_.enum_values = values;
        return *this;
    }
    
    // 使用 optional 容器
    FieldBuilder& useOptional() {
        validation_.use_optional = true;
        return *this;
    }
    
    // 设置嵌套对象的 schema（用于 OBJECT 类型）
    FieldBuilder& nestedSchema(BodySchemaBase* schema) {
        nested_schema_ = schema;
        return *this;
    }
    
    // 设置数组元素的 schema（用于 ARRAY 类型）
    FieldBuilder& itemSchema(BodySchemaBase* schema) {
        item_schema_ = schema;
        return *this;
    }
    
    // 生成 FieldDefinition
    FieldDefinition build() const {
        FieldDefinition def(name_, type_, offset_);
        def.validation = validation_;
        def.is_optional = validation_.use_optional;
        def.nested_schema = nested_schema_;
        def.item_schema = item_schema_;
        return def;
    }
    
private:
    BodySchemaBase* nested_schema_ = nullptr;
    BodySchemaBase* item_schema_ = nullptr;
};

// Schema 构建器
template<typename T>
class SchemaBuilder {
private:
    T* instance_;
    std::vector<FieldDefinition> fields_;
    
public:
    SchemaBuilder() : instance_(nullptr) {}
    
    // 定义字段（简化版本，直接添加）
    void addField(const std::string& name, FieldType type, size_t offset) {
        fields_.push_back(FieldDefinition(name, type, offset));
    }
    
    // 定义字段（使用 FieldBuilder）
    void addField(const FieldBuilder& builder) {
        fields_.push_back(builder.build());
    }
    
    // 获取所有字段
    const std::vector<FieldDefinition>& fields() const {
        return fields_;
    }
};

// 辅助宏：定义字段偏移量
#define FIELD_OFFSET(StructType, FieldName) offsetof(StructType, FieldName)

// DSL 便捷宏（需要在 DslBodySchema<T> 类内部使用）
// 注意：宏中的 T 会被替换为实际的类型名称

// 简化字段定义宏
#define FIELD(name, type, offset) field(createField(name, types::type(), FIELD_OFFSET(T, offset)))

// 简化必填字段定义
#define REQUIRED_FIELD(name, type, offset) FIELD(name, type, offset).required()

// 简化可选字段定义
#define OPTIONAL_FIELD(name, type, offset) FIELD(name, type, offset).optional()

// 字符串字段宏
#define STRING_FIELD(name, offset) string(name, FIELD_OFFSET(T, offset))
#define REQUIRED_STRING(name, offset) STRING_FIELD(name, offset).required()
#define OPTIONAL_STRING(name, offset) STRING_FIELD(name, offset).optional()

// 8位整数字段宏
#define INT8_FIELD(name, offset) int8(name, FIELD_OFFSET(T, offset))
#define REQUIRED_INT8(name, offset) INT8_FIELD(name, offset).required()
#define OPTIONAL_INT8(name, offset) INT8_FIELD(name, offset).optional()

// 16位整数字段宏
#define INT16_FIELD(name, offset) int16(name, FIELD_OFFSET(T, offset))
#define REQUIRED_INT16(name, offset) INT16_FIELD(name, offset).required()
#define OPTIONAL_INT16(name, offset) INT16_FIELD(name, offset).optional()

// 整数字段宏
#define INT_FIELD(name, offset) integer(name, FIELD_OFFSET(T, offset))
#define REQUIRED_INT(name, offset) INT_FIELD(name, offset).required()
#define OPTIONAL_INT(name, offset) INT_FIELD(name, offset).optional()

// 64位整数字段宏
#define INT64_FIELD(name, offset) integer64(name, FIELD_OFFSET(T, offset))
#define REQUIRED_INT64(name, offset) INT64_FIELD(name, offset).required()
#define OPTIONAL_INT64(name, offset) INT64_FIELD(name, offset).optional()

// 8位无符号整数字段宏
#define UINT8_FIELD(name, offset) uint8(name, FIELD_OFFSET(T, offset))
#define REQUIRED_UINT8(name, offset) UINT8_FIELD(name, offset).required()
#define OPTIONAL_UINT8(name, offset) UINT8_FIELD(name, offset).optional()

// 16位无符号整数字段宏
#define UINT16_FIELD(name, offset) uint16(name, FIELD_OFFSET(T, offset))
#define REQUIRED_UINT16(name, offset) UINT16_FIELD(name, offset).required()
#define OPTIONAL_UINT16(name, offset) UINT16_FIELD(name, offset).optional()

// 32位无符号整数字段宏
#define UINT32_FIELD(name, offset) uint32(name, FIELD_OFFSET(T, offset))
#define REQUIRED_UINT32(name, offset) UINT32_FIELD(name, offset).required()
#define OPTIONAL_UINT32(name, offset) UINT32_FIELD(name, offset).optional()

// 64位无符号整数字段宏
#define UINT64_FIELD(name, offset) uint64(name, FIELD_OFFSET(T, offset))
#define REQUIRED_UINT64(name, offset) UINT64_FIELD(name, offset).required()
#define OPTIONAL_UINT64(name, offset) UINT64_FIELD(name, offset).optional()

// 32位浮点数字段宏
#define FP32_FIELD(name, offset) fp32(name, FIELD_OFFSET(T, offset))
#define REQUIRED_FP32(name, offset) FP32_FIELD(name, offset).required()
#define OPTIONAL_FP32(name, offset) FP32_FIELD(name, offset).optional()

// 浮点数字段宏
#define FLOAT_FIELD(name, offset) floating(name, FIELD_OFFSET(T, offset))
#define REQUIRED_FLOAT(name, offset) FLOAT_FIELD(name, offset).required()
#define OPTIONAL_FLOAT(name, offset) FLOAT_FIELD(name, offset).optional()

// 双精度浮点数字段宏
#define NUMBER_FIELD(name, offset) number(name, FIELD_OFFSET(T, offset))
#define REQUIRED_NUMBER(name, offset) NUMBER_FIELD(name, offset).required()
#define OPTIONAL_NUMBER(name, offset) NUMBER_FIELD(name, offset).optional()

// 布尔字段宏
#define BOOL_FIELD(name, offset) boolean(name, FIELD_OFFSET(T, offset))
#define REQUIRED_BOOL(name, offset) BOOL_FIELD(name, offset).required()
#define OPTIONAL_BOOL(name, offset) BOOL_FIELD(name, offset).optional()

// 日期字段宏（YYYY-MM-DD）
#define DATE_FIELD(name, offset) date(name, FIELD_OFFSET(T, offset))
#define REQUIRED_DATE(name, offset) DATE_FIELD(name, offset).required()
#define OPTIONAL_DATE(name, offset) DATE_FIELD(name, offset).optional()

// 日期时间字段宏（YYYY-MM-DD HH:MM:SS）
#define DATETIME_FIELD(name, offset) datetime(name, FIELD_OFFSET(T, offset))
#define REQUIRED_DATETIME(name, offset) DATETIME_FIELD(name, offset).required()
#define OPTIONAL_DATETIME(name, offset) DATETIME_FIELD(name, offset).optional()

// 邮箱字段宏
#define EMAIL_FIELD(name, offset) email(name, FIELD_OFFSET(T, offset))
#define REQUIRED_EMAIL(name, offset) EMAIL_FIELD(name, offset).required()
#define OPTIONAL_EMAIL(name, offset) EMAIL_FIELD(name, offset).optional()

// URL 字段宏
#define URL_FIELD(name, offset) url(name, FIELD_OFFSET(T, offset))
#define REQUIRED_URL(name, offset) URL_FIELD(name, offset).required()
#define OPTIONAL_URL(name, offset) URL_FIELD(name, offset).optional()

// UUID 字段宏
#define UUID_FIELD(name, offset) uuid(name, FIELD_OFFSET(T, offset))
#define REQUIRED_UUID(name, offset) UUID_FIELD(name, offset).required()
#define OPTIONAL_UUID(name, offset) UUID_FIELD(name, offset).optional()

// 使用 std::optional 的可选字段宏
#define OPTIONAL_STRING_OPT(name, offset) STRING_FIELD(name, offset).optional().useOptional()
#define OPTIONAL_INT_OPT(name, offset) INT_FIELD(name, offset).optional().useOptional()
#define OPTIONAL_NUMBER_OPT(name, offset) NUMBER_FIELD(name, offset).optional().useOptional()
#define OPTIONAL_BOOL_OPT(name, offset) BOOL_FIELD(name, offset).optional().useOptional()

// 对象字段宏（嵌套对象）
#define OBJECT_FIELD(name, offset) object(name, FIELD_OFFSET(T, offset))
#define REQUIRED_OBJECT(name, offset) OBJECT_FIELD(name, offset).required()
#define OPTIONAL_OBJECT(name, offset) OBJECT_FIELD(name, offset).optional()
#define OPTIONAL_OBJECT_OPT(name, offset) OBJECT_FIELD(name, offset).optional().useOptional()

// 数组字段宏
#define ARRAY_FIELD(name, offset) array(name, FIELD_OFFSET(T, offset))
#define REQUIRED_ARRAY(name, offset) ARRAY_FIELD(name, offset).required()
#define OPTIONAL_ARRAY(name, offset) ARRAY_FIELD(name, offset).optional()
#define OPTIONAL_ARRAY_OPT(name, offset) ARRAY_FIELD(name, offset).optional().useOptional()

// Body 序列化辅助函数
template<typename T>
std::string toJson(const T& instance) {
    BodySchemaBase* schema = instance.schema();
    if (!schema) {
        std::cerr << "Error: Schema not defined" << std::endl;
        return "{}";
    }
    return schema->toJson(reinterpret_cast<void*>(&instance));
}

// 重载：直接返回字符串
inline std::string toJson(const char* json_str) {
    return std::string(json_str);
}

// 重载：直接返回字符串
inline std::string toJson(const std::string& json_str) {
    return json_str;
}

// Body Schema 模板基类（传统方式）
template<typename T>
class BodySchema : public BodySchemaBase {
public:
    // 子类必须实现这个方法来定义字段
    virtual void defineFields() = 0;
    
    // 获取字段定义
    std::vector<FieldDefinition> fields() const override {
        return field_definitions_;
    }
    
protected:
    std::vector<FieldDefinition> field_definitions_;
    
    // 添加字段（返回 FieldDefinition 引用以便链式设置）
    FieldDefinition& addField(const std::string& name, FieldType type, size_t offset) {
        field_definitions_.push_back(FieldDefinition(name, type, offset));
        return field_definitions_.back();
    }
};

// DSL 风格的 Body Schema 基类
template<typename T>
class DslBodySchema : public BodySchemaBase {
public:
    // 子类必须实现这个方法来定义字段
    virtual void define() = 0;
    
    // 获取字段定义
    std::vector<FieldDefinition> fields() const override {
        // 懒加载：首次调用时调用 define() 方法
        if (!defined_) {
            defined_ = true;
            // 调用子类的 define() 方法来注册字段
            const_cast<DslBodySchema*>(this)->define();
        }
        return builder_.fields();
    }
    
    // 序列化
    std::string toJson(void* instance) const override {
        cJSON* json = cJSON_CreateObject();
        if (!json) return "{}";
        
        for (const auto& field : fields()) {
            // 对于可选字段，检查是否有有效值
            if (!field.validation.required && !hasValidValue(instance, field.offset, field.type)) {
                continue; // 跳过无有效值的可选字段
            }
            
            // 对于 std::optional 字段，检查是否有值
            if (field.is_optional && !hasOptionalValue(instance, field.offset, field.type)) {
                continue; // 跳过空的 std::optional 字段
            }
            
            cJSON* field_json;
            if (field.is_optional) {
                field_json = createOptionalJsonField(instance, field.offset, field.type);
            } else {
                field_json = createJsonField(instance, field.offset, field.type);
            }
            
            if (field_json) {
                cJSON_AddItemToObject(json, field.name.c_str(), field_json);
            }
        }
        
        char* json_str = cJSON_Print(json);
        std::string result(json_str ? json_str : "{}");
        if (json_str) free(json_str);
        cJSON_Delete(json);
        
        return result;
    }
    
    // 反序列化
    bool fromJson(const std::string& json_str, void* instance) const override {
        cJSON* json = cJSON_Parse(json_str.c_str());
        if (!json || !cJSON_IsObject(json)) {
            if (json) cJSON_Delete(json);
            return false;
        }
        
        for (const auto& field : fields()) {
            cJSON* field_json = cJSON_GetObjectItem(json, field.name.c_str());
            if (field_json) {
                if (field.is_optional) {
                    setOptionalValue(instance, field.offset, field.type, field_json);
                } else {
                    setFieldValue(instance, field.offset, field.type, field_json);
                }
            } else if (!field.validation.required) {
                // 可选字段：如果 JSON 中不存在，设置默认值
                if (field.is_optional) {
                    // 对于 std::optional 字段，设置为空值
                    clearOptionalValue(instance, field.offset, field.type);
                } else {
                    setDefaultValue(instance, field.offset, field.type);
                }
            }
        }
        
        cJSON_Delete(json);
        return true;
    }
    
    // 验证
    std::string validate(const cJSON* json) const override {
        if (!cJSON_IsObject(json)) {
            return "Request body must be a JSON object";
        }
        
        // 检查必填字段
        for (const auto& field : fields()) {
            if (field.validation.required) {
                cJSON* field_json = cJSON_GetObjectItem(json, field.name.c_str());
                if (!field_json) {
                    return "Field '" + field.name + "' is required";
                }
            }
        }
        
        // 验证每个字段的值
        for (const auto& field : fields()) {
            cJSON* field_json = cJSON_GetObjectItem(json, field.name.c_str());
            
            // 如果字段不存在且不是必填的，跳过验证
            if (!field_json && !field.validation.required) {
                continue;
            }
            
            // 如果字段不存在但是必填的，已经在前面检查过了
            if (!field_json) {
                continue;
            }
            
            // 执行值验证（长度、范围等）
            std::string error = applyValidation(field_json, field.validation, field.name);
            if (!error.empty()) {
                return error;
            }
        }
        
        return "";
    }
    
protected:
    SchemaBuilder<T> builder_;
    mutable bool defined_ = false;  // 标记是否已调用 define() 方法
    
    // ========== 简化的字段定义方法 ==========
    
    // 定义字符串字段
    FieldBuilder string(const std::string& name, size_t offset) {
        return FieldBuilder(name, FieldType::STRING, offset);
    }
    
    // 定义整数字段
    FieldBuilder integer(const std::string& name, size_t offset) {
        return FieldBuilder(name, FieldType::INT, offset);
    }
    
    // 定义整数字段（64位）
    FieldBuilder integer64(const std::string& name, size_t offset) {
        return FieldBuilder(name, FieldType::INT64, offset);
    }
    
    // 定义浮点数字段
    FieldBuilder number(const std::string& name, size_t offset) {
        return FieldBuilder(name, FieldType::DOUBLE, offset);
    }
    
    // 定义浮点数字段（float）
    FieldBuilder floating(const std::string& name, size_t offset) {
        return FieldBuilder(name, FieldType::FLOAT, offset);
    }
    
    // 定义布尔字段
    FieldBuilder boolean(const std::string& name, size_t offset) {
        return FieldBuilder(name, FieldType::BOOL, offset);
    }
    
    // 定义对象字段（嵌套对象）
    FieldBuilder object(const std::string& name, size_t offset) {
        return FieldBuilder(name, FieldType::OBJECT, offset);
    }
    
    // 定义数组字段
    FieldBuilder array(const std::string& name, size_t offset) {
        return FieldBuilder(name, FieldType::ARRAY, offset);
    }
    
    // 定义字段（简化版本）
    void field(const std::string& name, FieldType type, size_t offset) {
        builder_.addField(name, type, offset);
    }
    
    // 定义字段（使用 FieldBuilder）
    void field(const FieldBuilder& builder) {
        builder_.addField(builder);
    }
    
    // 创建字段构建器
    FieldBuilder createField(const std::string& name, FieldType type, size_t offset) {
        return FieldBuilder(name, type, offset);
    }
    
    // ========== 超简化 DSL 方法 ==========
    // 提供最简洁的语法，但需要手动指定字段名和偏移量
    
    // 必填字段
    FieldBuilder required(const std::string& name) {
        return FieldBuilder(name, FieldType::STRING, 0).required();
    }
    
    // 可选字段
    FieldBuilder optional(const std::string& name) {
        return FieldBuilder(name, FieldType::STRING, 0).optional();
    }
    
private:
    // 创建 JSON 字段
    static cJSON* createJsonField(void* instance, size_t offset, FieldType type) {
        if (!instance) return nullptr;
        
        char* field_ptr = static_cast<char*>(instance) + offset;
        
        switch (type) {
            case FieldType::STRING:
            case FieldType::DATE:
            case FieldType::DATETIME:
            case FieldType::EMAIL:
            case FieldType::URL:
            case FieldType::UUID:
                return cJSON_CreateString(reinterpret_cast<std::string*>(field_ptr)->c_str());
            case FieldType::INT8:
                return cJSON_CreateNumber(static_cast<int>(*reinterpret_cast<int8_t*>(field_ptr)));
            case FieldType::INT16:
                return cJSON_CreateNumber(static_cast<int>(*reinterpret_cast<int16_t*>(field_ptr)));
            case FieldType::INT:
                return cJSON_CreateNumber(*reinterpret_cast<int*>(field_ptr));
            case FieldType::INT64:
                return cJSON_CreateNumber(static_cast<double>(*reinterpret_cast<int64_t*>(field_ptr)));
            case FieldType::UINT8:
                return cJSON_CreateNumber(static_cast<int>(*reinterpret_cast<uint8_t*>(field_ptr)));
            case FieldType::UINT16:
                return cJSON_CreateNumber(static_cast<int>(*reinterpret_cast<uint16_t*>(field_ptr)));
            case FieldType::UINT32:
                return cJSON_CreateNumber(static_cast<double>(*reinterpret_cast<uint32_t*>(field_ptr)));
            case FieldType::UINT64:
                return cJSON_CreateNumber(static_cast<double>(*reinterpret_cast<uint64_t*>(field_ptr)));
            case FieldType::FP32:
            case FieldType::FLOAT:
                return cJSON_CreateNumber(*reinterpret_cast<float*>(field_ptr));
            case FieldType::DOUBLE:
                return cJSON_CreateNumber(*reinterpret_cast<double*>(field_ptr));
            case FieldType::BOOL:
                return cJSON_CreateBool(*reinterpret_cast<bool*>(field_ptr));
            case FieldType::ARRAY:
            case FieldType::OBJECT:
            case FieldType::CUSTOM:
                return cJSON_CreateNull();
        }
        
        return nullptr;
    }
    
    // 创建 std::optional JSON 字段
    static cJSON* createOptionalJsonField(void* instance, size_t offset, FieldType type) {
        if (!instance) return nullptr;
        
        char* field_ptr = static_cast<char*>(instance) + offset;
        
        switch (type) {
            case FieldType::STRING: {
                auto& opt = *reinterpret_cast<uvapi::optional<std::string>*>(field_ptr);
                if (opt.has_value()) {
                    return cJSON_CreateString(opt->c_str());
                }
                break;
            }
            case FieldType::INT: {
                auto& opt = *reinterpret_cast<uvapi::optional<int>*>(field_ptr);
                if (opt.has_value()) {
                    return cJSON_CreateNumber(*opt);
                }
                break;
            }
            case FieldType::INT64: {
                auto& opt = *reinterpret_cast<uvapi::optional<int64_t>*>(field_ptr);
                if (opt.has_value()) {
                    return cJSON_CreateNumber(static_cast<double>(*opt));
                }
                break;
            }
            case FieldType::FLOAT: {
                auto& opt = *reinterpret_cast<uvapi::optional<float>*>(field_ptr);
                if (opt.has_value()) {
                    return cJSON_CreateNumber(*opt);
                }
                break;
            }
            case FieldType::DOUBLE: {
                auto& opt = *reinterpret_cast<uvapi::optional<double>*>(field_ptr);
                if (opt.has_value()) {
                    return cJSON_CreateNumber(*opt);
                }
                break;
            }
            case FieldType::BOOL: {
                auto& opt = *reinterpret_cast<uvapi::optional<bool>*>(field_ptr);
                if (opt.has_value()) {
                    return cJSON_CreateBool(*opt);
                }
                break;
            }
            case FieldType::ARRAY:
            case FieldType::OBJECT:
            case FieldType::CUSTOM:
                return cJSON_CreateNull();
        }
        
        return nullptr;
    }
    
    // 设置字段值
    static void setFieldValue(void* instance, size_t offset, FieldType type, const cJSON* json) {
        if (!instance || !json) return;
        
        char* field_ptr = static_cast<char*>(instance) + offset;
        
        switch (type) {
            case FieldType::STRING:
            case FieldType::DATE:
            case FieldType::DATETIME:
            case FieldType::EMAIL:
            case FieldType::URL:
            case FieldType::UUID:
                if (cJSON_IsString(json)) {
                    *reinterpret_cast<std::string*>(field_ptr) = json->valuestring;
                }
                break;
            case FieldType::INT8:
                if (cJSON_IsNumber(json)) {
                    *reinterpret_cast<int8_t*>(field_ptr) = static_cast<int8_t>(json->valuedouble);
                }
                break;
            case FieldType::INT16:
                if (cJSON_IsNumber(json)) {
                    *reinterpret_cast<int16_t*>(field_ptr) = static_cast<int16_t>(json->valuedouble);
                }
                break;
            case FieldType::INT:
                if (cJSON_IsNumber(json)) {
                    *reinterpret_cast<int*>(field_ptr) = static_cast<int>(json->valuedouble);
                }
                break;
            case FieldType::INT64:
                if (cJSON_IsNumber(json)) {
                    *reinterpret_cast<int64_t*>(field_ptr) = static_cast<int64_t>(json->valuedouble);
                }
                break;
            case FieldType::UINT8:
                if (cJSON_IsNumber(json)) {
                    *reinterpret_cast<uint8_t*>(field_ptr) = static_cast<uint8_t>(json->valuedouble);
                }
                break;
            case FieldType::UINT16:
                if (cJSON_IsNumber(json)) {
                    *reinterpret_cast<uint16_t*>(field_ptr) = static_cast<uint16_t>(json->valuedouble);
                }
                break;
            case FieldType::UINT32:
                if (cJSON_IsNumber(json)) {
                    *reinterpret_cast<uint32_t*>(field_ptr) = static_cast<uint32_t>(json->valuedouble);
                }
                break;
            case FieldType::UINT64:
                if (cJSON_IsNumber(json)) {
                    *reinterpret_cast<uint64_t*>(field_ptr) = static_cast<uint64_t>(json->valuedouble);
                }
                break;
            case FieldType::FP32:
            case FieldType::FLOAT:
                if (cJSON_IsNumber(json)) {
                    *reinterpret_cast<float*>(field_ptr) = static_cast<float>(json->valuedouble);
                }
                break;
            case FieldType::DOUBLE:
                if (cJSON_IsNumber(json)) {
                    *reinterpret_cast<double*>(field_ptr) = json->valuedouble;
                }
                break;
            case FieldType::BOOL:
                if (cJSON_IsBool(json)) {
                    *reinterpret_cast<bool*>(field_ptr) = json->type == cJSON_True;
                }
                break;
            case FieldType::ARRAY:
            case FieldType::OBJECT:
            case FieldType::CUSTOM:
                break;
        }
    }
    
    // 设置字段默认值
    static void setDefaultValue(void* instance, size_t offset, FieldType type) {
        if (!instance) return;
        
        char* field_ptr = static_cast<char*>(instance) + offset;
        
        switch (type) {
            case FieldType::STRING:
            case FieldType::DATE:
            case FieldType::DATETIME:
            case FieldType::EMAIL:
            case FieldType::URL:
            case FieldType::UUID:
                *reinterpret_cast<std::string*>(field_ptr) = "";
                break;
            case FieldType::INT8:
                *reinterpret_cast<int8_t*>(field_ptr) = 0;
                break;
            case FieldType::INT16:
                *reinterpret_cast<int16_t*>(field_ptr) = 0;
                break;
            case FieldType::INT:
                *reinterpret_cast<int*>(field_ptr) = 0;
                break;
            case FieldType::INT64:
                *reinterpret_cast<int64_t*>(field_ptr) = 0;
                break;
            case FieldType::UINT8:
                *reinterpret_cast<uint8_t*>(field_ptr) = 0;
                break;
            case FieldType::UINT16:
                *reinterpret_cast<uint16_t*>(field_ptr) = 0;
                break;
            case FieldType::UINT32:
                *reinterpret_cast<uint32_t*>(field_ptr) = 0;
                break;
            case FieldType::UINT64:
                *reinterpret_cast<uint64_t*>(field_ptr) = 0;
                break;
            case FieldType::FP32:
            case FieldType::FLOAT:
                *reinterpret_cast<float*>(field_ptr) = 0.0f;
                break;
            case FieldType::DOUBLE:
                *reinterpret_cast<double*>(field_ptr) = 0.0;
                break;
            case FieldType::BOOL:
                *reinterpret_cast<bool*>(field_ptr) = false;
                break;
            case FieldType::ARRAY:
            case FieldType::OBJECT:
            case FieldType::CUSTOM:
                break;
        }
    }
    
    // 检查字段是否有有效值（用于可选字段）
    static bool hasValidValue(void* instance, size_t offset, FieldType type) {
        if (!instance) return false;
        
        char* field_ptr = static_cast<char*>(instance) + offset;
        
        switch (type) {
            case FieldType::STRING:
            case FieldType::DATE:
            case FieldType::DATETIME:
            case FieldType::EMAIL:
            case FieldType::URL:
            case FieldType::UUID: {
                const std::string& str = *reinterpret_cast<std::string*>(field_ptr);
                return !str.empty();
            }
            case FieldType::INT8:
                return *reinterpret_cast<int8_t*>(field_ptr) != 0;
            case FieldType::INT16:
                return *reinterpret_cast<int16_t*>(field_ptr) != 0;
            case FieldType::INT:
                return *reinterpret_cast<int*>(field_ptr) != 0;
            case FieldType::INT64:
                return *reinterpret_cast<int64_t*>(field_ptr) != 0;
            case FieldType::UINT8:
                return *reinterpret_cast<uint8_t*>(field_ptr) != 0;
            case FieldType::UINT16:
                return *reinterpret_cast<uint16_t*>(field_ptr) != 0;
            case FieldType::UINT32:
                return *reinterpret_cast<uint32_t*>(field_ptr) != 0;
            case FieldType::UINT64:
                return *reinterpret_cast<uint64_t*>(field_ptr) != 0;
            case FieldType::FP32:
            case FieldType::FLOAT:
                return *reinterpret_cast<float*>(field_ptr) != 0.0f;
            case FieldType::DOUBLE:
                return *reinterpret_cast<double*>(field_ptr) != 0.0;
            case FieldType::BOOL:
                return *reinterpret_cast<bool*>(field_ptr) == true;
            case FieldType::ARRAY:
            case FieldType::OBJECT:
            case FieldType::CUSTOM:
                return false; // 复杂类型默认不认为是有效值
        }
        
        return false;
    }
    
    // 设置 std::optional 字段的值
    static void setOptionalValue(void* instance, size_t offset, FieldType type, const cJSON* json) {
        if (!instance || !json) return;
        
        char* field_ptr = static_cast<char*>(instance) + offset;
        
        switch (type) {
            case FieldType::STRING:
                if (cJSON_IsString(json)) {
                    *reinterpret_cast<uvapi::optional<std::string>*>(field_ptr) = json->valuestring;
                }
                break;
            case FieldType::INT:
                if (cJSON_IsNumber(json)) {
                    *reinterpret_cast<uvapi::optional<int>*>(field_ptr) = static_cast<int>(json->valuedouble);
                }
                break;
            case FieldType::INT64:
                if (cJSON_IsNumber(json)) {
                    *reinterpret_cast<uvapi::optional<int64_t>*>(field_ptr) = static_cast<int64_t>(json->valuedouble);
                }
                break;
            case FieldType::FLOAT:
                if (cJSON_IsNumber(json)) {
                    *reinterpret_cast<uvapi::optional<float>*>(field_ptr) = static_cast<float>(json->valuedouble);
                }
                break;
            case FieldType::DOUBLE:
                if (cJSON_IsNumber(json)) {
                    *reinterpret_cast<uvapi::optional<double>*>(field_ptr) = json->valuedouble;
                }
                break;
            case FieldType::BOOL:
                if (cJSON_IsBool(json)) {
                    *reinterpret_cast<uvapi::optional<bool>*>(field_ptr) = json->type == cJSON_True;
                }
                break;
            case FieldType::ARRAY:
            case FieldType::OBJECT:
            case FieldType::CUSTOM:
                break;
        }
    }
    
    // 清除 std::optional 字段的值（设置为空）
    static void clearOptionalValue(void* instance, size_t offset, FieldType type) {
        if (!instance) return;
        
        char* field_ptr = static_cast<char*>(instance) + offset;
        
        switch (type) {
            case FieldType::STRING:
                reinterpret_cast<uvapi::optional<std::string>*>(field_ptr)->reset();
                break;
            case FieldType::INT:
                reinterpret_cast<uvapi::optional<int>*>(field_ptr)->reset();
                break;
            case FieldType::INT64:
                reinterpret_cast<uvapi::optional<int64_t>*>(field_ptr)->reset();
                break;
            case FieldType::FLOAT:
                reinterpret_cast<uvapi::optional<float>*>(field_ptr)->reset();
                break;
            case FieldType::DOUBLE:
                reinterpret_cast<uvapi::optional<double>*>(field_ptr)->reset();
                break;
            case FieldType::BOOL:
                reinterpret_cast<uvapi::optional<bool>*>(field_ptr)->reset();
                break;
            case FieldType::ARRAY:
            case FieldType::OBJECT:
            case FieldType::CUSTOM:
                break;
        }
    }
    
    // 检查 std::optional 字段是否有值
    static bool hasOptionalValue(void* instance, size_t offset, FieldType type) {
        if (!instance) return false;
        
        char* field_ptr = static_cast<char*>(instance) + offset;
        
        switch (type) {
            case FieldType::STRING:
                return reinterpret_cast<uvapi::optional<std::string>*>(field_ptr)->has_value();
            case FieldType::INT:
                return reinterpret_cast<uvapi::optional<int>*>(field_ptr)->has_value();
            case FieldType::INT64:
                return reinterpret_cast<uvapi::optional<int64_t>*>(field_ptr)->has_value();
            case FieldType::FLOAT:
                return reinterpret_cast<uvapi::optional<float>*>(field_ptr)->has_value();
            case FieldType::DOUBLE:
                return reinterpret_cast<uvapi::optional<double>*>(field_ptr)->has_value();
            case FieldType::BOOL:
                return reinterpret_cast<uvapi::optional<bool>*>(field_ptr)->has_value();
            case FieldType::ARRAY:
            case FieldType::OBJECT:
            case FieldType::CUSTOM:
                return false;
        }
        
        return false;
    }
    
    // 直接验证对象（性能优化：避免序列化再解析）
    std::string validateObject(void* instance) const override {
        // 1. 字段级校验（直接从对象读取）
        for (const auto& field : fields()) {
            // 获取字段值（字符串形式）
            std::string field_value = getFieldValueAsString(instance, field.offset, field.type);
            
            // 检查必填字段
            if (field.validation.required && field_value.empty()) {
                return "Field '" + field.name + "' is required";
            }
            
            // 如果字段为空且不是必填的，跳过验证
            if (field_value.empty()) continue;
            
            // 验证字段值
            std::string error = applyValidationOnString(field_value, field.validation, field.name, field.type);
            if (!error.empty()) {
                return error;
            }
        }
        
        // 2. 整体校验
        return validateBody(instance);
    }
    
private:
    // 获取字段值（字符串形式）
    static std::string getFieldValueAsString(void* instance, size_t offset, FieldType type) {
        if (!instance) return "";
        
        char* field_ptr = static_cast<char*>(instance) + offset;
        
        switch (type) {
            case FieldType::STRING:
                return *reinterpret_cast<std::string*>(field_ptr);
            case FieldType::INT:
                return std::to_string(*reinterpret_cast<int*>(field_ptr));
            case FieldType::INT64:
                return std::to_string(*reinterpret_cast<int64_t*>(field_ptr));
            case FieldType::FLOAT:
                return std::to_string(*reinterpret_cast<float*>(field_ptr));
            case FieldType::DOUBLE:
                return std::to_string(*reinterpret_cast<double*>(field_ptr));
            case FieldType::BOOL:
                return *reinterpret_cast<bool*>(field_ptr) ? "true" : "false";
            case FieldType::ARRAY:
            case FieldType::OBJECT:
            case FieldType::CUSTOM:
                return "";
        }
        
        return "";
    }
    
    // 对字符串值应用验证规则
    static std::string applyValidationOnString(const std::string& value, 
                                                const FieldValidation& validation,
                                                const std::string& field_name,
                                                FieldType type) {
        size_t len = value.length();
        
        // 字符串类型验证
        if (type == FieldType::STRING) {
            if (validation.has_min_length && len < validation.min_length) {
                return "Field '" + field_name + "' must be at least " + 
                       std::to_string(validation.min_length) + " characters";
            }
            
            if (validation.has_max_length && len > validation.max_length) {
                return "Field '" + field_name + "' must be at most " + 
                       std::to_string(validation.max_length) + " characters";
            }
            
            // 枚举验证
            if (!validation.enum_values.empty()) {
                bool found = false;
                for (size_t i = 0; i < validation.enum_values.size(); ++i) {
                    if (validation.enum_values[i] == value) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return "Field '" + field_name + "' must be one of: " + 
                           joinEnumValues(validation.enum_values);
                }
            }
        }
        // 数值类型验证
        else if (type == FieldType::INT || type == FieldType::INT64 ||
                 type == FieldType::FLOAT || type == FieldType::DOUBLE) {
            double num = 0.0;
            try {
                if (type == FieldType::INT) {
                    num = static_cast<double>(std::stoi(value));
                } else if (type == FieldType::INT64) {
                    num = static_cast<double>(std::stoll(value));
                } else if (type == FieldType::FLOAT) {
                    num = static_cast<double>(std::stof(value));
                } else {
                    num = std::stod(value);
                }
            } catch (...) {
                return "Field '" + field_name + "' must be a valid number";
            }
            
            if (validation.has_min_value && num < validation.min_value) {
                return "Field '" + field_name + "' must be at least " + 
                       std::to_string(validation.min_value);
            }
            
            if (validation.has_max_value && num > validation.max_value) {
                return "Field '" + field_name + "' must be at most " + 
                       std::to_string(validation.max_value);
            }
        }
        
        return "";
    }
    
    // 连接枚举值
    static std::string joinEnumValues(const std::vector<std::string>& values) {
        if (values.empty()) return "";
        
        std::string result = "'";
        for (size_t i = 0; i < values.size(); ++i) {
            if (i > 0) result += "', '";
            result += values[i];
        }
        result += "'";
        
        return result;
    }
};

// Body 反序列化辅助函数（只执行解析，不自动校验）
template<typename T>
T parseBody(const std::string& json) {
    T instance;
    BodySchemaBase* schema = instance.schema();
    if (!schema) {
        std::cerr << "Error: Schema not defined" << std::endl;
        return instance;
    }
    
    // 验证 JSON 格式
    cJSON* root = cJSON_Parse(json.c_str());
    if (!root) {
        std::cerr << "Error: Invalid JSON format" << std::endl;
        return instance;
    }
    cJSON_Delete(root);
    
    // 反序列化
    if (!schema->fromJson(json, &instance)) {
        std::cerr << "Error: Failed to parse body" << std::endl;
        return instance;
    }
    
    return instance;
}

// 显式校验函数（性能优化：直接验证对象）
template<typename T>
ValidationResult validateRequest(const T& instance) {
    BodySchemaBase* schema = instance.schema();
    if (!schema) {
        return "Schema not defined";
    }
    
    // 直接验证对象（避免序列化再解析）
    std::string error = schema->validateObject(const_cast<T*>(&instance));
    if (!error.empty()) {
        return error;
    }
    
    return true;  // 隐式转换为成功的 ValidationResult
}

// HTTP 方法枚举（与 uvhttp_method_t 对应）
enum class HttpMethod {
    ANY = 0,
    GET = 1,
    POST = 2,
    PUT = 3,
    DELETE = 4,
    HEAD = 5,
    OPTIONS = 6,
    PATCH = 7
};

// 安全的 HTTP 方法转换函数
inline uvhttp_method_t toUvhttpMethod(HttpMethod method) {
    return static_cast<uvhttp_method_t>(static_cast<int>(method));
}

// HTTP 响应
struct HttpResponse {
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;
    
    HttpResponse(int code = 200) : status_code(code) {}
    
    HttpResponse(int code, const std::string& body_text) : status_code(code), body(body_text) {}
    
    HttpResponse(int code, const char* body_text) : status_code(code), body(body_text ? body_text : "") {}
    
    HttpResponse& header(const std::string& key, const std::string& value) {
        headers[key] = value;
        return *this;
    }
    
    HttpResponse& setBody(const std::string& body_text) {
        body = body_text;
        return *this;
    }
    
    HttpResponse& json(const std::string& json_body) {
        body = json_body;
        headers["Content-Type"] = "application/json";
        return *this;
    }
    
    // 自动序列化
    template<typename T>
    HttpResponse& json(const T& instance) {
        body = uvapi::toJson(instance);
        headers["Content-Type"] = "application/json";
        return *this;
    }
};

// 参数值包装类 - 支持自动类型推导
class ParamValue {
private:
    std::string value_;
    bool has_value_;
    
public:
    ParamValue() : value_(""), has_value_(false) {}
    explicit ParamValue(const std::string& value) : value_(value), has_value_(true) {}
    
    // 检查是否有值
    bool hasValue() const { return has_value_; }
    
    // 检查是否为空
    bool empty() const { return value_.empty(); }
    
    // 隐式类型转换运算符 - 支持自动类型推导
    template<typename T>
    operator T() const {
        return parseValue<T>(value_);
    }
    
private:
    // 辅助函数：安全地解析字符串值为指定类型
    template<typename T>
    static T parseValue(const std::string& value) {
        // 布尔类型（必须在整数类型之前检查）
        if (std::is_same<T, bool>::value) {
            std::string lower = value;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            return (lower == "true" || lower == "1" || lower == "yes" || lower == "on");
        }
        
        // 字符串类型
        if (std::is_same<T, std::string>::value) {
            return value;
        }
        
        // 整数类型（排除 bool，因为 bool 已经在上面处理了）
        if (std::is_integral<T>::value) {
            if (value.empty()) {
                return T();
            }
            
            errno = 0;
            char* endptr = nullptr;
            long long result = strtoll(value.c_str(), &endptr, 10);
            
            // 检查转换是否失败
            if (endptr == value.c_str() || *endptr != '\0') {
                return T();
            }
            
            // 检查是否溢出
            if (errno == ERANGE) {
                return T();
            }
            
            // 检查是否超出目标类型的范围
            if (result < static_cast<long long>(std::numeric_limits<T>::min()) ||
                result > static_cast<long long>(std::numeric_limits<T>::max())) {
                return T();
            }
            
            return static_cast<T>(result);
        }
        
        // 浮点数类型
        if (std::is_floating_point<T>::value) {
            if (value.empty()) {
                return T();
            }
            
            errno = 0;
            char* endptr = nullptr;
            double result = strtod(value.c_str(), &endptr);
            
            // 检查转换是否失败
            if (endptr == value.c_str() || *endptr != '\0') {
                return T();
            }
            
            // 检查是否溢出
            if (errno == ERANGE) {
                return T();
            }
            
            // 检查是否超出目标类型的范围
            if (result < static_cast<double>(std::numeric_limits<T>::min()) ||
                result > static_cast<double>(std::numeric_limits<T>::max())) {
                return T();
            }
            
            return static_cast<T>(result);
        }
        
        // 默认返回默认构造的值
        return T();
    }
};

// 参数访问器
class ParamAccessor {
private:
    const std::map<std::string, std::string>& params_;
    
public:
    explicit ParamAccessor(const std::map<std::string, std::string>& params)
        : params_(params) {}
    
    // operator[] - 返回 ParamValue，支持自动类型推导
    ParamValue operator[](const std::string& key) const {
        auto it = params_.find(key);
        if (it != params_.end()) {
            return ParamValue(it->second);
        }
        return ParamValue();  // 无值
    }
    
    // 检查参数是否存在
    bool has(const std::string& key) const {
        return params_.find(key) != params_.end();
    }
};

// HTTP 请求
struct HttpRequest {
    HttpMethod method;
    std::string url_path;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> query_params;
    std::map<std::string, std::string> path_params;
    std::string body;
    int64_t user_id;
    
    // 参数访问器（使用 operator[] 自动类型推导）
    ParamAccessor pathParam{path_params};
    ParamAccessor queryParam{query_params};
    
    // Request Body - 返回 optional<T>
    // 框架会根据 Schema 验证并解析 Body
    template<typename T>
    optional<T> parseBody() const {
        if (body.empty()) {
            return optional<T>();
        }
        T result = uvapi::parseBody<T>(body);
        return optional<T>(result);
    }

private:
    // Body 反序列化
    template<typename T>
    T parseBody() const {
        return uvapi::parseBody<T>(body);
    }
    
    std::string getHeader(const std::string& key) const;
    bool isAuthenticated() const;
};

// ========== Server 层：底层 HTTP 服务器 ==========
namespace server {

// 前向声明
// ========== TLS 配置结构 ==========
struct TlsConfig {
    bool enabled;
    std::string cert_file;      // 证书文件路径
    std::string key_file;       // 私钥文件路径
    std::string ca_file;        // CA 证书文件路径（可选，用于客户端认证）
    
    TlsConfig() : enabled(false) {}
    
    TlsConfig(const std::string& cert, const std::string& key) 
        : enabled(true), cert_file(cert), key_file(key) {}
    
    TlsConfig(const std::string& cert, const std::string& key, const std::string& ca)
        : enabled(true), cert_file(cert), key_file(key), ca_file(ca) {}
};

class Server;

// 前向声明
int on_uvhttp_request(uvhttp_request_t* req, uvhttp_response_t* resp);

class Server {
public:
    /**
     * @brief 构造 Server 实例
     * 
     * ## 设计哲学
     * 
     * 1. **零异常 (Zero Exceptions)**: 不使用异常机制
     *    - 所有公共方法返回 bool 表示成功/失败
     *    - 通过 uvhttp_error_t 获取详细错误信息
     *    - RAII 确保资源自动释放
     * 
     * 2. **事件循环注入**: 支持多实例共享循环
     * 
     * 3. **RAII 资源管理**: 自动清理，无需手动释放
     */
    Server(uv_loop_t* loop);  // 事件循环注入
    ~Server();
    
    // 禁止拷贝
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    
    // 允许移动
    Server(Server&& other) noexcept;
    Server& operator=(Server&& other) noexcept;
    
    // 配置 TLS/SSL
    void enableTls(const TlsConfig& tls_config);
    
    // 启动监听
    bool listen(const std::string& host, int port);
    bool listenHttps(const std::string& host, int port);
    
    void stop();
    uv_loop_t* getLoop() const { return loop_; }
    
    // 路由注册（供上层 API 使用）
    void addRoute(const std::string& path, HttpMethod method, 
                  std::function<HttpResponse(const HttpRequest&)> handler);
    
    // 声明友元函数
    friend int on_uvhttp_request(uvhttp_request_t* req, uvhttp_response_t* resp);
    
    // 路由查找（供内部使用）
    std::function<HttpResponse(const HttpRequest&)> findHandler(const std::string& path, HttpMethod method) const;
    
private:
    uv_loop_t* loop_;  // 注入的事件循环，不拥有所有权
    UvhttpServerPtr server_;
    UvhttpRouterPtr router_;
    UvhttpContextPtr uvhttp_ctx_;
    UvhttpConfigPtr config_;
    TlsConfig tls_config_;  // TLS 配置
    bool use_https_;
    // 使用 unordered_map 存储自定义处理器（直接使用 uvhttp 路由）
    std::unordered_map<std::string, std::unordered_map<uvhttp_method_t, std::function<HttpResponse(const HttpRequest&)> > > handlers_;
};

} // namespace server

// ========== RESTful API 层：RESTful 特定功能 ==========

// 在 restful 命名空间外部包含 framework_types.h，以避免命名空间污染
#include "framework_types.h"

namespace restful {

// 使用 uvapi 命名空间的类型（因为 restful 在 uvapi 命名空间内部）
using CJsonPtr = uvapi::CJsonPtr;
using CJsonStringPtr = uvapi::CJsonStringPtr;
using HttpRequest = uvapi::HttpRequest;
using HttpResponse = uvapi::HttpResponse;
using ValidationResult = uvapi::ValidationResult;
using HttpMethod = uvapi::HttpMethod;

// JSON 序列化/反序列化工具类
class JSON {
public:
    // ========== 序列化 API ==========
    
    // 快速构造：成功响应
    static std::string success(const std::string& message = "Success") {
        return Object()
            .set("code", "0")
            .set("message", message)
            .toString();
    }
    
    // 快速构造：错误响应
    static std::string error(const std::string& message) {
        return Object()
            .set("code", "-1")
            .set("message", message)
            .toString();
    }
    
    // 快速构造：数据响应
    static std::string data(const std::string& json_data) {
        // 直接将 JSON 字符串包装到 data 字段中
        return Object()
            .set("code", "0")
            .set("message", "Success")
            .set("data", json_data)
            .toString();
    }
    
    // ========== JSON 对象构建器 ==========
    class Array;  // 前向声明
    
    class Object {
    private:
        CJsonPtr root_;
        
    public:
        Object() : root_(uvapi::makeCJson(cJSON_CreateObject())) {}
        
        // 链式设置方法
        Object& set(const std::string& key, const std::string& value) {
            if (root_) {
                cJSON_AddStringToObject(root_.get(), key.c_str(), value.c_str());
            }
            return *this;
        }
        
        Object& set(const std::string& key, const char* value) {
            if (root_ && value) {
                cJSON_AddStringToObject(root_.get(), key.c_str(), value);
            }
            return *this;
        }
        
        Object& set(const std::string& key, int value) {
            if (root_) {
                cJSON_AddNumberToObject(root_.get(), key.c_str(), value);
            }
            return *this;
        }
        
        Object& set(const std::string& key, int64_t value) {
            if (root_) {
                cJSON_AddNumberToObject(root_.get(), key.c_str(), static_cast<double>(value));
            }
            return *this;
        }
        
        Object& set(const std::string& key, double value) {
            if (root_) {
                cJSON_AddNumberToObject(root_.get(), key.c_str(), value);
            }
            return *this;
        }
        
        Object& set(const std::string& key, bool value) {
            if (root_) {
                cJSON_AddBoolToObject(root_.get(), key.c_str(), value);
            }
            return *this;
        }
        
        Object& set(const std::string& key, const Object& obj) {
            if (root_ && obj.isValid()) {
                cJSON_AddItemToObject(root_.get(), key.c_str(), cJSON_Duplicate(obj.get(), true));
            }
            return *this;
        }
        
        Object& set(const std::string& key, const Array& arr) {
            if (root_ && arr.isValid()) {
                cJSON_AddItemToObject(root_.get(), key.c_str(), cJSON_Duplicate(arr.get(), true));
            }
            return *this;
        }
        
        Object& setNull(const std::string& key) {
            if (root_) {
                cJSON_AddNullToObject(root_.get(), key.c_str());
            }
            return *this;
        }
        
        // 转换为 JSON 字符串
        std::string toString() const {
            if (!root_) return "{}";
            CJsonStringPtr json = uvapi::makeCJsonString(cJSON_Print(root_.get()));
            return std::string(json.get() ? json.get() : "{}");
        }
        
        // 转换为紧凑 JSON 字符串
        std::string toCompactString() const {
            if (!root_) return "{}";
            CJsonStringPtr json = uvapi::makeCJsonString(cJSON_PrintUnformatted(root_.get()));
            return std::string(json.get() ? json.get() : "{}");
        }
        
        // 检查对象是否有效
        bool isValid() const { return root_ != nullptr; }
        
        // 获取原始 cJSON 指针（用于高级操作）
        cJSON* get() const { return root_.get(); }
    };
    
    // ========== JSON 数组构建器 ==========
    class Array {
    private:
        CJsonPtr root_;
        
    public:
        Array() : root_(uvapi::makeCJson(cJSON_CreateArray())) {}
        
        // 链式添加方法
        Array& append(const std::string& value) {
            if (root_) {
                cJSON_AddItemToArray(root_.get(), cJSON_CreateString(value.c_str()));
            }
            return *this;
        }
        
        Array& append(const char* value) {
            if (root_ && value) {
                cJSON_AddItemToArray(root_.get(), cJSON_CreateString(value));
            }
            return *this;
        }
        
        Array& append(int value) {
            if (root_) {
                cJSON_AddItemToArray(root_.get(), cJSON_CreateNumber(value));
            }
            return *this;
        }
        
        Array& append(int64_t value) {
            if (root_) {
                cJSON_AddItemToArray(root_.get(), cJSON_CreateNumber(static_cast<double>(value)));
            }
            return *this;
        }
        
        Array& append(double value) {
            if (root_) {
                cJSON_AddItemToArray(root_.get(), cJSON_CreateNumber(value));
            }
            return *this;
        }
        
        Array& append(bool value) {
            if (root_) {
                cJSON_AddItemToArray(root_.get(), cJSON_CreateBool(value));
            }
            return *this;
        }
        
        Array& append(const Object& obj) {
            if (root_ && obj.isValid()) {
                cJSON_AddItemToArray(root_.get(), cJSON_Duplicate(obj.get(), true));
            }
            return *this;
        }
        
        // 转换为 JSON 字符串
        std::string toString() const {
            if (!root_) return "[]";
            CJsonStringPtr json = uvapi::makeCJsonString(cJSON_Print(root_.get()));
            return std::string(json.get() ? json.get() : "[]");
        }
        
        // 转换为紧凑 JSON 字符串
        std::string toCompactString() const {
            if (!root_) return "[]";
            CJsonStringPtr json = uvapi::makeCJsonString(cJSON_PrintUnformatted(root_.get()));
            return std::string(json.get() ? json.get() : "[]");
        }
        
        // 检查数组是否有效
        bool isValid() const { return root_ != nullptr; }
        
        // 获取原始 cJSON 指针（用于高级操作）
        cJSON* get() const { return root_.get(); }
    };
    
    // ========== JSON 解析器 ==========
    class Parser {
    private:
        CJsonPtr root_;
        
    public:
        explicit Parser(const std::string& json_str) 
            : root_(uvapi::makeCJson(cJSON_Parse(json_str.c_str()))) {}
        
        explicit Parser(const char* json_str) 
            : root_(json_str ? uvapi::makeCJson(cJSON_Parse(json_str)) : nullptr) {}
        
        // 检查是否有效
        bool isValid() const { return root_ != nullptr; }
        
        // 获取字段值
        std::string getString(const std::string& key, const std::string& default_value = "") const {
            if (!root_) return default_value;
            cJSON* item = cJSON_GetObjectItem(root_.get(), key.c_str());
            return (item && cJSON_IsString(item)) ? item->valuestring : default_value;
        }
        
        int getInt(const std::string& key, int default_value = 0) const {
            if (!root_) return default_value;
            cJSON* item = cJSON_GetObjectItem(root_.get(), key.c_str());
            return (item && cJSON_IsNumber(item)) ? static_cast<int>(item->valuedouble) : default_value;
        }
        
        int64_t getInt64(const std::string& key, int64_t default_value = 0) const {
            if (!root_) return default_value;
            cJSON* item = cJSON_GetObjectItem(root_.get(), key.c_str());
            return (item && cJSON_IsNumber(item)) ? static_cast<int64_t>(item->valuedouble) : default_value;
        }
        
        double getDouble(const std::string& key, double default_value = 0.0) const {
            if (!root_) return default_value;
            cJSON* item = cJSON_GetObjectItem(root_.get(), key.c_str());
            return (item && cJSON_IsNumber(item)) ? item->valuedouble : default_value;
        }
        
        bool getBool(const std::string& key, bool default_value = false) const {
            if (!root_) return default_value;
            cJSON* item = cJSON_GetObjectItem(root_.get(), key.c_str());
            return (item && cJSON_IsBool(item)) ? (item->type == cJSON_True) : default_value;
        }
        
        // 检查字段是否存在
        bool has(const std::string& key) const {
            if (!root_) return false;
            return cJSON_GetObjectItem(root_.get(), key.c_str()) != nullptr;
        }
        
        // 获取原始 cJSON 指针（用于高级操作）
        cJSON* get() const { return root_.get(); }
    };
};

// CORS 配置
struct CorsConfig {
    bool enabled;
    std::string allowed_origins;
    std::string allowed_methods;
    std::string allowed_headers;
    bool allow_credentials;
    int max_age;
    
    CorsConfig() 
        : enabled(false), allowed_origins("*"), allowed_methods("GET, POST, PUT, DELETE, OPTIONS"),
          allowed_headers("Content-Type, Authorization"), allow_credentials(false), max_age(86400) {}
};

// Token 信息
struct TokenInfo {
    int64_t user_id;
    std::string username;
    std::string role;
    int64_t expires_at;
    
    TokenInfo() : user_id(0), expires_at(0) {}
    TokenInfo(int64_t uid, const std::string& uname, const std::string& r, int64_t exp)
        : user_id(uid), username(uname), role(r), expires_at(exp) {}
};

// 请求处理器类型（原始类型，用于向后兼容）
using RequestHandler = std::function<HttpResponse(const HttpRequest&)>;

// 前向声明 jsonError
std::string jsonError(const std::string& message);

// ========== 类型安全的请求处理器 ==========

/**
 * @brief 类型安全的请求处理器模板
 * 
 * 支持三种形式：
 * 1. handler(const HttpRequest&) -> HttpResponse
 * 2. handler(ReqBody) -> HttpResponse
 * 3. handler(ReqBody) -> ResBody
 * 
 * 框架会自动处理 Body 的反序列化和响应的序列化
 */

// 类型擦除包装器
class TypedRequestHandler {
private:
    std::function<HttpResponse(const HttpRequest&)> handler_;
    std::string request_type_name_;
    std::string response_type_name_;
    
public:
    template<typename Func>
    TypedRequestHandler(Func func) : handler_(wrapHandler(func, typename std::is_same<typename std::result_of<Func(const HttpRequest&)>::type, HttpResponse>::type())) {
        request_type_name_ = getRequestTypeName<Func>();
        response_type_name_ = getResponseTypeName<Func>();
    }    
    HttpResponse operator()(const HttpRequest& req) const {
        return handler_(req);
    }
    
    const std::function<HttpResponse(const HttpRequest&)>& getHandler() const {
        return handler_;
    }
    
    const std::string& getRequestTypeName() const { return request_type_name_; }
    const std::string& getResponseTypeName() const { return response_type_name_; }
    
private:
    // 提取请求类型名称
    template<typename Func>
    static std::string getRequestTypeName() {
        // 使用 SFINAE 替代 if constexpr
        if (std::is_same<typename std::result_of<Func(const HttpRequest&)>::type, HttpResponse>::value) {
            return "HttpRequest";
        } else {
            return typeid(typename function_traits<Func>::template arg<0>::type).name();
        }
        return "unknown";
    }
    
    // 提取响应类型名称
    template<typename Func>
    static std::string getResponseTypeName() {
        using Ret = typename function_traits<Func>::return_type;
        if (std::is_same<Ret, HttpResponse>::value) {
            return "HttpResponse";
        } else {
            return typeid(Ret).name();
        }
    }
    
    // 包装处理器 - 重载1: (HttpRequest) -> HttpResponse
    template<typename Func>
    static auto wrapHandler(Func func, std::true_type) 
        -> std::function<HttpResponse(const HttpRequest&)> {
        return [func](const HttpRequest& req) -> HttpResponse {
            return func(req);
        };
    }
    
    // 包装处理器 - 重载2: (ReqBody) -> HttpResponse
    template<typename Func>
    static auto wrapHandler(Func func, std::false_type)
        -> std::function<HttpResponse(const HttpRequest&)> {
        using ReqBody = typename function_traits<Func>::template arg<0>::type;
        using ResBody = typename function_traits<Func>::return_type;
        
        return wrapHandlerInternal<Func, ReqBody, ResBody>(func);
    }
    
    // 内部包装处理器
    template<typename Func, typename ReqBody, typename ResBody>
    static auto wrapHandlerInternal(Func func)
        -> std::function<HttpResponse(const HttpRequest&)> {
        if (std::is_same<ResBody, HttpResponse>::value) {
            // 情况2: handler(ReqBody) -> HttpResponse
            return [func](const HttpRequest& req) -> HttpResponse {
                try {
                    // 反序列化请求
                    ReqBody body = req.parseBody<ReqBody>();
                    
                    // 验证请求
                    ValidationResult validation = validateRequest(body);
                    if (!validation) {
                        return HttpResponse(400).json(jsonError(validation.error_message));
                    }
                    
                    // 调用处理器
                    return func(body);
                } catch (const std::exception& e) {
                    return HttpResponse(400).json(jsonError(std::string("Parse error: ") + e.what()));
                }
            };
        } else {
            // 情况3: handler(ReqBody) -> ResBody
            return [func](const HttpRequest& req) -> HttpResponse {
                try {
                    // 反序列化请求
                    ReqBody body = req.parseBody<ReqBody>();
                    
                    // 验证请求
                    ValidationResult validation = validateRequest(body);
                    if (!validation) {
                        return HttpResponse(400).json(jsonError(validation.error_message));
                    }
                    
                    // 调用处理器获取响应
                    ResBody response = func(body);
                    
                    // 序列化响应
                    std::string json = response.schema()->toJson(&response);
                    
                    return HttpResponse(200).json(json);
                } catch (const std::exception& e) {
                    return HttpResponse(400).json(jsonError(std::string("Error: ") + e.what()));
                }
            };
        }
    }
    
    // 辅助：函数特征提取
    template<typename T>
    struct function_traits;
    
    template<typename R, typename... Args>
    struct function_traits<R(Args...)> {
        using return_type = R;
        template<size_t I>
        using arg = typename std::tuple_element<I, std::tuple<Args...>>::type;
    };
    
    template<typename R, typename... Args>
    struct function_traits<R(*)(Args...)> : function_traits<R(Args...)> {};
    
    template<typename R, typename C, typename... Args>
    struct function_traits<R(C::*)(Args...) const> : function_traits<R(Args...)> {};
    
    template<typename R, typename C, typename... Args>
    struct function_traits<R(C::*)(Args...)> : function_traits<R(Args...)> {};
    
    // Lambda 特化
    template<typename T>
    struct function_traits : function_traits<decltype(&T::operator())> {};
};

// 参数类型
enum class ParamType {
    PATH,   // URL 路径参数
    QUERY,  // Query 查询参数
    BODY    // Request Body 字段
};

// 参数验证规则
struct ParamValidation {
    bool required;
    int min_value;
    int max_value;
    double min_double;
    double max_double;
    std::string pattern;
    std::vector<std::string> enum_values;
    bool has_min;
    bool has_max;
    bool has_pattern;
    bool has_enum;
    int min_length;
    int max_length;
    bool has_min_length;
    bool has_max_length;
    
    ParamValidation() 
        : required(false), min_value(0), max_value(0), 
          min_double(0.0), max_double(0.0),
          has_min(false), has_max(false), has_pattern(false), has_enum(false),
          min_length(0), max_length(0), has_min_length(false), has_max_length(false) {}
};

// 参数定义
struct ParamDefinition {
    std::string name;
    ParamType type;
    std::string default_value;
    ParamValidation validation;
    int data_type;  // 0=string, 1=int, 2=int64, 3=double, 4=float, 5=bool
    
    ParamDefinition(const std::string& n, ParamType t) 
        : name(n), type(t), data_type(0) {}  // 默认为 string
};

// 参数构建器
class ParamBuilder {
private:
    ParamDefinition param_;
    
public:
    ParamBuilder(const std::string& name, ParamType type) : param_(name, type) {}
    
    // 设置默认值
    ParamBuilder& defaultValue(const std::string& value) {
        param_.default_value = value;
        return *this;
    }
    
    ParamBuilder& defaultValue(int value) {
        param_.default_value = std::to_string(value);
        return *this;
    }
    
    ParamBuilder& defaultValue(double value) {
        param_.default_value = std::to_string(value);
        return *this;
    }
    
    ParamBuilder& defaultValue(bool value) {
        param_.default_value = value ? "true" : "false";
        param_.data_type = 5;  // bool
        return *this;
    }
    
    // 类型设置
    ParamBuilder& asString() {
        param_.data_type = 0;  // string
        return *this;
    }
    
    ParamBuilder& asInt() {
        param_.data_type = 1;  // int
        return *this;
    }
    
    ParamBuilder& asInt64() {
        param_.data_type = 2;  // int64
        return *this;
    }
    
    ParamBuilder& asDouble() {
        param_.data_type = 3;  // double
        return *this;
    }
    
    ParamBuilder& asFloat() {
        param_.data_type = 4;  // float
        return *this;
    }
    
    ParamBuilder& asBool() {
        param_.data_type = 5;  // bool
        return *this;
    }
    
    // 必填参数
    ParamBuilder& required() {
        param_.validation.required = true;
        return *this;
    }
    
    // 可选参数
    ParamBuilder& optional() {
        param_.validation.required = false;
        return *this;
    }
    
    // 整数范围
    ParamBuilder& range(int min_val, int max_val) {
        param_.validation.min_value = min_val;
        param_.validation.max_value = max_val;
        param_.validation.has_min = true;
        param_.validation.has_max = true;
        return *this;
    }
    
    // 最小值
    ParamBuilder& min(int min_val) {
        param_.validation.min_value = min_val;
        param_.validation.has_min = true;
        return *this;
    }
    
    // 最大值
    ParamBuilder& max(int max_val) {
        param_.validation.max_value = max_val;
        param_.validation.has_max = true;
        return *this;
    }
    
    // 浮点数范围
    ParamBuilder& range(double min_val, double max_val) {
        param_.validation.min_double = min_val;
        param_.validation.max_double = max_val;
        param_.validation.has_min = true;
        param_.validation.has_max = true;
        return *this;
    }
    
    // 正则表达式
    ParamBuilder& pattern(const std::string& regex) {
        param_.validation.pattern = regex;
        param_.validation.has_pattern = true;
        return *this;
    }
    
    // 枚举值
    ParamBuilder& enum_(const std::vector<std::string>& values) {
        param_.validation.enum_values = values;
        param_.validation.has_enum = true;
        return *this;
    }
    
    const ParamDefinition& get() const { return param_; }
};

// 前向声明
class RouteBuilder;

// 参数组 - 用于复用参数定义
class ParamGroup {
private:
    std::vector<ParamDefinition> params_;
    
public:
    ParamGroup() {}
    
    // 添加路径参数
    ParamGroup& addPathParam(const std::string& name, std::function<void(ParamBuilder&)> config) {
        ParamBuilder builder(name, ParamType::PATH);
        config(builder);
        params_.push_back(builder.get());
        return *this;
    }
    
    // 添加查询参数
    ParamGroup& addQueryParam(const std::string& name, std::function<void(ParamBuilder&)> config) {
        ParamBuilder builder(name, ParamType::QUERY);
        config(builder);
        params_.push_back(builder.get());
        return *this;
    }
    
    // 应用到路由构建器
    void applyTo(RouteBuilder& route_builder);
    
    const std::vector<ParamDefinition>& getParams() const { return params_; }
};

// 常用参数组定义（全局可复用）
class CommonParams {
public:
    // 翻页参数
    static ParamGroup pagination() {
        ParamGroup group;
        group.addQueryParam("page", [](ParamBuilder& p) {
            p.defaultValue(1).min(1).optional();
        });
        group.addQueryParam("limit", [](ParamBuilder& p) {
            p.defaultValue(10).range(1, 100).optional();
        });
        group.addQueryParam("sort", [](ParamBuilder& p) {
            p.defaultValue("id").optional();
        });
        group.addQueryParam("order", [](ParamBuilder& p) {
            p.defaultValue("asc").enum_({"asc", "desc"}).optional();
        });
        return group;
    }
    
    // 搜索参数
    static ParamGroup search() {
        ParamGroup group;
        group.addQueryParam("q", [](ParamBuilder& p) {
            p.optional();
        });
        group.addQueryParam("fields", [](ParamBuilder& p) {
            p.optional();
        });
        return group;
    }
    
    // ID 参数
    static ParamGroup idParam(const std::string& name = "id", int min_val = 1, int max_val = 1000000) {
        ParamGroup group;
        group.addPathParam(name, [min_val, max_val](ParamBuilder& p) {
            p.required().range(min_val, max_val);
        });
        return group;
    }
    
    // 时间范围参数
    static ParamGroup dateRange() {
        ParamGroup group;
        group.addQueryParam("start_date", [](ParamBuilder& p) {
            p.optional();
        });
        group.addQueryParam("end_date", [](ParamBuilder& p) {
            p.optional();
        });
        return group;
    }
    
// 过滤参数
    static ParamGroup filter(const std::vector<std::string>& filter_fields) {
        ParamGroup group;
        for (const auto& field : filter_fields) {
            group.addQueryParam(field, [](ParamBuilder& p) {
                p.optional();
            });
        }
        return group;
    }
};

// 路由定义
struct RouteDefinition {
    std::string path;
    HttpMethod method;
    RequestHandler handler;
    std::vector<ParamDefinition> path_params;
    std::vector<ParamDefinition> query_params;
    
    RouteDefinition(const std::string& p, HttpMethod m, RequestHandler h)
        : path(p), method(m), handler(h) {}
};

// 前向声明
class Api;

// 路由构建器 - 支持在 handler 之前声明参数
class RouteBuilder {
private:
    Api* api_;
    RouteDefinition route_;
    ParamGroup param_group_;
    
public:
    RouteBuilder(Api* api, const std::string& path, HttpMethod method)
        : api_(api), route_(path, method, [](const HttpRequest& /*req*/) -> HttpResponse {
            return HttpResponse(500).json("{\"code\":\"500\",\"message\":\"Handler not set\"}");
        }) {}
    
    // 添加路径参数
    RouteBuilder& param(const std::string& name, std::function<void(ParamBuilder&)> config) {
        param_group_.addPathParam(name, config);
        return *this;
    }
    
    // 添加查询参数
    RouteBuilder& query(const std::string& name, std::function<void(ParamBuilder&)> config) {
        param_group_.addQueryParam(name, config);
        return *this;
    }
    
    // 应用参数组
    RouteBuilder& apply(const ParamGroup& group) {
        for (const auto& param : group.getParams()) {
            if (param.type == ParamType::PATH) {
                param_group_.addPathParam(param.name, [param](ParamBuilder& p) {
                    if (param.validation.required) p.required();
                    if (!param.validation.required) p.optional();
                    if (param.validation.has_min) p.min(param.validation.min_value);
                    if (param.validation.has_max) p.max(param.validation.max_value);
                    if (param.validation.has_min && param.validation.has_max) {
                        p.range(param.validation.min_value, param.validation.max_value);
                    }
                    if (param.validation.has_pattern) p.pattern(param.validation.pattern);
                    if (param.validation.has_enum) p.enum_(param.validation.enum_values);
                });
            } else {
                param_group_.addQueryParam(param.name, [param](ParamBuilder& p) {
                    if (!param.default_value.empty()) {
                        if (param.validation.has_min || param.validation.has_max) {
                            // 尝试解析为整数
                            char* endptr = nullptr;
                            long val = std::strtol(param.default_value.c_str(), &endptr, 10);
                            if (endptr && *endptr == '\0') {
                                p.defaultValue(static_cast<int>(val));
                            } else {
                                p.defaultValue(param.default_value);
                            }
                        } else {
                            p.defaultValue(param.default_value);
                        }
                    }
                    if (param.validation.required) p.required();
                    if (!param.validation.required) p.optional();
                    if (param.validation.has_min) p.min(param.validation.min_value);
                    if (param.validation.has_max) p.max(param.validation.max_value);
                    if (param.validation.has_min && param.validation.has_max) {
                        p.range(param.validation.min_value, param.validation.max_value);
                    }
                    if (param.validation.has_pattern) p.pattern(param.validation.pattern);
                    if (param.validation.has_enum) p.enum_(param.validation.enum_values);
                });
            }
        }
        return *this;
    }
    
    // 设置 handler（返回 RouteBuilder 支持链式调用）
    RouteBuilder& handler(RequestHandler handler) {
        route_.handler = handler;
        return *this;
    }
    
    // 注册路由
    void register_();
};

// 中间件类型
using Middleware = std::function<HttpResponse(const HttpRequest&, RequestHandler)>;

// RESTful API 应用类
/**
 * @brief RESTful API 应用类
 * 
 * ## 设计哲学
 * 
 * 1. **零异常 (Zero Exceptions)**: 不使用异常机制进行错误处理
 *    - 所有错误通过返回码和错误对象传递
 *    - 使用 ValidationResult、HTTP 状态码等表示错误
 *    - 避免异常带来的性能开销和不确定性
 * 
 * 2. **类型安全**: 编译时类型检查，自动序列化/反序列化
 * 
 * 3. **高性能**: 零拷贝优化，事件驱动架构
 * 
 * 4. **可测试性**: 无全局变量，支持多实例
 */
class Api {
public:
    Api(uv_loop_t* loop);  // 事件循环注入
    ~Api();
    
    // ========== 类型安全的路由注册（推荐使用）==========
    // 支持三种形式的处理器：
    // 1. (HttpRequest) -> HttpResponse
    // 2. (ReqBody) -> HttpResponse
    // 3. (ReqBody) -> ResBody
    
    template<typename Handler>
    Api& get(const std::string& path, Handler handler) {
        TypedRequestHandler typed_handler(handler);
        return get(path, typed_handler.getHandler());
    }
    
    template<typename Handler>
    Api& post(const std::string& path, Handler handler) {
        TypedRequestHandler typed_handler(handler);
        return post(path, typed_handler.getHandler());
    }
    
    template<typename Handler>
    Api& put(const std::string& path, Handler handler) {
        TypedRequestHandler typed_handler(handler);
        return put(path, typed_handler.getHandler());
    }
    
    template<typename Handler>
    Api& delete_(const std::string& path, Handler handler) {
        TypedRequestHandler typed_handler(handler);
        return delete_(path, typed_handler.getHandler());
    }
    
    template<typename Handler>
    Api& patch(const std::string& path, Handler handler) {
        TypedRequestHandler typed_handler(handler);
        return patch(path, typed_handler.getHandler());
    }
    
    template<typename Handler>
    Api& head(const std::string& path, Handler handler) {
        TypedRequestHandler typed_handler(handler);
        return head(path, typed_handler.getHandler());
    }
    
    template<typename Handler>
    Api& options(const std::string& path, Handler handler) {
        TypedRequestHandler typed_handler(handler);
        return options(path, typed_handler.getHandler());
    }
    
    // ========== 路由注册（传统方式，向后兼容）==========
    Api& get(const std::string& path, RequestHandler handler);
    Api& post(const std::string& path, RequestHandler handler);
    Api& put(const std::string& path, RequestHandler handler);
    Api& delete_(const std::string& path, RequestHandler handler);
    Api& patch(const std::string& path, RequestHandler handler);
    Api& head(const std::string& path, RequestHandler handler);
    Api& options(const std::string& path, RequestHandler handler);
    
    // 路由注册（DSL 方式 - 参数在 handler 之前）
    RouteBuilder route(const std::string& path, HttpMethod method);
    RouteBuilder get(const std::string& path);
    RouteBuilder post(const std::string& path);
    RouteBuilder put(const std::string& path);
    RouteBuilder delete_(const std::string& path);
    RouteBuilder patch(const std::string& path);
    RouteBuilder head(const std::string& path);
    RouteBuilder options(const std::string& path);
    
    // API 信息
    Api& title(const std::string& t) { api_title_ = t; return *this; }
    Api& description(const std::string& d) { api_description_ = d; return *this; }
    Api& version(const std::string& v) { api_version_ = v; return *this; }
    
    // CORS 配置
    Api& enableCors(const CorsConfig& config);
    Api& enableCors(bool enabled = true);
    Api& disableCors();
    
    // 启动应用
    bool run(const std::string& host = "0.0.0.0", int port = 8080);
    
    // 停止应用
    void stop();
    
    // 获取服务器实例（供 RouteBuilder 使用）
    uvapi::server::Server* getServer() { return server_.get(); }
    
    // Token 管理
    std::string generateToken(int64_t user_id, const std::string& username, 
                              const std::string& role, int64_t expires_in_seconds = 3600);
    bool validateToken(const std::string& token, int64_t& user_id, std::string& username, std::string& role);
    std::string refreshToken(const std::string& token, int64_t expires_in_seconds = 3600);
    bool revokeToken(const std::string& token);
    void cleanupExpiredTokens();
    
    // 请求处理
    HttpResponse handle_request(const HttpRequest& req);
    
private:
    // 辅助方法：检查 token 是否过期
    bool isTokenExpired(const TokenInfo& info) const;
    
private:
    std::string api_title_;
    std::string api_description_;
    std::string api_version_;
    
    bool running_;
    CorsConfig cors_config_;
    bool cors_enabled_;
    std::map<std::string, TokenInfo> tokens_;
    int64_t token_generation_count_;
    int64_t last_cleanup_time_;
    
    std::unique_ptr<uvapi::server::Server> server_;  // 使用 unique_ptr 管理 Server 层
    
    std::string generateRandomString(size_t length);
    std::string extractBearerToken(const std::string& auth_header);
};

// ========== 声明式 Response DSL ==========

/**
 * @brief 声明式 Response DSL
 * 
 * 风格与 Request DSL 统一，使用流式链式 API 构建响应
 * 
 * 示例：
 * Response::ok()
 *     .code(200)
 *     .message("Success")
 *     .data("user", user_obj)
 *     .build();
 */
class Response {
private:
    HttpResponse response_;
    
    // 声明 ResponseBuilder 为友元类
    friend class ResponseBuilder;
    
public:
    // ========== 构造方法 ==========
    
    Response() = default;
    
    explicit Response(int status_code) {
        response_.status_code = status_code;
    }
    
    // ========== 转换为 HttpResponse ==========
    
    // 隐式转换
    operator HttpResponse() const {
        return response_;
    }
    
    // 显式转换
    HttpResponse toHttpResponse() const {
        return response_;
    }
    
    // ========== 访问器 ==========
    
    int status() const { return response_.status_code; }
    const std::string& body() const { return response_.body; }
    const std::map<std::string, std::string>& headers() const { return response_.headers; }
    
    // ========== 修改器（仅用于高级场景） ==========
    
    Response& setStatus(int status) {
        response_.status_code = status;
        return *this;
    }
    
    Response& setBody(const std::string& body) {
        response_.body = body;
        return *this;
    }
    
    Response& setHeader(const std::string& key, const std::string& value) {
        response_.headers[key] = value;
        return *this;
    }
};

// ========== Response 构建器 - 支持在 handler 之前声明 Response 结构 ==========

/**
 * @brief Response 构建器 - 支持在 handler 之前声明 Response 结构
 * 
 * 类似于 RouteBuilder，允许在 handler 函数外部声明 Response 的结构
 * 
 * 设计原则：
 * 1. 零全局变量 - 使用函数返回局部对象，避免 static 全局变量
 * 2. 声明式风格 - 描述响应属性（what），而非执行动作（how）
 * 3. 类型安全 - 编译期类型检查，自动序列化
 * 4. 错误处理 - 捕获异常，返回错误响应
 * 5. 性能优化 - 链式调用，减少拷贝
 * 
 * 示例（纯粹的声明式风格）：
 * 
 * 描述响应具有什么属性：
 * - "status is 200" → .status(200)
 * - "message is Success" → .message("Success")
 * - "has cache-control no-cache" → .cacheControl("no-cache")
 * - "has data user" → .data(user)
 * 
 * auto handler = [](const HttpRequest& req) -> HttpResponse {
 *     User user = {1, "alice", "alice@example.com"};
 *     return ResponseBuilder::created()
 *         .message("User created successfully")
 *         .requestId("12345")
 *         .data(user)
 *         .toHttpResponse();
 * };
 */
class ResponseBuilder {
private:
    int status_code_;
    std::string message_;
    std::map<std::string, std::string> headers_;
    mutable std::string pending_data_;  // 待序列化的数据
    
public:
    ResponseBuilder(int code = 200, const std::string& msg = "Success")
        : status_code_(code), message_(msg) {}
    
    // ========== 声明式方法（描述响应属性） ==========
    
    // 描述状态码
    ResponseBuilder& status(int code) {
        status_code_ = code;
        return *this;
    }
    
    // 描述消息
    ResponseBuilder& message(const std::string& msg) {
        message_ = msg;
        return *this;
    }
    
    // 描述头部
    ResponseBuilder& header(const std::string& key, const std::string& value) {
        headers_[key] = value;
        return *this;
    }
    
    // 描述 Content-Type
    ResponseBuilder& contentType(const std::string& type) {
        headers_["Content-Type"] = type;
        return *this;
    }
    
    // 描述 Cache-Control
    ResponseBuilder& cacheControl(const std::string& value) {
        headers_["Cache-Control"] = value;
        return *this;
    }
    
    // 描述请求 ID（常用头部，提供便捷方法）
    ResponseBuilder& requestId(const std::string& id) {
        headers_["X-Request-ID"] = id;
        return *this;
    }
    
    // 描述追踪 ID（常用头部，提供便捷方法）
    ResponseBuilder& traceId(const std::string& id) {
        headers_["X-Trace-ID"] = id;
        return *this;
    }
    
    // ========== 快速构造方法 ==========
    
    static ResponseBuilder ok() {
        return ResponseBuilder(200, "Success");
    }
    
    static ResponseBuilder ok(const std::string& message) {
        return ResponseBuilder(200, message);
    }
    
    static ResponseBuilder created() {
        return ResponseBuilder(201, "Created");
    }
    
    static ResponseBuilder created(const std::string& message) {
        return ResponseBuilder(201, message);
    }
    
    static ResponseBuilder accepted() {
        return ResponseBuilder(202, "Accepted");
    }
    
    static ResponseBuilder noContent() {
        return ResponseBuilder(204, "No Content");
    }
    
    static ResponseBuilder badRequest() {
        return ResponseBuilder(400, "Bad Request");
    }
    
    static ResponseBuilder badRequest(const std::string& message) {
        return ResponseBuilder(400, message);
    }
    
    static ResponseBuilder unauthorized() {
        return ResponseBuilder(401, "Unauthorized");
    }
    
    static ResponseBuilder unauthorized(const std::string& message) {
        return ResponseBuilder(401, message);
    }
    
    static ResponseBuilder forbidden() {
        return ResponseBuilder(403, "Forbidden");
    }
    
    static ResponseBuilder forbidden(const std::string& message) {
        return ResponseBuilder(403, message);
    }
    
    static ResponseBuilder notFound() {
        return ResponseBuilder(404, "Not Found");
    }
    
    static ResponseBuilder notFound(const std::string& message) {
        return ResponseBuilder(404, message);
    }
    
    static ResponseBuilder methodNotAllowed() {
        return ResponseBuilder(405, "Method Not Allowed");
    }
    
    static ResponseBuilder conflict() {
        return ResponseBuilder(409, "Conflict");
    }
    
    static ResponseBuilder internalServerError() {
        return ResponseBuilder(500, "Internal Server Error");
    }
    
    static ResponseBuilder serviceUnavailable() {
        return ResponseBuilder(503, "Service Unavailable");
    }
    
    // ========== 数据设置（声明式风格） ==========
    
    // 描述单个对象数据
    template<typename T>
    auto data(const T& instance) -> typename std::enable_if<
        std::is_same<decltype(std::declval<T>().toJson()), std::string>::value,
        ResponseBuilder&
    >::type {
        // 错误处理：捕获 toJson() 可能的异常
        try {
            std::string json_str = instance.toJson();
            if (json_str.empty()) {
                // toJson() 返回空字符串，可能是错误
                status_code_ = 500;
                message_ = "Failed to serialize object";
                pending_data_ = "{}";
            } else {
                pending_data_ = json_str;
            }
        } catch (...) {
            // 捕获所有异常，返回错误响应
            status_code_ = 500;
            message_ = "Serialization error";
            pending_data_ = "{}";
        }
        return *this;
    }
    
    // 描述数组数据
    template<typename T>
    auto data(const std::vector<T>& instances) -> typename std::enable_if<
        std::is_same<decltype(std::declval<T>().toJson()), std::string>::value,
        ResponseBuilder&
    >::type {
        // 错误处理
        try {
            JSON::Array arr;
            for (const auto& instance : instances) {
                arr.append(instance.toJson());
            }
            pending_data_ = arr.toString();
        } catch (...) {
            status_code_ = 500;
            message_ = "Serialization error";
            pending_data_ = "[]";
        }
        return *this;
    }
    
    // 描述字符串数据
    ResponseBuilder& data(const std::string& json_data) {
        pending_data_ = json_data;
        return *this;
    }
    
    // ========== 构建 Response 对象 ==========
    
    // 私有方法：构建带数据的 Response（消除代码冗余）
    Response buildWithData(const std::string& data_str) const {
        Response resp;
        resp.response_.status_code = status_code_;
        
        // 构建响应体
        resp.response_.body = JSON::Object()
            .set("code", std::to_string(status_code_))
            .set("message", message_)
            .set("data", data_str)
            .toString();
        
        // 设置头部
        resp.response_.headers["Content-Type"] = "application/json";
        for (const auto& h : headers_) {
            if (h.first != "Content-Type") {
                resp.response_.headers[h.first] = h.second;
            }
        }
        
        return resp;
    }
    
    // 构建 Response（const 版本）
    Response build() const {
        if (!pending_data_.empty()) {
            return buildWithData(pending_data_);
        }
        // 构建基础响应
        Response resp;
        resp.response_.status_code = status_code_;
        resp.response_.body = JSON::Object()
            .set("code", std::to_string(status_code_))
            .set("message", message_)
            .toString();
        for (const auto& h : headers_) {
            resp.response_.headers[h.first] = h.second;
        }
        return resp;
    }
    
    // 转换为 HttpResponse（隐式转换，可自动调用）
    operator HttpResponse() const {
        return build();
    }
    
    // 显式转换（与隐式转换功能相同，保持一致性）
    HttpResponse toHttpResponse() const {
        return build();
    }
    
    // 获取内部状态（用于高级操作）
    int getStatus() const { return status_code_; }
    const std::string& getMessage() const { return message_; }
    const std::map<std::string, std::string>& getHeaders() const { return headers_; }
};

// ========== 响应模板工厂函数（避免 static 全局变量） ==========

/**
 * @brief 创建成功响应模板
 * 
 * 使用函数返回局部对象，避免 static 全局变量
 * 符合零全局变量原则
 * 使用声明式风格（描述响应属性）
 */
inline ResponseBuilder makeSuccessResponse() {
    return ResponseBuilder::ok()
        .cacheControl("no-cache")
        .contentType("application/json");
}

/**
 * @brief 创建创建用户响应模板
 */
inline ResponseBuilder makeCreatedResponse() {
    return ResponseBuilder::created()
        .message("User created successfully")
        .contentType("application/json");
}

/**
 * @brief 创建错误响应模板
 */
inline ResponseBuilder makeErrorResponse() {
    return ResponseBuilder::badRequest()
        .contentType("application/json");
}

/**
 * @brief 创建未找到响应模板
 */
inline ResponseBuilder makeNotFoundResponse() {
    return ResponseBuilder::notFound()
        .contentType("application/json");
}

/**
 * @brief 创建列表响应模板（带分页信息）
 */
inline ResponseBuilder makeListResponse() {
    return ResponseBuilder::ok()
        .message("List retrieved successfully")
        .cacheControl("private, max-age=60")
        .contentType("application/json");
}

} // namespace restful

} // namespace uvapi

// ========== Body 序列化/反序列化使用示例 ==========
/*
 * 
 * 1. 定义一个 Body 类并实现 Schema:
 * 
 * struct LoginRequest {
 *     std::string username;
 *     std::string password;
 *     bool remember_me;
 *     
 *     // 定义 Schema
 *     class LoginRequestSchema : public uvapi::BodySchema<LoginRequest> {
 *     public:
 *         LoginRequestSchema() {
 *             addField("username", uvapi::FieldType::STRING, offsetof(LoginRequest, username))
 *                 .required().minLength(3).maxLength(20);
 *             addField("password", uvapi::FieldType::STRING, offsetof(LoginRequest, password))
 *                 .required().minLength(6);
 *             addField("remember_me", uvapi::FieldType::BOOL, offsetof(LoginRequest, remember_me));
 *         }
 *     };
 *     
 *     BodySchemaBase* schema() const override {
 *         static LoginRequestSchema instance;
 *         return &instance;
 *     }
 * };
 * };
 * 
 * 2. 在路由处理中使用:
 * 
 * app.post("/api/auth/login", [](const HttpRequest& req) -> HttpResponse {
 *     // 自动反序列化 JSON 到对象
 *     LoginRequest loginReq = req.parseBody<LoginRequest>();
 *     
 *     // 业务逻辑...
 *     
 *     // 自动序列化对象到 JSON
 *     return HttpResponse(200).json(LoginResponse{token: "abc123"});
 * });
 * 
 * 3. 响应类定义:
 * 
 * struct LoginResponse {
 *     std::string token;
 *     int64_t user_id;
 *     std::string username;
 *     std::string role;
 *     
 *     class LoginResponseSchema : public uvapi::BodySchema<LoginResponse> {
 *     public:
 *         LoginResponseSchema() {
 *             addField("token", uvapi::FieldType::STRING, offsetof(LoginResponse, token));
 *             addField("user_id", uvapi::FieldType::INT64, offsetof(LoginResponse, user_id));
 *             addField("username", uvapi::FieldType::STRING, offsetof(LoginResponse, username));
 *             addField("role", uvapi::FieldType::STRING, offsetof(LoginResponse, role));
 *         }
 *     };
 *     
 *     BodySchemaBase* schema() const override {
 *         static LoginResponseSchema instance;
 *         return &instance;
 *     }
 * };
 * };
 *
 */

#endif // RESTFUL_FRAMEWORK_H
