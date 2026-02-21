/**
 * @file builtin_types.h
 * @brief 内置高级数据类型处理器
 * 
 * 提供日期、日期时间、邮箱、URL、UUID 等内置高级数据类型的序列化、反序列化和验证功能。
 * 
 * ## 支持的类型
 * 
 * - **DATE** - 日期类型（YYYY-MM-DD）
 * - **DATETIME** - 日期时间类型（YYYY-MM-DD HH:MM:SS）
 * - **EMAIL** - 邮箱类型（自动验证邮箱格式）
 * - **URL** - URL 类型（自动验证 URL 格式）
 * - **UUID** - UUID 类型（自动验证 UUID 格式）
 * 
 * ## 使用示例
 * 
 * @code
 * #include "builtin_types.h"
 * #include "schema_dsl.h"
 * 
 * struct UserProfile {
 *     std::string email;
 *     std::string website;
 *     std::string user_id;  // UUID
 *     std::string birth_date;  // DATE
 *     std::string created_at;  // DATETIME
 * };
 * 
 * class ProfileSchema : public uvapi::DslBodySchema<UserProfile> {
 * public:
 *     void define() override {
 *         this->field(&UserProfile::email, "email").asEmail().required();
 *         this->field(&UserProfile::website, "website").asUrl().optional();
 *         this->field(&UserProfile::user_id, "user_id").asUuid().required();
 *         this->field(&UserProfile::birth_date, "birth_date").asDate().required();
 *         this->field(&UserProfile::created_at, "created_at").asDatetime().required();
 *     }
 * };
 * @endcode
 */

#ifndef BUILTIN_TYPES_H
#define BUILTIN_TYPES_H

#include "framework.h"
#include <regex>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <mutex>

namespace uvapi {

// ========== 验证辅助函数 ==========
// 验证函数已在 framework_types.h 中定义，此处不再重复定义

// ========== 内置类型处理器 ==========

/**
 * @brief 邮箱类型处理器
 */
class EmailTypeHandler : public ICustomTypeHandler {
public:
    std::string serialize(void* instance, size_t offset) const override {
        std::string* value = reinterpret_cast<std::string*>(reinterpret_cast<char*>(instance) + offset);
        return "\"" + *value + "\"";
    }
    
    bool deserialize(const cJSON* json, void* instance, size_t offset, std::string& error) const override {
        if (!cJSON_IsString(json)) {
            error = "Email field must be a string";
            return false;
        }
        
        std::string value = json->valuestring;
        ValidationResult result = validators::validateEmail(value);
        if (!result.success) {
            error = result.error_message;
            return false;
        }
        
        std::string* field = reinterpret_cast<std::string*>(reinterpret_cast<char*>(instance) + offset);
        *field = value;
        return true;
    }
    
    ValidationResult validate(const cJSON* json) const override {
        if (!cJSON_IsString(json)) {
            return ValidationResult("Email field must be a string");
        }
        return validators::validateEmail(json->valuestring);
    }
};

/**
 * @brief URL 类型处理器
 */
class UrlTypeHandler : public ICustomTypeHandler {
public:
    std::string serialize(void* instance, size_t offset) const override {
        std::string* value = reinterpret_cast<std::string*>(reinterpret_cast<char*>(instance) + offset);
        return "\"" + *value + "\"";
    }
    
    bool deserialize(const cJSON* json, void* instance, size_t offset, std::string& error) const override {
        if (!cJSON_IsString(json)) {
            error = "URL field must be a string";
            return false;
        }
        
        std::string value = json->valuestring;
        ValidationResult result = validators::validateUrl(value);
        if (!result.success) {
            error = result.error_message;
            return false;
        }
        
        std::string* field = reinterpret_cast<std::string*>(reinterpret_cast<char*>(instance) + offset);
        *field = value;
        return true;
    }
    
    ValidationResult validate(const cJSON* json) const override {
        if (!cJSON_IsString(json)) {
            return ValidationResult("URL field must be a string");
        }
        return validators::validateUrl(json->valuestring);
    }
};

/**
 * @brief UUID 类型处理器
 */
class UuidTypeHandler : public ICustomTypeHandler {
public:
    std::string serialize(void* instance, size_t offset) const override {
        std::string* value = reinterpret_cast<std::string*>(reinterpret_cast<char*>(instance) + offset);
        return "\"" + *value + "\"";
    }
    
