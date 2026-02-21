/**
 * @file params_dsl.h
 * @brief 参数声明 DSL - 增强版参数定义系统
 *
 * 提供更完善的参数类型声明、验证和访问功能，支持多种数据类型和验证规则。
 *
 * 主要功能:
 * 1. 参数类型定义 - 支持 STRING, INTEGER, DOUBLE, BOOLEAN, EMAIL, URL, UUID 等类型
 * 2. 验证规则 - 支持必填/可选、长度、范围、正则、枚举等验证
 * 3. 参数访问 - 提供类型安全的参数访问方法
 * 4. 便捷函数 - 提供常用参数定义（分页、搜索、时间范围等）
 *
 * 使用示例:
 * - 基本参数定义: EnhancedParamBuilder builder("username", ParamType::QUERY, ParamDataType::STRING);
 *   builder.required().minLength(3).maxLength(20);
 * - 参数访问: ParamAccessor params(req); int page = params.getQueryInt("page", 1);
 * - 参数验证: std::string error = ParamValidator::validate(param, value);
 * - 批量验证: std::string error = ParamValidator::validateAll(params, req.query_params);
 *
 * 支持的数据类型: STRING, INT8, INT16, INT32, INT64, UINT8, UINT16, UINT32, UINT64,
 * FP32, FP64, BOOLEAN, DATE, DATETIME, EMAIL, URL, UUID
 *
 * 支持的验证规则: required() / optional(), minLength() / maxLength(),
 * min() / max() / range(), pattern(), oneOf()
 */

#ifndef PARAMS_DSL_H
#define PARAMS_DSL_H

#include <string>
#include <functional>
#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <regex>
#include "framework.h"

namespace uvapi {

// ========== 参数类型定义 ==========

enum class ParamDataType {
    STRING,      // 字符串
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
    BOOLEAN,     // 布尔值
    DATE,        // 日期
    DATETIME,    // 日期时间
    EMAIL,       // 邮箱
    URL,         // URL
    UUID         // UUID
};

// 兼容旧代码的类型别名
using INTEGER = int;
using INTEGER64 = int64_t;
using DOUBLE = double;
using FLOAT = float;

// 参数类型信息
struct ParamTypeInfo {
    ParamDataType data_type;
    std::string type_name;
    
    ParamTypeInfo(ParamDataType type, const std::string& name) 
        : data_type(type), type_name(name) {}
};

// ========== 增强的参数构建器 ==========

class EnhancedParamBuilder {
private:
    restful::ParamDefinition param_;
    ParamDataType data_type_;
    std::string description_;
    std::string example_;
    
public:
    EnhancedParamBuilder(const std::string& name, restful::ParamType param_type, ParamDataType data_type)
        : param_(name, param_type), data_type_(data_type) {}
    
    // ========== 类型设置 ==========
    
    // 字符串类型
    EnhancedParamBuilder& asString() {
        data_type_ = ParamDataType::STRING;
        return *this;
    }
    
    // 整数类型
    EnhancedParamBuilder& asInt() {
        data_type_ = ParamDataType::INT32;
        return *this;
    }

    // 64位整数类型
    EnhancedParamBuilder& asInt64() {
        data_type_ = ParamDataType::INT64;
        return *this;
    }

    // 浮点类型
    EnhancedParamBuilder& asDouble() {
        data_type_ = ParamDataType::FP64;
        return *this;
    }

    // 浮点类型
    EnhancedParamBuilder& asFloat() {
        data_type_ = ParamDataType::FP32;
        return *this;
    }
    
    // 布尔类型
    EnhancedParamBuilder& asBool() {
        data_type_ = ParamDataType::BOOLEAN;
        return *this;
    }
    
    // 邮箱类型
    EnhancedParamBuilder& asEmail() {
        data_type_ = ParamDataType::EMAIL;
        return *this;
    }
    
    // URL 类型
    EnhancedParamBuilder& asUrl() {
        data_type_ = ParamDataType::URL;
        return *this;
    }
    
    // UUID 类型
    EnhancedParamBuilder& asUuid() {
        data_type_ = ParamDataType::UUID;
        return *this;
    }
    
    // ========== 默认值 ==========
    
    EnhancedParamBuilder& defaultValue(const std::string& value) {
        param_.default_value = value;
        return *this;
    }
    
