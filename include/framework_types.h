/**
 * @file framework_types.h
 * @brief RESTful 低代码框架基础类型定义（扩展类型）
 * 
 * 注意：核心类型定义在 framework.h 中，此文件只包含扩展类型和验证辅助函数
 */

#ifndef FRAMEWORK_TYPES_H
#define FRAMEWORK_TYPES_H

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
inline restful::ValidationResult validateEmail(const std::string& email) {
    // 简化的邮箱验证正则表达式
    static const std::regex email_regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!std::regex_match(email, email_regex)) {
        return restful::ValidationResult("Invalid email format");
    }
    return restful::ValidationResult::ok();
}

/**
 * @brief 验证 URL 格式
 * @param url URL 地址
 * @return 验证结果
 */
inline restful::ValidationResult validateUrl(const std::string& url) {
    // 简化的 URL 验证正则表达式
    static const std::regex url_regex(R"(^(https?|ftp)://[^\s/$.?#][^\s]*$)");
    if (!std::regex_match(url, url_regex)) {
        return restful::ValidationResult("Invalid URL format");
    }
    return restful::ValidationResult::ok();
}

/**
 * @brief 验证 UUID 格式
 * @param uuid UUID 字符串
 * @return 验证结果
 */
inline restful::ValidationResult validateUuid(const std::string& uuid) {
    // UUID 格式验证（8-4-4-4-12 格式）
    static const std::regex uuid_regex(R"(^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$)");
    if (!std::regex_match(uuid, uuid_regex)) {
        return restful::ValidationResult("Invalid UUID format");
    }
    return restful::ValidationResult::ok();
}

/**
 * @brief 验证日期格式 (YYYY-MM-DD)
 * @param date 日期字符串
 * @return 验证结果
 */
inline restful::ValidationResult validateDate(const std::string& date) {
    static const std::regex date_regex(R"(^\d{4}-\d{2}-\d{2}$)");
    if (!std::regex_match(date, date_regex)) {
        return restful::ValidationResult("Invalid date format, expected YYYY-MM-DD");
    }
    
    // 验证日期是否有效
    std::tm tm = {};
    std::istringstream ss(date);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) {
        return restful::ValidationResult("Invalid date value");
    }
    
    return restful::ValidationResult::ok();
}

/**
 * @brief 验证日期时间格式 (YYYY-MM-DD HH:MM:SS)
 * @param datetime 日期时间字符串
 * @return 验证结果
 */
inline restful::ValidationResult validateDatetime(const std::string& datetime) {
    static const std::regex datetime_regex(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}$)");
    if (!std::regex_match(datetime, datetime_regex)) {
        return restful::ValidationResult("Invalid datetime format, expected YYYY-MM-DD HH:MM:SS");
    }
    
    // 验证日期时间是否有效
    std::tm tm = {};
    std::istringstream ss(datetime);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        return restful::ValidationResult("Invalid datetime value");
    }
    
    return restful::ValidationResult::ok();
}

} // namespace validators

// ========== 自定义类型处理器 ==========

/**
 * @brief 自定义类型处理器基类
 * 
 * 用于处理框架内置类型之外的复杂类型
 */
class ICustomTypeHandler {
public:
    virtual ~ICustomTypeHandler() {}
    
    /**
     * @brief 序列化：从实例转换为 JSON 字符串
     * @param instance 实例指针
     * @param offset 字段偏移量
     * @return JSON 字符串
     */
    virtual std::string serialize(void* instance, size_t offset) const = 0;
    
    /**
     * @brief 反序列化：从 JSON 值转换为实例
     * @param json JSON 对象
     * @param instance 实例指针
     * @param offset 字段偏移量
     * @param error 错误信息输出
     * @return 是否成功
     */
    virtual bool deserialize(const cJSON* json, void* instance, size_t offset, std::string& error) const = 0;
    
    /**
     * @brief 验证：验证 JSON 值
     * @param json JSON 对象
     * @return 验证结果
     */
    virtual restful::ValidationResult validate(const cJSON* json) const = 0;
};

} // namespace uvapi

#endif // FRAMEWORK_TYPES_H