    bool deserialize(const cJSON* json, void* instance, size_t offset, std::string& error) const override {
        if (!cJSON_IsString(json)) {
            error = "UUID field must be a string";
            return false;
        }
        
        std::string value = json->valuestring;
        ValidationResult result = validators::validateUuid(value);
        if (!result.success) {
            error = result.error_message;
            return false;
        }
        
        std::string* field = reinterpret_cast<std::string*>(reinterpret_cast<char*>(instance) + offset);
        *field = value;
        return true;
    }
    
    ValidationResult validate(const cJSON* json) const override {
        if (!cJSON_IsString(json)) {
            return ValidationResult("UUID field must be a string");
        }
        return validators::validateUuid(json->valuestring);
    }
};

/**
 * @brief 日期类型处理器（YYYY-MM-DD）
 */
class DateTypeHandler : public ICustomTypeHandler {
public:
    std::string serialize(void* instance, size_t offset) const override {
        std::string* value = reinterpret_cast<std::string*>(reinterpret_cast<char*>(instance) + offset);
        return "\"" + *value + "\"";
    }
    
    bool deserialize(const cJSON* json, void* instance, size_t offset, std::string& error) const override {
        if (!cJSON_IsString(json)) {
            error = "Date field must be a string";
            return false;
        }
        
        std::string value = json->valuestring;
        ValidationResult result = validators::validateDate(value);
        if (!result.success) {
            error = result.error_message;
            return false;
        }
        
        std::string* field = reinterpret_cast<std::string*>(reinterpret_cast<char*>(instance) + offset);
        *field = value;
        return true;
    }
    
    ValidationResult validate(const cJSON* json) const override {
        if (!cJSON_IsString(json)) {
            return ValidationResult("Date field must be a string");
        }
        return validators::validateDate(json->valuestring);
    }
};

/**
 * @brief 日期时间类型处理器（YYYY-MM-DD HH:MM:SS）
 */
class DatetimeTypeHandler : public ICustomTypeHandler {
public:
    std::string serialize(void* instance, size_t offset) const override {
        std::string* value = reinterpret_cast<std::string*>(reinterpret_cast<char*>(instance) + offset);
        return "\"" + *value + "\"";
    }
    
    bool deserialize(const cJSON* json, void* instance, size_t offset, std::string& error) const override {
        if (!cJSON_IsString(json)) {
            error = "Datetime field must be a string";
            return false;
        }
        
        std::string value = json->valuestring;
        ValidationResult result = validators::validateDatetime(value);
        if (!result.success) {
            error = result.error_message;
            return false;
        }
        
        std::string* field = reinterpret_cast<std::string*>(reinterpret_cast<char*>(instance) + offset);
        *field = value;
        return true;
    }
    
    ValidationResult validate(const cJSON* json) const override {
        if (!cJSON_IsString(json)) {
            return ValidationResult("Datetime field must be a string");
        }
        return validators::validateDatetime(json->valuestring);
    }
};

// ========== 类型处理器注册表 ==========

class TypeHandlerRegistry {
private:
    static std::map<FieldType, std::unique_ptr<ICustomTypeHandler>> handlers_;
    static std::once_flag init_flag_;
    
    static void doInit() {
        handlers_[FieldType::EMAIL].reset(new EmailTypeHandler());
        handlers_[FieldType::URL].reset(new UrlTypeHandler());
        handlers_[FieldType::UUID].reset(new UuidTypeHandler());
        handlers_[FieldType::DATE].reset(new DateTypeHandler());
        handlers_[FieldType::DATETIME].reset(new DatetimeTypeHandler());
    }
    
public:
    static void init() {
        std::call_once(init_flag_, doInit);
    }
    
    static ICustomTypeHandler* get(FieldType type) {
        init();
        
        auto it = handlers_.find(type);
        if (it != handlers_.end()) {
            return it->second.get();
        }
        return nullptr;
    }
    
    static void registerCustom(FieldType type, std::unique_ptr<ICustomTypeHandler> handler) {
        init();
        handlers_[type] = std::move(handler);
    }
};

// 静态成员初始化
std::map<FieldType, std::unique_ptr<ICustomTypeHandler>> TypeHandlerRegistry::handlers_;
std::once_flag TypeHandlerRegistry::init_flag_;

} // namespace uvapi

#endif // BUILTIN_TYPES_H