    EnhancedParamBuilder& defaultValue(int value) {
        param_.default_value = std::to_string(value);
        return *this;
    }
    
    EnhancedParamBuilder& defaultValue(int64_t value) {
        param_.default_value = std::to_string(value);
        return *this;
    }
    
    EnhancedParamBuilder& defaultValue(double value) {
        param_.default_value = std::to_string(value);
        return *this;
    }
    
    EnhancedParamBuilder& defaultValue(bool value) {
        param_.default_value = value ? "true" : "false";
        return *this;
    }
    
    // ========== 验证规则 ==========
    
    EnhancedParamBuilder& required() {
        param_.validation.required = true;
        return *this;
    }
    
    EnhancedParamBuilder& optional() {
        param_.validation.required = false;
        return *this;
    }
    
    // 字符串长度验证
    EnhancedParamBuilder& minLength(size_t len) {
        param_.validation.min_length = len;
        param_.validation.has_min_length = true;
        return *this;
    }
    
    EnhancedParamBuilder& maxLength(size_t len) {
        param_.validation.max_length = len;
        param_.validation.has_max_length = true;
        return *this;
    }
    
    EnhancedParamBuilder& length(size_t min_len, size_t max_len) {
        return minLength(min_len).maxLength(max_len);
    }
    
    // 数值范围验证
    EnhancedParamBuilder& min(int val) {
        param_.validation.min_value = val;
        param_.validation.has_min = true;
        return *this;
    }
    
    EnhancedParamBuilder& max(int val) {
        param_.validation.max_value = val;
        param_.validation.has_max = true;
        return *this;
    }
    
    EnhancedParamBuilder& range(int min_val, int max_val) {
        return min(min_val).max(max_val);
    }
    
    EnhancedParamBuilder& min(double val) {
        param_.validation.min_double = val;
        param_.validation.has_min = true;
        return *this;
    }
    
    EnhancedParamBuilder& max(double val) {
        param_.validation.max_double = val;
        param_.validation.has_max = true;
        return *this;
    }
    
    EnhancedParamBuilder& range(double min_val, double max_val) {
        return min(min_val).max(max_val);
    }
    
    // 正则表达式验证
    EnhancedParamBuilder& pattern(const std::string& regex) {
        param_.validation.pattern = regex;
        param_.validation.has_pattern = true;
        return *this;
    }
    
    // 枚举值验证
    EnhancedParamBuilder& oneOf(const std::vector<std::string>& values) {
        param_.validation.enum_values = values;
        param_.validation.has_enum = true;
        return *this;
    }
    
    EnhancedParamBuilder& oneOf(const std::string& v1, const std::string& v2, 
                                 const std::string& v3 = "", const std::string& v4 = "",
                                 const std::string& v5 = "") {
        std::vector<std::string> values;
        if (!v1.empty()) values.push_back(v1);
        if (!v2.empty()) values.push_back(v2);
        if (!v3.empty()) values.push_back(v3);
        if (!v4.empty()) values.push_back(v4);
        if (!v5.empty()) values.push_back(v5);
        return oneOf(values);
    }
    
    // ========== 文档元数据 ==========
    
    EnhancedParamBuilder& description(const std::string& desc) {
        description_ = desc;
        return *this;
    }
    
    EnhancedParamBuilder& example(const std::string& ex) {
        example_ = ex;
        return *this;
    }
    
    // 获取参数定义
    restful::ParamDefinition get() const { return param_; }
    
    // 获取数据类型
    ParamDataType getDataType() const { return data_type_; }
    
    // 获取描述
    const std::string& getDescription() const { return description_; }
    
