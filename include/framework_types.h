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

// 字段类型枚举
enum class FieldType {
    STRING,
    INT,
    INT64,
    FLOAT,
    DOUBLE,
    BOOL,
    ARRAY,
    OBJECT,
    CUSTOM
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
    
    FieldDefinition(const std::string& n, FieldType t, size_t o)
        : name(n), type(t), offset(o), is_optional(false), 
          nested_schema(nullptr), item_schema(nullptr) {}
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

// Response 字段类型
enum class ResponseFieldType {
    STRING,
    INT,
    INT64,
    FLOAT,
    DOUBLE,
    BOOL,
    ARRAY,
    OBJECT
};

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
                case ResponseFieldType::INT:
                    field_json = cJSON_CreateNumber(std::stoi(value));
                    break;
                case ResponseFieldType::INT64:
                    field_json = cJSON_CreateNumber(std::stoll(value));
                    break;
                case ResponseFieldType::FLOAT:
                    field_json = cJSON_CreateNumber(std::stof(value));
                    break;
                case ResponseFieldType::DOUBLE:
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
    ResponseSchema& integer(const std::string& name, std::function<int(T*)> getter) {
        return add(name, ResponseFieldType::INT, [getter](T* instance) -> std::string {
            return std::to_string(getter(instance));
        });
    }
    
    // 64位整数字段
    ResponseSchema& integer64(const std::string& name, std::function<int64_t(T*)> getter) {
        return add(name, ResponseFieldType::INT64, [getter](T* instance) -> std::string {
            return std::to_string(getter(instance));
        });
    }
    
    // 浮点数字段
    ResponseSchema& floating(const std::string& name, std::function<float(T*)> getter) {
        return add(name, ResponseFieldType::FLOAT, [getter](T* instance) -> std::string {
            return std::to_string(getter(instance));
        });
    }
    
    // 双精度字段
    ResponseSchema& number(const std::string& name, std::function<double(T*)> getter) {
        return add(name, ResponseFieldType::DOUBLE, [getter](T* instance) -> std::string {
            return std::to_string(getter(instance));
        });
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