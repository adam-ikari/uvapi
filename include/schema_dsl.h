/**
 * @file cpp11_schema_final.h
 * @brief C++11 Schema DSL - 使用闭包捕获避免全局变量
 */

#ifndef CPP11_SCHEMA_FINAL_H
#define CPP11_SCHEMA_FINAL_H

#include <string>
#include <vector>
#include <functional>
#include <cstring>
#include <cJSON.h>

namespace uvapi {

// ========== 类型枚举 ==========

enum class FieldType {
    STRING,
    INT,
    INT64,
    FLOAT,
    DOUBLE,
    BOOL,
    ARRAY,
    OBJECT
};

// ========== 验证规则 ==========

struct ValidationRules {
    bool required;
    bool has_min_length;
    size_t min_length;
    bool has_max_length;
    size_t max_length;
    bool has_min_value;
    double min_value;
    bool has_max_value;
    double max_value;
    std::string pattern;
    std::vector<std::string> enum_values;
    
    ValidationRules()
        : required(false), has_min_length(false), min_length(0),
          has_max_length(false), max_length(0),
          has_min_value(false), min_value(0),
          has_max_value(false), max_value(0) {}
    
    void setMinLength(size_t len) {
        has_min_length = true;
        min_length = len;
    }
    
    void setMaxLength(size_t len) {
        has_max_length = true;
        max_length = len;
    }
    
    void setMinValue(double val) {
        has_min_value = true;
        min_value = val;
    }
    
    void setMaxValue(double val) {
        has_max_value = true;
        max_value = val;
    }
    
    void setPattern(const std::string& p) {
        pattern = p;
    }
    
    void setEnum(const std::vector<std::string>& vals) {
        enum_values = vals;
    }
};

// ========== 字段定义 ==========

template<typename T>
struct FieldDef {
    std::string name;
    FieldType type;
    std::function<std::string(T*)> getter;
    std::function<void(T*, const std::string&)> setter;
    ValidationRules validation;
    
    FieldDef(const std::string& n, FieldType t,
             const std::function<std::string(T*)>& g,
             const std::function<void(T*, const std::string&)>& s)
        : name(n), type(t), getter(g), setter(s) {}
};

// ========== Schema 类 ==========

template<typename T>
class Schema {
private:
    std::vector<FieldDef<T>> fields_;
    std::function<std::string(T*)> custom_validator_;  // 自定义整体校验函数
    
public:
    Schema() {}
    
    // 字段定义方法
    Schema& string(const std::string& name, std::string T::*member) {
        fields_.push_back(FieldDef<T>(name, FieldType::STRING,
            [member](T* obj) { return obj->*member; },
            [member](T* obj, const std::string& val) { obj->*member = val; }));
        return *this;
    }
    
    Schema& integer(const std::string& name, int T::*member) {
        fields_.push_back(FieldDef<T>(name, FieldType::INT,
            [member](T* obj) { return std::to_string(obj->*member); },
            [member](T* obj, const std::string& val) { obj->*member = std::stoi(val); }));
        return *this;
    }
    
    Schema& integer64(const std::string& name, int64_t T::*member) {
        fields_.push_back(FieldDef<T>(name, FieldType::INT64,
            [member](T* obj) { return std::to_string(obj->*member); },
            [member](T* obj, const std::string& val) { obj->*member = std::stoll(val); }));
        return *this;
    }
    
    Schema& number(const std::string& name, double T::*member) {
        fields_.push_back(FieldDef<T>(name, FieldType::DOUBLE,
            [member](T* obj) { return std::to_string(obj->*member); },
            [member](T* obj, const std::string& val) { obj->*member = std::stod(val); }));
        return *this;
    }
    
    Schema& boolean(const std::string& name, bool T::*member) {
        fields_.push_back(FieldDef<T>(name, FieldType::BOOL,
            [member](T* obj) { return obj->*member ? "true" : "false"; },
            [member](T* obj, const std::string& val) { obj->*member = (val == "true" || val == "1"); }));
        return *this;
    }
    
    // 设置自定义整体校验函数（通过闭包捕获 this）
    Schema& validateBody(std::function<std::string(T*)> validator) {
        custom_validator_ = validator;
        return *this;
    }
    
    // 验证规则
    Schema& required() {
        if (!fields_.empty()) {
            fields_.back().validation.required = true;
        }
        return *this;
    }
    