    // 获取示例
    const std::string& getExample() const { return example_; }
};

// ========== 参数验证器 ==========

class ParamValidator {
public:
    // 验证参数值
    static std::string validate(const restful::ParamDefinition& param, const std::string& value) {
        // 检查必填参数
        if (param.validation.required && value.empty()) {
            return "Parameter '" + param.name + "' is required";
        }
        
        // 可选参数为空时跳过验证
        if (!param.validation.required && value.empty()) {
            return "";
        }
        
        // 字符串长度验证
        if (param.validation.has_min_length && value.length() < param.validation.min_length) {
            return "Parameter '" + param.name + "' must be at least " + 
                   std::to_string(param.validation.min_length) + " characters";
        }
        
        if (param.validation.has_max_length && value.length() > param.validation.max_length) {
            return "Parameter '" + param.name + "' must be at most " + 
                   std::to_string(param.validation.max_length) + " characters";
        }
        
        // 整数范围验证
        if (param.validation.has_min || param.validation.has_max) {
            try {
                long val = std::stol(value);
                if (param.validation.has_min && val < param.validation.min_value) {
                    return "Parameter '" + param.name + "' must be at least " + 
                           std::to_string(param.validation.min_value);
                }
                if (param.validation.has_max && val > param.validation.max_value) {
                    return "Parameter '" + param.name + "' must be at most " + 
                           std::to_string(param.validation.max_value);
                }
            } catch (const std::exception&) {
                return "Parameter '" + param.name + "' must be a valid integer";
            }
        }
        
        // 浮点数范围验证
        if (param.validation.has_min && param.validation.has_max) {
            try {
                double val = std::stod(value);
                if (val < param.validation.min_double || val > param.validation.max_double) {
                    return "Parameter '" + param.name + "' must be between " + 
                           std::to_string(param.validation.min_double) + " and " + 
                           std::to_string(param.validation.max_double);
                }
            } catch (const std::exception&) {
                return "Parameter '" + param.name + "' must be a valid number";
            }
        }
        
        // 正则表达式验证
        if (param.validation.has_pattern) {
            try {
                std::regex regex_pattern(param.validation.pattern);
                if (!std::regex_match(value, regex_pattern)) {
                    return "Parameter '" + param.name + "' does not match the required pattern";
                }
            } catch (const std::regex_error& e) {
                return "Invalid regex pattern for parameter '" + param.name + "'";
            }
        }
        
        // 枚举值验证
        if (param.validation.has_enum) {
            bool found = std::find(param.validation.enum_values.begin(), 
                                  param.validation.enum_values.end(), 
                                  value) != param.validation.enum_values.end();
            if (!found) {
                std::string enum_str = "[";
                for (size_t i = 0; i < param.validation.enum_values.size(); ++i) {
                    if (i > 0) enum_str += ", ";
                    enum_str += param.validation.enum_values[i];
                }
                enum_str += "]";
                return "Parameter '" + param.name + "' must be one of " + enum_str;
            }
        }
        
        return "";
    }
    
    // 批量验证参数
    static std::string validateAll(const std::vector<restful::ParamDefinition>& params, 
                                   const std::map<std::string, std::string>& values) {
        for (const auto& param : params) {
            auto it = values.find(param.name);
            std::string value = (it != values.end()) ? it->second : "";
            
            std::string error = validate(param, value);
            if (!error.empty()) {
                return error;
            }
        }
        return "";
    }
};

// ========== 参数访问辅助类 ==========

class ParamAccessor {
private:
    const HttpRequest& req_;
    
public:
    ParamAccessor(const HttpRequest& req) : req_(req) {}
    
    // 获取路径参数（带验证）
    template<typename T>
    T getPath(const std::string& name, const T& default_value = T()) const {
        return req_.path<T>(name, default_value);
    }
    
    // 获取查询参数（带验证）
    template<typename T>
    T getQuery(const std::string& name, const T& default_value = T()) const {
        return req_.query<T>(name, default_value);
    }
    
    // 获取查询参数（带默认值和类型转换）
    std::string getQueryString(const std::string& name, const std::string& default_value = "") const {
        auto it = req_.query_params.find(name);
        return (it != req_.query_params.end()) ? it->second : default_value;
    }
    
    // 获取整数查询参数
    int getQueryInt(const std::string& name, int default_value = 0) const {
        auto it = req_.query_params.find(name);
        if (it == req_.query_params.end()) return default_value;
        
        try {
            return std::stoi(it->second);
        } catch (...) {
            return default_value;
        }
    }
    
    // 获取64位整数查询参数
    int64_t getQueryInt64(const std::string& name, int64_t default_value = 0) const {
        auto it = req_.query_params.find(name);
        if (it == req_.query_params.end()) return default_value;
        
        try {
            return std::stoll(it->second);
        } catch (...) {
            return default_value;
        }
    }
    
