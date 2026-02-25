/**
 * @file declarative_dsl.h
 * @brief 声明式 DSL
 * 
 * 整体式 API 声明，清晰的 Required 和 OptionalWithDefault 语法
 */

#ifndef DECLARATIVE_DSL_H
#define DECLARATIVE_DSL_H

#include <string>
#include <vector>
#include <functional>
#include "framework.h"

namespace uvapi {

// 前向声明
class Api;

namespace declarative {

// ========== 参数需求（模板化）==========

template<typename T>
struct Required {
    Required() {}
};

template<typename T>
struct OptionalWithDefault {
    T default_value;
    explicit OptionalWithDefault(T def) : default_value(def) {}
};

// ========== Schema 验证（用于 Request Body）==========

template<typename T>
class Schema {
private:
    std::vector<restful::ParamDefinition> fields_;
    
public:
    Schema() {}
    
    // 添加必需字段
    template<typename FieldT>
    Schema& field(const std::string& name, const Required<FieldT>& req) {
        restful::ParamDefinition def(name, restful::ParamType::BODY);
        def.validation.required = true;
        
        if (std::is_same<FieldT, int>::value) def.data_type = 1;
        else if (std::is_same<FieldT, int64_t>::value) def.data_type = 2;
        else if (std::is_same<FieldT, double>::value) def.data_type = 3;
        else if (std::is_same<FieldT, float>::value) def.data_type = 4;
        else if (std::is_same<FieldT, bool>::value) def.data_type = 5;
        else if (std::is_same<FieldT, std::string>::value) def.data_type = 0;
        
        fields_.push_back(def);
        return *this;
    }
    
    // 添加可选字段（带默认值）
    template<typename FieldT>
    Schema& field(const std::string& name, const OptionalWithDefault<FieldT>& opt) {
        restful::ParamDefinition def(name, restful::ParamType::BODY);
        def.validation.required = false;
        
        if (std::is_same<FieldT, bool>::value) {
            def.default_value = opt.default_value ? "true" : "false";
        } else if (std::is_same<FieldT, std::string>::value) {
            def.default_value = opt.default_value;
        } else {
            def.default_value = std::to_string(opt.default_value);
        }
        
        if (std::is_same<FieldT, int>::value) def.data_type = 1;
        else if (std::is_same<FieldT, int64_t>::value) def.data_type = 2;
        else if (std::is_same<FieldT, double>::value) def.data_type = 3;
        else if (std::is_same<FieldT, float>::value) def.data_type = 4;
        else if (std::is_same<FieldT, bool>::value) def.data_type = 5;
        else if (std::is_same<FieldT, std::string>::value) def.data_type = 0;
        
        fields_.push_back(def);
        return *this;
    }
    
    // 验证规则
    Schema& range(int min_val, int max_val) {
        if (!fields_.empty()) {
            fields_.back().validation.min_value = min_val;
            fields_.back().validation.max_value = max_val;
            fields_.back().validation.has_min = true;
            fields_.back().validation.has_max = true;
        }
        return *this;
    }
    
    Schema& length(size_t min_len, size_t max_len) {
        if (!fields_.empty()) {
            fields_.back().validation.min_length = min_len;
            fields_.back().validation.max_length = max_len;
            fields_.back().validation.has_min_length = true;
            fields_.back().validation.has_max_length = true;
        }
        return *this;
    }
    
    Schema& pattern(const std::string& regex) {
        if (!fields_.empty()) {
            fields_.back().validation.pattern = regex;
            fields_.back().validation.has_pattern = true;
        }
        return *this;
    }
    
    Schema& oneOf(std::initializer_list<std::string> values) {
        if (!fields_.empty()) {
            fields_.back().validation.enum_values = values;
            fields_.back().validation.has_enum = true;
        }
        return *this;
    }
    
    const std::vector<restful::ParamDefinition>& getFields() const {
        return fields_;
    }
};

// ========== 常用参数预设 ==========

struct PageParam {
    int default_page;
    int default_limit;
    