    Schema& length(size_t min_len, size_t max_len) {
        if (!fields_.empty()) {
            fields_.back().validation.setMinLength(min_len);
            fields_.back().validation.setMaxLength(max_len);
        }
        return *this;
    }
    
    Schema& minLength(size_t len) {
        if (!fields_.empty()) {
            fields_.back().validation.setMinLength(len);
        }
        return *this;
    }
    
    Schema& maxLength(size_t len) {
        if (!fields_.empty()) {
            fields_.back().validation.setMaxLength(len);
        }
        return *this;
    }
    
    Schema& range(double min_val, double max_val) {
        if (!fields_.empty()) {
            fields_.back().validation.setMinValue(min_val);
            fields_.back().validation.setMaxValue(max_val);
        }
        return *this;
    }
    
    Schema& min(double val) {
        if (!fields_.empty()) {
            fields_.back().validation.setMinValue(val);
        }
        return *this;
    }
    
    Schema& max(double val) {
        if (!fields_.empty()) {
            fields_.back().validation.setMaxValue(val);
        }
        return *this;
    }
    
    Schema& pattern(const std::string& p) {
        if (!fields_.empty()) {
            fields_.back().validation.setPattern(p);
        }
        return *this;
    }
    
    Schema& oneOf(const std::vector<std::string>& vals) {
        if (!fields_.empty()) {
            fields_.back().validation.setEnum(vals);
        }
        return *this;
    }
    
    // 单个字段校验方法（供用户在重载的 validateBody 中逐个调用）
    std::string validateField(const std::string& field_name, T* obj) const {
        for (typename std::vector<FieldDef<T>>::const_iterator it = fields_.begin();
             it != fields_.end(); ++it) {
            const FieldDef<T>& field = *it;
            if (field.name == field_name) {
                std::string value = field.getter(obj);
                
                // 创建临时 cJSON 对象用于验证
                cJSON* item = nullptr;
                switch (field.type) {
                    case FieldType::STRING:
                        item = cJSON_CreateString(value.c_str());
                        break;
                    case FieldType::INT:
                        item = cJSON_CreateNumber(std::stoi(value));
                        break;
                    case FieldType::INT64:
                        item = cJSON_CreateNumber(std::stoll(value));
                        break;
                    case FieldType::FLOAT:
                        item = cJSON_CreateNumber(std::stof(value));
                        break;
                    case FieldType::DOUBLE:
                        item = cJSON_CreateNumber(std::stod(value));
                        break;
                    case FieldType::BOOL:
                        item = cJSON_CreateBool(value == "true" || value == "1");
                        break;
                    default:
                        item = cJSON_CreateString(value.c_str());
                        break;
                }
                
                if (item) {
                    std::string error = validateField(item, field);
                    cJSON_Delete(item);
                    return error;
                }
                return "Failed to create validation item";
            }
        }
        return "Field '" + field_name + "' not found in schema";
    }
    
    // 字段校验方法（供 validateBody 调用）
    std::string validateFields(T* obj) const {
        std::string json_str = toJson(obj);
        cJSON* json = cJSON_Parse(json_str.c_str());
        if (!json || !cJSON_IsObject(json)) {
            if (json) cJSON_Delete(json);
            return "Failed to serialize object to JSON";
        }
        
        for (typename std::vector<FieldDef<T>>::const_iterator it = fields_.begin();
             it != fields_.end(); ++it) {
            const FieldDef<T>& field = *it;
            std::string value = field.getter(obj);
            
            // 创建临时 cJSON 对象用于验证
            cJSON* item = nullptr;
            switch (field.type) {
                case FieldType::STRING:
                    item = cJSON_CreateString(value.c_str());
                    break;
                case FieldType::INT:
                    item = cJSON_CreateNumber(std::stoi(value));
                    break;
                case FieldType::INT64:
                    item = cJSON_CreateNumber(std::stoll(value));
                    break;
                case FieldType::FLOAT:
                    item = cJSON_CreateNumber(std::stof(value));
                    break;
                case FieldType::DOUBLE:
                    item = cJSON_CreateNumber(std::stod(value));
                    break;
                case FieldType::BOOL:
                    item = cJSON_CreateBool(value == "true" || value == "1");
                    break;
                default:
                    item = cJSON_CreateString(value.c_str());
                    break;
            }
            
            if (item) {
                std::string error = validateField(item, field);
                if (!error.empty()) {
                    cJSON_Delete(item);
                    cJSON_Delete(json);
                    return error;
                }
                cJSON_Delete(item);
            }
        }
        
        cJSON_Delete(json);
        return "";
    }
    