    // 获取浮点查询参数
    double getQueryDouble(const std::string& name, double default_value = 0.0) const {
        auto it = req_.query_params.find(name);
        if (it == req_.query_params.end()) return default_value;
        
        try {
            return std::stod(it->second);
        } catch (...) {
            return default_value;
        }
    }
    
    // 获取布尔查询参数
    bool getQueryBool(const std::string& name, bool default_value = false) const {
        auto it = req_.query_params.find(name);
        if (it == req_.query_params.end()) return default_value;
        
        const std::string& value = it->second;
        return (value == "true" || value == "1" || value == "yes" || value == "on");
    }
    
    // 检查参数是否存在
    bool hasPath(const std::string& name) const {
        return req_.path_params.find(name) != req_.path_params.end();
    }
    
    bool hasQuery(const std::string& name) const {
        return req_.query_params.find(name) != req_.query_params.end();
    }
};

// ========== 便捷函数 ==========

// 创建参数访问器
inline ParamAccessor params(const HttpRequest& req) {
    return ParamAccessor(req);
}

// 验证路径参数
inline std::string validatePathParams(const HttpRequest& req, 
                                       const std::vector<restful::ParamDefinition>& params) {
    return ParamValidator::validateAll(params, req.path_params);
}

// 验证查询参数
inline std::string validateQueryParams(const HttpRequest& req, 
                                        const std::vector<restful::ParamDefinition>& params) {
    return ParamValidator::validateAll(params, req.query_params);
}

// ========== 常用参数定义 ==========

namespace CommonParamDefs {
    // ID 参数（路径参数）
    inline EnhancedParamBuilder idParam(const std::string& name = "id",
                                         int min_val = 1, int max_val = 1000000) {
        return EnhancedParamBuilder(name, restful::ParamType::PATH, ParamDataType::INT64)
            .required()
            .range(min_val, max_val)
            .description("Resource ID")
            .example("123");
    }
    
    // 分页参数（查询参数）
    inline std::vector<EnhancedParamBuilder> pagination() {
        return {
            EnhancedParamBuilder("page", restful::ParamType::QUERY, ParamDataType::INT32)
                .defaultValue(1)
                .min(1)
                .optional()
                .description("Page number (1-based)")
                .example("1"),

            EnhancedParamBuilder("limit", restful::ParamType::QUERY, ParamDataType::INT32)
                .defaultValue(10)
                .range(1, 100)
                .optional()
                .description("Items per page")
                .example("10"),
            
            EnhancedParamBuilder("sort", restful::ParamType::QUERY, ParamDataType::STRING)
                .defaultValue("id")
                .optional()
                .description("Sort field")
                .example("created_at"),
            
            EnhancedParamBuilder("order", restful::ParamType::QUERY, ParamDataType::STRING)
                .defaultValue("asc")
                .oneOf("asc", "desc")
                .optional()
                .description("Sort order")
                .example("asc")
        };
    }
    
    // 搜索参数（查询参数）
    inline std::vector<EnhancedParamBuilder> search() {
        return {
            EnhancedParamBuilder("q", restful::ParamType::QUERY, ParamDataType::STRING)
                .optional()
                .minLength(1)
                .maxLength(100)
                .description("Search query")
                .example("keyword"),
            
            EnhancedParamBuilder("fields", restful::ParamType::QUERY, ParamDataType::STRING)
                .optional()
                .description("Comma-separated list of fields to return")
                .example("id,name,email")
        };
    }
    
    // 时间范围参数（查询参数）
    inline std::vector<EnhancedParamBuilder> dateRange() {
        return {
            EnhancedParamBuilder("start_date", restful::ParamType::QUERY, ParamDataType::DATE)
                .optional()
                .pattern("^\\d{4}-\\d{2}-\\d{2}$")
                .description("Start date (YYYY-MM-DD)")
                .example("2024-01-01"),
            
            EnhancedParamBuilder("end_date", restful::ParamType::QUERY, ParamDataType::DATE)
                .optional()
                .pattern("^\\d{4}-\\d{2}-\\d{2}$")
                .description("End date (YYYY-MM-DD)")
                .example("2024-12-31")
        };
    }
}

} // namespace uvapi

#endif // PARAMS_DSL_H