    PageParam() : default_page(1), default_limit(10) {}
    PageParam(int page, int limit) 
        : default_page(page), default_limit(limit) {}
    
    static PageParam page(int value) {
        PageParam param;
        param.default_page = value;
        return param;
    }
    
    static PageParam limit(int value) {
        PageParam param;
        param.default_limit = value;
        return param;
    }
};

struct SearchParam {
    std::string default_value;
    
    SearchParam() : default_value("") {}
    explicit SearchParam(const std::string& def) 
        : default_value(def) {}
};

struct SortParam {
    std::string default_field;
    std::string default_order;
    std::vector<std::string> valid_fields;
    std::vector<std::string> valid_orders;
    
    SortParam() 
        : default_field("id"), default_order("asc")
        , valid_fields({"id", "created_at"}), valid_orders({"asc", "desc"}) {}
    
    SortParam(const std::string& field, const std::string& order,
              const std::vector<std::string>& fields = {"id", "created_at"},
              const std::vector<std::string>& orders = {"asc", "desc"})
        : default_field(field), default_order(order)
        , valid_fields(fields), valid_orders(orders) {}
    
    static SortParam field(const std::string& field) {
        SortParam param;
        param.default_field = field;
        return param;
    }
    
    static SortParam order(const std::string& order) {
        SortParam param;
        param.default_order = order;
        return param;
    }
};

struct RangeParam {
    int default_min;
    int default_max;
    
    RangeParam() : default_min(0), default_max(1000000) {}
    RangeParam(int min_val, int max_val) 
        : default_min(min_val), default_max(max_val) {}
    
    static RangeParam min(int value) {
        RangeParam param;
        param.default_min = value;
        return param;
    }
    
    static RangeParam max(int value) {
        RangeParam param;
        param.default_max = value;
        return param;
    }
};

// ========== 参数验证器 ==========

struct ValidationResult {
    bool success;
    std::string error_message;
    std::string field_name;

    ValidationResult() : success(true), error_message(""), field_name("") {}
    ValidationResult(bool s, const std::string& msg, const std::string& field)
        : success(s), error_message(msg), field_name(field) {}

    static ValidationResult ok() {
        return ValidationResult(true, "", "");
    }

    static ValidationResult error(const std::string& field, const std::string& msg) {
        return ValidationResult(false, msg, field);
    }
};

class ParameterValidator {
public:
    // 验证单个参数
    static ValidationResult validate(const std::string& name, const std::string& value,
                                     const restful::ParamDefinition& def) {
        // 检查必需参数
        if (def.validation.required && value.empty()) {
            return ValidationResult::error(name, "Required parameter is missing");
        }

        // 如果值为空且非必需，跳过验证
        if (value.empty() && !def.validation.required) {
            return ValidationResult::ok();
        }

        // 类型验证
        if (!validateType(value, def.data_type)) {
            return ValidationResult::error(name, "Invalid parameter type");
        }

        // 范围验证
        if (def.validation.has_min || def.validation.has_max) {
            ValidationResult range_result = validateRange(name, value, def);
            if (!range_result.success) {
                return range_result;
            }
        }

        // 长度验证
        if (def.validation.has_min_length || def.validation.has_max_length) {
            ValidationResult length_result = validateLength(name, value, def);
            if (!length_result.success) {
                return length_result;
            }
        }

        // 正则表达式验证
        if (def.validation.has_pattern) {
            ValidationResult pattern_result = validatePattern(name, value, def);
            if (!pattern_result.success) {
                return pattern_result;
            }
        }

        // 枚举值验证
        if (def.validation.has_enum) {
            ValidationResult enum_result = validateEnum(name, value, def);
            if (!enum_result.success) {
                return enum_result;
            }
        }

        return ValidationResult::ok();
    }

private:
    // 验证类型
    static bool validateType(const std::string& value, int data_type) {
        if (value.empty()) return true;

        switch (data_type) {
            case 0:  // STRING
                return true;
            case 1:  // INT
            case 2:  // INT64
            case 3:  // DOUBLE
            case 4:  // FLOAT
                return isNumeric(value);
            case 5:  // BOOL
                return isBoolean(value);
            default:
                return true;
        }
    }