    // 序列化
    std::string toJson(T* obj) const {
        cJSON* root = cJSON_CreateObject();
        if (!root) return "{}";
        
        for (typename std::vector<FieldDef<T>>::const_iterator it = fields_.begin();
             it != fields_.end(); ++it) {
            const FieldDef<T>& field = *it;
            std::string value = field.getter(obj);
            
            cJSON* item = nullptr;
            switch (field.type) {
                case FieldType::STRING:
                    item = cJSON_CreateString(value.c_str());
                    break;
                case FieldType::INT:
                    item = cJSON_CreateNumber(std::stoi(value));
                    break;
                case FieldType::INT64:
                    item = cJSON_CreateNumber(std::stoll(value));
                    break;
                case FieldType::FLOAT:
                    item = cJSON_CreateNumber(std::stof(value));
                    break;
                case FieldType::DOUBLE:
                    item = cJSON_CreateNumber(std::stod(value));
                    break;
                case FieldType::BOOL:
                    item = cJSON_CreateBool(value == "true" || value == "1");
                    break;
                default:
                    item = cJSON_CreateString(value.c_str());
                    break;
            }
            
            if (item) {
                cJSON_AddItemToObject(root, field.name.c_str(), item);
            }
        }
        
        char* json = cJSON_Print(root);
        std::string result(json ? json : "{}");
        if (json) free(json);
        cJSON_Delete(root);
        
        return result;
    }
    
    // 反序列化
    bool fromJson(const std::string& json_str, T* obj) const {
        cJSON* json = cJSON_Parse(json_str.c_str());
        if (!json || !cJSON_IsObject(json)) {
            if (json) cJSON_Delete(json);
            return false;
        }
        
        for (typename std::vector<FieldDef<T>>::const_iterator it = fields_.begin();
             it != fields_.end(); ++it) {
            const FieldDef<T>& field = *it;
            cJSON* item = cJSON_GetObjectItem(json, field.name.c_str());
            
            if (item) {
                const char* value = nullptr;
                if (cJSON_IsString(item)) {
                    value = item->valuestring;
                } else if (cJSON_IsNumber(item)) {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%g", item->valuedouble);
                    value = buf;
                } else if (cJSON_IsBool(item)) {
                    value = cJSON_IsTrue(item) ? "true" : "false";
                }
                
                if (value) {
                    field.setter(obj, value);
                }
            }
        }
        
        cJSON_Delete(json);
        return true;
    }
    
    // 验证 JSON 字符串
    std::string validate(const std::string& json_str) const {
        // 1. 解析 JSON
        cJSON* json = cJSON_Parse(json_str.c_str());
        if (!json || !cJSON_IsObject(json)) {
            if (json) cJSON_Delete(json);
            return "Invalid JSON object";
        }
        
        // 2. 字段级校验
        for (typename std::vector<FieldDef<T>>::const_iterator it = fields_.begin();
             it != fields_.end(); ++it) {
            const FieldDef<T>& field = *it;
            cJSON* item = cJSON_GetObjectItem(json, field.name.c_str());
            
            // 检查必填字段
            if (field.validation.required && !item) {
                cJSON_Delete(json);
                return "Field '" + field.name + "' is required";
            }
            
            if (!item) continue;
            
            // 验证字段值
            std::string error = validateField(item, field);
            if (!error.empty()) {
                cJSON_Delete(json);
                return error;
            }
        }
        
        cJSON_Delete(json);
        
        // 3. 反序列化到对象
        T obj;
        bool success = fromJson(json_str, &obj);
        if (!success) {
            return "Failed to parse JSON to object";
        }
        
        // 4. 整体校验
        return validateBody(&obj);
    }
    
    // 整体校验入口函数
    std::string validateBody(T* obj) const {
        // 如果有自定义校验函数，使用自定义校验
        if (custom_validator_) {
            return custom_validator_(obj);
        }
        
        // 默认实现：调用字段校验
        return validateFields(obj);
    }
    
