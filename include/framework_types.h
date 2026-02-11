/**
 * @file framework_types.h
 * @brief RESTful 低代码框架基础类型定义
 */

#ifndef FRAMEWORK_TYPES_H
#define FRAMEWORK_TYPES_H

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <unordered_map>

namespace uvapi {

// 前向声明统一的 Schema 基类
class SchemaBase;

// ========== 校验结果类型 ==========

struct ValidationResult {
    bool success;
    std::string error_message;
    
    // 默认构造：成功
    ValidationResult() : success(true), error_message("") {}
    
    // 从字符串隐式转换：失败
    ValidationResult(const std::string& error) : success(false), error_message(error) {}
    
    // 从 const char* 隐式转换：失败
    ValidationResult(const char* error) : success(false), error_message(error ? error : "") {}
    
    // 从 bool 隐式转换
    ValidationResult(bool success) : success(success), error_message("") {}
    
    // 静态工厂方法（保留以兼容现有代码）
    static ValidationResult ok() {
        return ValidationResult{true};
    }
    
    static ValidationResult fail(const std::string& message) {
        return ValidationResult{message};
    }
    
    // 转换为 bool（用于 if 条件判断）
    explicit operator bool() const {
        return success;
    }
};

// ========== Body 序列化和反序列化 ==========

// 自定义类型处理器接口
class ICustomTypeHandler {
public:
    virtual ~ICustomTypeHandler() {}
    
    // 序列化：从实例转换为 JSON 字符串
    virtual std::string serialize(void* instance, size_t offset) const = 0;
    
    // 反序列化：从 JSON 值转换为实例
    virtual bool deserialize(const cJSON* json, void* instance, size_t offset, std::string& error) const = 0;
    
    // 验证：验证 JSON 值
    virtual std::string validate(const cJSON* json) const = 0;
};

// 字段类型枚举（使用标准的整数和浮点数位宽命名）
enum class FieldType {
    STRING,      // 字符串类型
    INT8,        // 8位有符号整数（-128 ~ 127）
    INT16,       // 16位有符号整数（-32768 ~ 32767）
    INT32,       // 32位有符号整数（-2147483648 ~ 2147483647）
    INT64,       // 64位有符号整数
    UINT8,       // 8位无符号整数（0 ~ 255）
    UINT16,      // 16位无符号整数（0 ~ 65535）
    UINT32,      // 32位无符号整数（0 ~ 4294967295）
    UINT64,      // 64位无符号整数
    FP32,        // 32位浮点数（单精度 float）
    FP64,        // 64位浮点数（双精度 double）
    BOOL,        // 布尔值
    DATE,        // 日期类型（YYYY-MM-DD）
    DATETIME,    // 日期时间类型（YYYY-MM-DD HH:MM:SS）
    EMAIL,       // 邮箱类型（自动验证邮箱格式）
    URL,         // URL 类型（自动验证 URL 格式）
    UUID,        // UUID 类型（自动验证 UUID 格式）
    ARRAY,       // 数组类型
    OBJECT,      // 对象类型
    CUSTOM       // 自定义类型（通过 ICustomTypeHandler 处理）
};

// 兼容旧代码的类型别名（标记为已弃用）
using INT = INT32;
using INT64_TYPE = INT64;
using FLOAT = FP32;
using DOUBLE = FP64;

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
    bool use_optional;  // 是否使用 std::optional 容器
    
    FieldValidation() 
        : required(false), min_length(0), max_length(0), has_min_length(false), has_max_length(false),
          min_value(0), max_value(0), has_min_value(false), has_max_value(false),
          use_optional(false) {}
};

// 前向声明
class BodySchemaBase;

// 字段定义
struct FieldDefinition {
    std::string name;
    FieldType type;
    size_t offset;
    FieldValidation validation;
    bool is_optional;  // 是否使用 std::optional 容器
    
    // 嵌套对象/数组支持
    BodySchemaBase* nested_schema;  // 嵌套对象的 schema
    BodySchemaBase* item_schema;    // 数组元素的 schema
    FieldType element_type;         // 数组元素类型（用于简单类型数组）
    
    // 自定义类型处理器
    ICustomTypeHandler* custom_handler;  // 自定义类型处理器
    