    // 验证范围
    static ValidationResult validateRange(const std::string& name, const std::string& value,
                                           const restful::ParamDefinition& def) {
        if (value.empty()) return ValidationResult::ok();

        if (def.data_type == 1 || def.data_type == 2) {
            // 整数范围验证
            int64_t int_val;
            if (!parseInt64(value, int_val)) {
                return ValidationResult::error(name, "Invalid integer value");
            }

            if (def.validation.has_min && int_val < def.validation.min_value) {
                return ValidationResult::error(name, "Value is too small");
            }

            if (def.validation.has_max && int_val > def.validation.max_value) {
                return ValidationResult::error(name, "Value is too large");
            }
        } else if (def.data_type == 3 || def.data_type == 4) {
            // 浮点数范围验证
            double double_val;
            if (!parseDouble(value, double_val)) {
                return ValidationResult::error(name, "Invalid numeric value");
            }

            if (def.validation.has_min && double_val < static_cast<double>(def.validation.min_value)) {
                return ValidationResult::error(name, "Value is too small");
            }

            if (def.validation.has_max && double_val > static_cast<double>(def.validation.max_value)) {
                return ValidationResult::error(name, "Value is too large");
            }
        }

        return ValidationResult::ok();
    }

    // 验证长度
    static ValidationResult validateLength(const std::string& name, const std::string& value,
                                           const restful::ParamDefinition& def) {
        size_t len = value.length();

        if (def.validation.has_min_length && len < static_cast<size_t>(def.validation.min_length)) {
            return ValidationResult::error(name, "Value is too short");
        }

        if (def.validation.has_max_length && len > static_cast<size_t>(def.validation.max_length)) {
            return ValidationResult::error(name, "Value is too long");
        }

        return ValidationResult::ok();
    }

    // 验证正则表达式
    static ValidationResult validatePattern(const std::string& name, const std::string& value,
                                            const restful::ParamDefinition& def) {
        try {
            std::regex pattern(def.validation.pattern);
            if (!std::regex_match(value, pattern)) {
                return ValidationResult::error(name, "Value does not match required pattern");
            }
        } catch (const std::regex_error&) {
            // 正则表达式错误，跳过验证
        }

        return ValidationResult::ok();
    }

    // 验证枚举值
    static ValidationResult validateEnum(const std::string& name, const std::string& value,
                                         const restful::ParamDefinition& def) {
        for (const std::string& valid_value : def.validation.enum_values) {
            if (value == valid_value) {
                return ValidationResult::ok();
            }
        }

        return ValidationResult::error(name, "Value is not in the allowed list");
    }

    // 辅助函数：检查是否为数字
    static bool isNumeric(const std::string& s) {
        if (s.empty()) return false;
        size_t start = 0;
        if (s[0] == '-') start = 1;
        for (size_t i = start; i < s.length(); ++i) {
            if (!std::isdigit(s[i]) && s[i] != '.') {
                return false;
            }
        }
        return true;
    }

    // 辅助函数：检查是否为布尔值
    static bool isBoolean(const std::string& s) {
        return s == "true" || s == "false" || s == "1" || s == "0";
    }

    // 辅助函数：解析整数
    static bool parseInt64(const std::string& s, int64_t& result) {
        char* end = nullptr;
        errno = 0;
        int64_t val = std::strtoll(s.c_str(), &end, 10);
        if (errno != 0 || *end != '\0') {
            return false;
        }
        result = val;
        return true;
    }