    // 直接验证对象（性能优化：避免序列化再解析）
    std::string validateObject(T* obj) const {
        // 1. 字段级校验（直接从对象读取）
        for (typename std::vector<FieldDef<T>>::const_iterator it = fields_.begin();
             it != fields_.end(); ++it) {
            const FieldDef<T>& field = *it;
            std::string value = field.getter(obj);
            
            // 检查必填字段
            if (field.validation.required && value.empty()) {
                return "Field '" + field.name + "' is required";
            }
            
            if (value.empty()) continue;
            
            // 验证字段值
            std::string error = validateObjectField(value, field);
            if (!error.empty()) {
                return error;
            }
        }
        
        // 2. 整体校验
        return validateBody(obj);
    }
    
private:
    std::string validateField(cJSON* item, const FieldDef<T>& field) const {
        const char* value = nullptr;
        
        if (cJSON_IsString(item)) {
            value = item->valuestring;
            size_t len = strlen(value);
            
            if (field.validation.has_min_length && len < field.validation.min_length) {
                return "Field '" + field.name + "' must be at least " + 
                       std::to_string(field.validation.min_length) + " characters";
            }
            
            if (field.validation.has_max_length && len > field.validation.max_length) {
                return "Field '" + field.name + "' must be at most " + 
                       std::to_string(field.validation.max_length) + " characters";
            }
            
            // 枚举验证
            if (!field.validation.enum_values.empty()) {
                bool found = false;
                for (size_t i = 0; i < field.validation.enum_values.size(); ++i) {
                    if (field.validation.enum_values[i] == value) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return "Field '" + field.name + "' must be one of: " + 
                           joinEnumValues(field.validation.enum_values);
                }
            }
        } else if (cJSON_IsNumber(item)) {
            double num = item->valuedouble;
            
            if (field.validation.has_min_value && num < field.validation.min_value) {
                return "Field '" + field.name + "' must be at least " + 
                       std::to_string(field.validation.min_value);
            }
            
            if (field.validation.has_max_value && num > field.validation.max_value) {
                return "Field '" + field.name + "' must be at most " + 
                       std::to_string(field.validation.max_value);
            }
        }
        
        return "";
    }
    
    // 验证对象字段（性能优化：直接验证字符串值）
    std::string validateObjectField(const std::string& value, const FieldDef<T>& field) const {
        size_t len = value.length();
        
        // 字符串类型验证
        if (field.type == FieldType::STRING) {
            if (field.validation.has_min_length && len < field.validation.min_length) {
                return "Field '" + field.name + "' must be at least " + 
                       std::to_string(field.validation.min_length) + " characters";
            }
            
            if (field.validation.has_max_length && len > field.validation.max_length) {
                return "Field '" + field.name + "' must be at most " + 
                       std::to_string(field.validation.max_length) + " characters";
            }
            
            // 枚举验证
            if (!field.validation.enum_values.empty()) {
                bool found = false;
                for (size_t i = 0; i < field.validation.enum_values.size(); ++i) {
                    if (field.validation.enum_values[i] == value) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return "Field '" + field.name + "' must be one of: " + 
                           joinEnumValues(field.validation.enum_values);
                }
            }
        }
        // 数值类型验证
        else if (field.type == FieldType::INT || field.type == FieldType::INT64 ||
                 field.type == FieldType::FLOAT || field.type == FieldType::DOUBLE) {
            double num = 0.0;
            try {
                if (field.type == FieldType::INT) {
                    num = static_cast<double>(std::stoi(value));
                } else if (field.type == FieldType::INT64) {
                    num = static_cast<double>(std::stoll(value));
                } else if (field.type == FieldType::FLOAT) {
                    num = static_cast<double>(std::stof(value));
                } else {
                    num = std::stod(value);
                }
            } catch (...) {
                return "Field '" + field.name + "' must be a valid number";
            }
            
            if (field.validation.has_min_value && num < field.validation.min_value) {
                return "Field '" + field.name + "' must be at least " + 
                       std::to_string(field.validation.min_value);
            }
            
            if (field.validation.has_max_value && num > field.validation.max_value) {
                return "Field '" + field.name + "' must be at most " + 
                       std::to_string(field.validation.max_value);
            }
        }
        
        return "";
    }
    
    std::string joinEnumValues(const std::vector<std::string>& values) const {
        if (values.empty()) return "";
        
        std::string result = values[0];
        for (size_t i = 1; i < values.size(); ++i) {
            result += ", " + values[i];
        }
        return result;
    }
};

// ========== Schema 构建器辅助函数 ==========

template<typename T>
Schema<T> makeSchema() {
    return Schema<T>();
}

} // namespace uvapi

#endif // CPP11_SCHEMA_FINAL_H