    FieldDefinition(const std::string& n, FieldType t, size_t o)
        : name(n), type(t), offset(o), is_optional(false), 
          nested_schema(nullptr), item_schema(nullptr), element_type(FieldType::STRING), 
          custom_handler(nullptr) {}
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
    virtual std::string validateBody(void* instance) const {
        // 默认实现：无额外校验，返回空字符串表示通过
        return "";
    }
};

// ========== Response Body Schema DSL ==========

// Response 字段类型（使用标准的整数和浮点数位宽命名）
enum class ResponseFieldType {
    STRING,      // 字符串类型
    INT8,        // 8位有符号整数
    INT16,       // 16位有符号整数
    INT32,       // 32位有符号整数
    INT64,       // 64位有符号整数
    UINT8,       // 8位无符号整数
    UINT16,      // 16位无符号整数
    UINT32,      // 32位无符号整数
    UINT64,      // 64位无符号整数
    FP32,        // 32位浮点数（单精度）
    FP64,        // 64位浮点数（双精度）
    BOOL,        // 布尔值
    DATE,        // 日期类型
    DATETIME,    // 日期时间类型
    EMAIL,       // 邮箱类型
    URL,         // URL 类型
    UUID,        // UUID 类型
    ARRAY,       // 数组类型
    OBJECT,      // 对象类型
    CUSTOM       // 自定义类型
};

// 兼容旧代码的类型别名
using INT = INT32;
using INT64_TYPE = INT64;
using FLOAT = FP32;
using DOUBLE = FP64;

// Response 字段定义
struct ResponseFieldDef {
    std::string name;
    ResponseFieldType type;
    std::function<std::string(void*)> getter;  // 字段获取函数
    
    ResponseFieldDef(const std::string& n, ResponseFieldType t, std::function<std::string(void*)> g)
        : name(n), type(t), getter(g) {}
};

// Response Schema 基类
class ResponseSchemaBase {
public:
    virtual ~ResponseSchemaBase() {}
    virtual std::vector<ResponseFieldDef> fields() const = 0;
    
    // 序列化为 JSON
    std::string toJson(void* instance) const {
        cJSON* root = cJSON_CreateObject();
        if (!root) return "{}";
        
        for (const auto& field : fields()) {
            std::string value = field.getter(instance);
            
            cJSON* field_json = nullptr;
            switch (field.type) {
                case ResponseFieldType::STRING:
                    field_json = cJSON_CreateString(value.c_str());
                    break;
                case ResponseFieldType::INT8:
                case ResponseFieldType::INT16:
                case ResponseFieldType::INT32:
                    field_json = cJSON_CreateNumber(std::stoi(value));
                    break;
                case ResponseFieldType::INT64:
                case ResponseFieldType::UINT8:
                case ResponseFieldType::UINT16:
                case ResponseFieldType::UINT32:
                case ResponseFieldType::UINT64:
                    field_json = cJSON_CreateNumber(std::stoll(value));
                    break;
                case ResponseFieldType::FP32:
                    field_json = cJSON_CreateNumber(std::stof(value));
                    break;
                case ResponseFieldType::FP64:
                    field_json = cJSON_CreateNumber(std::stod(value));
                    break;
                case ResponseFieldType::BOOL:
                    field_json = cJSON_CreateBool(value == "true");
                    break;
                case ResponseFieldType::OBJECT:
                case ResponseFieldType::ARRAY:
                    // 直接解析 JSON 字符串
                    field_json = cJSON_Parse(value.c_str());
                    if (!field_json) {
                        field_json = cJSON_CreateString(value.c_str());
                    }
                    break;
            }
            
            if (field_json) {
                cJSON_AddItemToObject(root, field.name.c_str(), field_json);
            }
        }
        
        char* json = cJSON_Print(root);
        std::string result(json ? json : "{}");
        if (json) free(json);
        cJSON_Delete(root);
        
        return result;
    }
};

// DSL 风格的 Response Schema
template<typename T>
class ResponseSchema : public ResponseSchemaBase {
private:
    std::vector<ResponseFieldDef> fields_;
    
public:
    // 添加字段
    ResponseSchema& add(const std::string& name, ResponseFieldType type, std::function<std::string(T*)> getter) {
        fields_.push_back(ResponseFieldDef(name, type, [getter](void* instance) -> std::string {
            return getter(static_cast<T*>(instance));
        }));
        return *this;
    }
    