    // 辅助函数：解析浮点数
    static bool parseDouble(const std::string& s, double& result) {
        char* end = nullptr;
        errno = 0;
        double val = std::strtod(s.c_str(), &end);
        if (errno != 0 || *end != '\0') {
            return false;
        }
        result = val;
        return true;
    }
};

// ========== 参数解析器 ==========

class ParameterParser {
public:
    // 从 HttpRequest 提取并解析所有参数
    static std::map<std::string, std::string> extract(const HttpRequest& req,
                                                       const std::vector<restful::ParamDefinition>& params) {
        std::map<std::string, std::string> result;

        for (const restful::ParamDefinition& def : params) {
            std::string value;

            // 根据参数类型从不同位置提取
            if (def.type == restful::ParamType::QUERY) {
                value = extractQueryParam(req, def.name);
            } else if (def.type == restful::ParamType::PATH) {
                value = extractPathParam(req, def.name);
            }

            // 如果没有值且不是必需参数，使用默认值
            if (value.empty() && !def.validation.required) {
                value = def.default_value;
            }

            result[def.name] = value;
        }

        return result;
    }

    // 验证所有参数
    static ValidationResult validateAll(const std::map<std::string, std::string>& values,
                                        const std::vector<restful::ParamDefinition>& params) {
        for (const restful::ParamDefinition& def : params) {
            std::map<std::string, std::string>::const_iterator it = values.find(def.name);
            if (it == values.end()) {
                return ValidationResult::error(def.name, "Parameter not found");
            }

            ValidationResult result = ParameterValidator::validate(def.name, it->second, def);
            if (!result.success) {
                return result;
            }
        }

        return ValidationResult::ok();
    }

private:
    // 提取查询参数
    static std::string extractQueryParam(const HttpRequest& req, const std::string& name) {
        std::map<std::string, std::string>::const_iterator it = req.query_params.find(name);
        if (it != req.query_params.end()) {
            return it->second;
        }
        return "";
    }

    // 提取路径参数
    static std::string extractPathParam(const HttpRequest& req, const std::string& name) {
        std::map<std::string, std::string>::const_iterator it = req.path_params.find(name);
        if (it != req.path_params.end()) {
            return it->second;
        }
        return "";
    }
};

// ========== API 定义 ==========

struct ApiDefinition {
    std::string path;
    HttpMethod method;
    std::vector<restful::ParamDefinition> params;
    std::function<HttpResponse(const HttpRequest&)> handler;
    std::function<HttpResponse(const HttpRequest&, const std::map<std::string, std::string>&)> handler_with_params;
    std::vector<restful::ParamDefinition> body_fields;  // Body Schema 字段

    ApiDefinition(const std::string& p, HttpMethod m)
        : path(p), method(m) {}
    
    // 添加必需参数
    template<typename T>
    ApiDefinition& param(const std::string& name, const Required<T>& req) {
        (void)req;  // 验证规则暂未实现
        restful::ParamDefinition def(name, restful::ParamType::QUERY);
        def.validation.required = true;

        if (std::is_same<T, int>::value) def.data_type = 1;
        else if (std::is_same<T, int64_t>::value) def.data_type = 2;
        else if (std::is_same<T, double>::value) def.data_type = 3;
        else if (std::is_same<T, float>::value) def.data_type = 4;
        else if (std::is_same<T, bool>::value) def.data_type = 5;
        else if (std::is_same<T, std::string>::value) def.data_type = 0;

        params.push_back(def);
        return *this;
    }

    // 添加可选参数（带默认值）- 模板特化
    // bool 特化
    ApiDefinition& param(const std::string& name, const OptionalWithDefault<bool>& opt) {
        (void)opt;
        restful::ParamDefinition def(name, restful::ParamType::QUERY);
        def.validation.required = false;
        def.default_value = opt.default_value ? "true" : "false";
        def.data_type = 5;
        params.push_back(def);
        return *this;
    }

    // string 特化
    ApiDefinition& param(const std::string& name, const OptionalWithDefault<std::string>& opt) {
        (void)opt;
        restful::ParamDefinition def(name, restful::ParamType::QUERY);
        def.validation.required = false;
        def.default_value = opt.default_value;
        def.data_type = 0;
        params.push_back(def);
        return *this;
    }

