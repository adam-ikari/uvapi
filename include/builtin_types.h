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

namespace uvapi {

// ========== 验证辅助函数 ==========

namespace validators {

/**
 * @brief 验证邮箱格式
 * @param email 邮箱地址
 * @return 验证结果
 */
inline ValidationResult validateEmail(const std::string& email) {
    // 简化的邮箱验证正则表达式
    static const std::regex email_regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!std::regex_match(email, email_regex)) {
        return ValidationResult("Invalid email format");
    }
    return ValidationResult::ok();
}

/**
 * @brief 验证 URL 格式
 * @param url URL 地址
 * @return 验证结果
 */
inline ValidationResult validateUrl(const std::string& url) {
    // 简化的 URL 验证正则表达式
    static const std::regex url_regex(R"(^(https?|ftp)://[^\s/$.?#][^\s]*$)");
    if (!std::regex_match(url, url_regex)) {
        return ValidationResult("Invalid URL format");
    }
    return ValidationResult::ok();
}

/**
 * @brief 验证 UUID 格式
 * @param uuid UUID 字符串
 * @return 验证结果
 */
inline ValidationResult validateUuid(const std::string& uuid) {
    // UUID v4 格式：xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    static const std::regex uuid_regex(R"(^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-4[0-9a-fA-F]{3}-[89abAB][0-9a-fA-F]{3}-[0-9a-fA-F]{12}$)");
    if (!std::regex_match(uuid, uuid_regex)) {
        return ValidationResult("Invalid UUID format (expected format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx)");
    }
    return ValidationResult::ok();
}

/**
 * @brief 验证日期格式（YYYY-MM-DD）
 * @param date 日期字符串
 * @return 验证结果
 */
inline ValidationResult validateDate(const std::string& date) {
    static const std::regex date_regex(R"(^\d{4}-\d{2}-\d{2}$)");
    if (!std::regex_match(date, date_regex)) {
        return ValidationResult("Invalid date format (expected format: YYYY-MM-DD)");
    }
    
    // 验证日期是否有效
    int year, month, day;
    char dash1, dash2;
    std::istringstream iss(date);
    iss >> year >> dash1 >> month >> dash2 >> day;
    
    if (dash1 != '-' || dash2 != '-') {
        return ValidationResult("Invalid date format");
    }
    
    if (month < 1 || month > 12) {
        return ValidationResult("Invalid month");
    }
    
    if (day < 1 || day > 31) {
        return ValidationResult("Invalid day");
    }
    
    // 检查月份的天数
    static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int max_days = days_in_month[month - 1];
    
    // 闰年二月
    if (month == 2 && ((year % 400 == 0) || (year % 100 != 0 && year % 4 == 0))) {
        max_days = 29;
    }
    
    if (day > max_days) {
        return ValidationResult("Invalid day for the given month");
    }
    
    return ValidationResult::ok();
}

/**
 * @brief 验证日期时间格式（YYYY-MM-DD HH:MM:SS）
 * @param datetime 日期时间字符串
 * @return 验证结果
 */
inline ValidationResult validateDatetime(const std::string& datetime) {
    static const std::regex datetime_regex(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}$)");
    if (!std::regex_match(datetime, datetime_regex)) {
        return ValidationResult("Invalid datetime format (expected format: YYYY-MM-DD HH:MM:SS)");
    }
    
    // 分离日期和时间部分
    std::string date_part = datetime.substr(0, 10);
    std::string time_part = datetime.substr(11, 8);
    
    // 验证日期部分
    ValidationResult date_result = validateDate(date_part);
    if (!date_result.success) {
        return date_result;
    }
    
    // 验证时间部分
    int hour, minute, second;
    char colon1, colon2;
    std::istringstream iss(time_part);
    iss >> hour >> colon1 >> minute >> colon2 >> second;
    
    if (colon1 != ':' || colon2 != ':') {
        return ValidationResult("Invalid time format");
    }
    
    if (hour < 0 || hour > 23) {
        return ValidationResult("Invalid hour");
    }
    
    if (minute < 0 || minute > 59) {
        return ValidationResult("Invalid minute");
    }
    
    if (second < 0 || second > 59) {
        return ValidationResult("Invalid second");
    }
    
    return ValidationResult::ok();
}

} // namespace validators

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
    
    std::string validate(const cJSON* json) const override {
        if (!cJSON_IsString(json)) {
            return "Email field must be a string";
        }
        ValidationResult result = validators::validateEmail(json->valuestring);
        if (!result.success) {
            return result.error_message;
        }
        return "";
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
    
    std::string validate(const cJSON* json) const override {
        if (!cJSON_IsString(json)) {
            return "URL field must be a string";
        }
        ValidationResult result = validators::validateUrl(json->valuestring);
        if (!result.success) {
            return result.error_message;
        }
        return "";
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
    
    std::string validate(const cJSON* json) const override {
        if (!cJSON_IsString(json)) {
            return "UUID field must be a string";
        }
        ValidationResult result = validators::validateUuid(json->valuestring);
        if (!result.success) {
            return result.error_message;
        }
        return "";
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
    
    std::string validate(const cJSON* json) const override {
        if (!cJSON_IsString(json)) {
            return "Date field must be a string";
        }
        ValidationResult result = validators::validateDate(json->valuestring);
        if (!result.success) {
            return result.error_message;
        }
        return "";
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
    
    std::string validate(const cJSON* json) const override {
        if (!cJSON_IsString(json)) {
            return "Datetime field must be a string";
        }
        ValidationResult result = validators::validateDatetime(json->valuestring);
        if (!result.success) {
            return result.error_message;
        }
        return "";
    }
};

// ========== 类型处理器注册表 ==========

class TypeHandlerRegistry {
private:
    static std::map<FieldType, std::unique_ptr<ICustomTypeHandler>> handlers_;
    static bool initialized_;
    
public:
    static void init() {
        if (initialized_) return;
        
        handlers_[FieldType::EMAIL] = std::make_unique<EmailTypeHandler>();
        handlers_[FieldType::URL] = std::make_unique<UrlTypeHandler>();
        handlers_[FieldType::UUID] = std::make_unique<UuidTypeHandler>();
        handlers_[FieldType::DATE] = std::make_unique<DateTypeHandler>();
        handlers_[FieldType::DATETIME] = std::make_unique<DatetimeTypeHandler>();
        
        initialized_ = true;
    }
    
    static ICustomTypeHandler* get(FieldType type) {
        if (!initialized_) {
            init();
        }
        
        auto it = handlers_.find(type);
        if (it != handlers_.end()) {
            return it->second.get();
        }
        return nullptr;
    }
    
    static void registerCustom(FieldType type, std::unique_ptr<ICustomTypeHandler> handler) {
        handlers_[type] = std::move(handler);
    }
};

// 静态成员初始化
std::map<FieldType, std::unique_ptr<ICustomTypeHandler>> TypeHandlerRegistry::handlers_;
bool TypeHandlerRegistry::initialized_ = false;

} // namespace uvapi

#endif // BUILTIN_TYPES_H