    // 字符串字段
    ResponseSchema& string(const std::string& name, std::function<std::string(T*)> getter) {
        return add(name, ResponseFieldType::STRING, getter);
    }
    
    // 整数字段
    ResponseSchema& integer8(const std::string& name, std::function<int8_t(T*)> getter) {
        return add(name, ResponseFieldType::INT8, [getter](T* instance) -> std::string {
            return std::to_string(getter(instance));
        });
    }
    
    ResponseSchema& integer16(const std::string& name, std::function<int16_t(T*)> getter) {
        return add(name, ResponseFieldType::INT16, [getter](T* instance) -> std::string {
            return std::to_string(getter(instance));
        });
    }
    
    ResponseSchema& integer32(const std::string& name, std::function<int32_t(T*)> getter) {
        return add(name, ResponseFieldType::INT32, [getter](T* instance) -> std::string {
            return std::to_string(getter(instance));
        });
    }
    
    // 64位整数字段
    ResponseSchema& integer64(const std::string& name, std::function<int64_t(T*)> getter) {
        return add(name, ResponseFieldType::INT64, [getter](T* instance) -> std::string {
            return std::to_string(getter(instance));
        });
    }
    
    // 无符号整数字段
    ResponseSchema& uinteger8(const std::string& name, std::function<uint8_t(T*)> getter) {
        return add(name, ResponseFieldType::UINT8, [getter](T* instance) -> std::string {
            return std::to_string(getter(instance));
        });
    }
    
    ResponseSchema& uinteger16(const std::string& name, std::function<uint16_t(T*)> getter) {
        return add(name, ResponseFieldType::UINT16, [getter](T* instance) -> std::string {
            return std::to_string(getter(instance));
        });
    }
    
    ResponseSchema& uinteger32(const std::string& name, std::function<uint32_t(T*)> getter) {
        return add(name, ResponseFieldType::UINT32, [getter](T* instance) -> std::string {
            return std::to_string(getter(instance));
        });
    }
    
    ResponseSchema& uinteger64(const std::string& name, std::function<uint64_t(T*)> getter) {
        return add(name, ResponseFieldType::UINT64, [getter](T* instance) -> std::string {
            return std::to_string(getter(instance));
        });
    }
    
    // 32位浮点数字段
    ResponseSchema& fp32(const std::string& name, std::function<float(T*)> getter) {
        return add(name, ResponseFieldType::FP32, [getter](T* instance) -> std::string {
            return std::to_string(getter(instance));
        });
    }
    
    // 64位浮点数字段
    ResponseSchema& fp64(const std::string& name, std::function<double(T*)> getter) {
        return add(name, ResponseFieldType::FP64, [getter](T* instance) -> std::string {
            return std::to_string(getter(instance));
        });
    }
    
    // 兼容旧代码的方法
    ResponseSchema& integer(const std::string& name, std::function<int32_t(T*)> getter) {
        return integer32(name, getter);
    }
    
    ResponseSchema& floating(const std::string& name, std::function<float(T*)> getter) {
        return fp32(name, getter);
    }
    
    ResponseSchema& number(const std::string& name, std::function<double(T*)> getter) {
        return fp64(name, getter);
    }
    
    // 布尔字段
    ResponseSchema& boolean(const std::string& name, std::function<bool(T*)> getter) {
        return add(name, ResponseFieldType::BOOL, [getter](T* instance) -> std::string {
            return getter(instance) ? "true" : "false";
        });
    }
    
    // 对象字段（JSON 字符串）
    ResponseSchema& object(const std::string& name, std::function<std::string(T*)> getter) {
        return add(name, ResponseFieldType::OBJECT, getter);
    }
    
    // 数组字段（JSON 字符串）
    ResponseSchema& array(const std::string& name, std::function<std::string(T*)> getter) {
        return add(name, ResponseFieldType::ARRAY, getter);
    }
    
    std::vector<ResponseFieldDef> fields() const override {
        return fields_;
    }
};

} // namespace uvapi

#endif // FRAMEWORK_TYPES_H