    // int 特化
    ApiDefinition& param(const std::string& name, const OptionalWithDefault<int>& opt) {
        (void)opt;
        restful::ParamDefinition def(name, restful::ParamType::QUERY);
        def.validation.required = false;
        def.default_value = std::to_string(opt.default_value);
        def.data_type = 1;
        params.push_back(def);
        return *this;
    }

    // int64_t 特化
    ApiDefinition& param(const std::string& name, const OptionalWithDefault<int64_t>& opt) {
        (void)opt;
        restful::ParamDefinition def(name, restful::ParamType::QUERY);
        def.validation.required = false;
        def.default_value = std::to_string(opt.default_value);
        def.data_type = 2;
        params.push_back(def);
        return *this;
    }

    // double 特化
    ApiDefinition& param(const std::string& name, const OptionalWithDefault<double>& opt) {
        (void)opt;
        restful::ParamDefinition def(name, restful::ParamType::QUERY);
        def.validation.required = false;
        def.default_value = std::to_string(opt.default_value);
        def.data_type = 3;
        params.push_back(def);
        return *this;
    }

    // float 特化
    ApiDefinition& param(const std::string& name, const OptionalWithDefault<float>& opt) {
        (void)opt;
        restful::ParamDefinition def(name, restful::ParamType::QUERY);
        def.validation.required = false;
        def.default_value = std::to_string(opt.default_value);
        def.data_type = 4;
        params.push_back(def);
        return *this;
    }

    // 添加路径参数
    template<typename T>
    ApiDefinition& pathParam(const std::string& name, const Required<T>& req) {
        (void)req;  // 验证规则暂未实现
        restful::ParamDefinition def(name, restful::ParamType::PATH);
        def.validation.required = true;

        if (std::is_same<T, int>::value) def.data_type = 1;
        else if (std::is_same<T, int64_t>::value) def.data_type = 2;
        else if (std::is_same<T, double>::value) def.data_type = 3;
        else if (std::is_same<T, float>::value) def.data_type = 4;
        else if (std::is_same<T, bool>::value) def.data_type = 5;
        else if (std::is_same<T, std::string>::value) def.data_type = 0;

        params.push_back(def);
        return *this;
    }
    
    // 验证规则
    ApiDefinition& range(int min_val, int max_val) {
        if (!params.empty()) {
            params.back().validation.min_value = min_val;
            params.back().validation.max_value = max_val;
            params.back().validation.has_min = true;
            params.back().validation.has_max = true;
        }
        return *this;
    }
    
    ApiDefinition& length(size_t min_len, size_t max_len) {
        if (!params.empty()) {
            params.back().validation.min_length = min_len;
            params.back().validation.max_length = max_len;
            params.back().validation.has_min_length = true;
            params.back().validation.has_max_length = true;
        }
        return *this;
    }
    
    ApiDefinition& pattern(const std::string& regex) {
        if (!params.empty()) {
            params.back().validation.pattern = regex;
            params.back().validation.has_pattern = true;
        }
        return *this;
    }
    
    ApiDefinition& oneOf(std::initializer_list<std::string> values) {
        if (!params.empty()) {
            params.back().validation.enum_values = values;
            params.back().validation.has_enum = true;
        }
        return *this;
    }
    
    ApiDefinition& oneOf(const std::vector<std::string>& values) {
        if (!params.empty()) {
            params.back().validation.enum_values = values;
            params.back().validation.has_enum = true;
        }
        return *this;
    }
    
    // 设置处理器（旧版：手动解析参数）
    ApiDefinition& handle(std::function<HttpResponse(const HttpRequest&)> h) {
        handler = h;
        return *this;
    }

    // 设置处理器（新版：自动解析和验证参数）
    ApiDefinition& handleWithParams(std::function<HttpResponse(const HttpRequest&, const std::map<std::string, std::string>&)> h) {
        handler_with_params = h;
        return *this;
    }

    // 执行处理器（自动解析和验证）
    HttpResponse executeHandler(const HttpRequest& req) const {
        // 如果有自动解析的 handler，使用它
        if (handler_with_params) {
            // 提取参数
            std::map<std::string, std::string> extracted_params = ParameterParser::extract(req, params);

            // 验证参数
            ValidationResult validation_result = ParameterParser::validateAll(extracted_params, params);
            if (!validation_result.success) {
                // 验证失败，返回 400 错误
                std::string error_json = "{\"code\":400,\"message\":\"" + validation_result.error_message +
                                        "\",\"field\":\"" + validation_result.field_name + "\"}";
                return HttpResponse(400).json(error_json);
            }

            // 调用处理器
            return handler_with_params(req, extracted_params);
        }

        // 否则使用旧的 handler
        if (handler) {
            return handler(req);
        }

        // 没有 handler，返回 404
        return HttpResponse(404).json("{\"code\":404,\"message\":\"No handler found\"}");
    }
    
    // 设置 Request Body Schema
    template<typename T>
    ApiDefinition& body(const Schema<T>& schema) {
        body_fields = schema.getFields();
        return *this;
    }
    
    // ========== 便捷方法：常用参数预设 ==========
    
    // 添加分页参数
    ApiDefinition& pagination(const PageParam& page_param = PageParam()) {
        param("page", OptionalWithDefault<int>(page_param.default_page))
            .range(1, 1000000);
        param("limit", OptionalWithDefault<int>(page_param.default_limit))
            .range(1, 1000);
        return *this;
    }
    
    // 添加搜索参数
    ApiDefinition& search(const SearchParam& search_param = SearchParam()) {
        param("search", OptionalWithDefault<std::string>(search_param.default_value));
        return *this;
    }
    
    // 添加排序参数
    ApiDefinition& sort(const SortParam& sort_param = SortParam()) {
        param("sort", OptionalWithDefault<std::string>(sort_param.default_field))
            .oneOf(sort_param.valid_fields);
        param("order", OptionalWithDefault<std::string>(sort_param.default_order))
            .oneOf(sort_param.valid_orders);
        return *this;
    }
    
    // 添加范围参数（价格、年龄等）
    ApiDefinition& range(const std::string& min_name, const std::string& max_name, 
                        const RangeParam& range_param = RangeParam()) {
        param(min_name, OptionalWithDefault<int>(range_param.default_min))
            .range(0, 1000000);
        param(max_name, OptionalWithDefault<int>(range_param.default_max))
            .range(0, 1000000);
        return *this;
    }
    
    // 添加时间范围参数
    ApiDefinition& dateRange(const std::string& start_name = "start_date", 
                               const std::string& end_name = "end_date") {
        param(start_name, OptionalWithDefault<std::string>(""));
        param(end_name, OptionalWithDefault<std::string>(""));
        return *this;
    }
    
    // 添加状态筛选参数
    ApiDefinition& statusFilter(const std::vector<std::string>& valid_statuses,
                                  const std::string& default_status = "") {
        param("status", OptionalWithDefault<std::string>(default_status))
            .oneOf(valid_statuses);
        return *this;
    }
};

// ========== API 构建器 ==========

class ApiBuilder {
private:
    std::vector<ApiDefinition> apis_;
    
public:
    ApiBuilder() {}
    
    ApiDefinition& get(const std::string& path) {
        apis_.emplace_back(path, HttpMethod::GET);
        return apis_.back();
    }
    
    ApiDefinition& post(const std::string& path) {
        apis_.emplace_back(path, HttpMethod::POST);
        return apis_.back();
    }
    
    ApiDefinition& put(const std::string& path) {
        apis_.emplace_back(path, HttpMethod::PUT);
        return apis_.back();
    }
    
    ApiDefinition& del(const std::string& path) {
        apis_.emplace_back(path, HttpMethod::DELETE);
        return apis_.back();
    }
    
    ApiDefinition& patch(const std::string& path) {
        apis_.emplace_back(path, HttpMethod::PATCH);
        return apis_.back();
    }
    
    // 获取所有 API 定义
    const std::vector<ApiDefinition>& getApis() const {
        return apis_;
    }
};

} // namespace declarative
} // namespace uvapi

#endif // DECLARATIVE_DSL_H