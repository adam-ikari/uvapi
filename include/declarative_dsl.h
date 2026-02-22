/**
 * @file declarative_dsl.h
 * @brief 声明式 DSL
 * 
 * 整体式 API 声明，使用明确的命名而非布尔值
 */

#ifndef DECLARATIVE_DSL_H
#define DECLARATIVE_DSL_H

#include <string>
#include <vector>
#include <functional>
#include "framework.h"

namespace uvapi {
namespace declarative {

// ========== 参数需求 ==========

struct Required {
    std::string type;
    Required(std::string t) : type(t) {}
};

struct Optional {
    std::string type;
    std::string default_value;
    Optional(std::string t, std::string def = "") : type(t), default_value(def) {}
};

// ========== API 定义 ==========

struct ApiDefinition {
    std::string path;
    HttpMethod method;
    std::vector<restful::ParamDefinition> params;
    std::function<HttpResponse(const HttpRequest&)> handler;
    
    ApiDefinition(const std::string& p, HttpMethod m) 
        : path(p), method(m) {}
    
    // 添加必需参数
    ApiDefinition& param(const std::string& name, const Required& req) {
        restful::ParamDefinition def(name, restful::ParamType::QUERY);
        def.validation.required = true;
        
        if (req.type == "int") def.data_type = 1;
        else if (req.type == "string") def.data_type = 0;
        else if (req.type == "bool") def.data_type = 5;
        else if (req.type == "double") def.data_type = 3;
        
        params.push_back(def);
        return *this;
    }
    
    // 添加可选参数
    ApiDefinition& param(const std::string& name, const Optional& opt) {
        restful::ParamDefinition def(name, restful::ParamType::QUERY);
        def.validation.required = false;
        def.default_value = opt.default_value;
        
        if (opt.type == "int") def.data_type = 1;
        else if (opt.type == "string") def.data_type = 0;
        else if (opt.type == "bool") def.data_type = 5;
        else if (opt.type == "double") def.data_type = 3;
        
        params.push_back(def);
        return *this;
    }
    
    // 添加路径参数
    ApiDefinition& pathParam(const std::string& name, const Required& req) {
        restful::ParamDefinition def(name, restful::ParamType::PATH);
        def.validation.required = true;
        
        if (req.type == "int") def.data_type = 1;
        else if (req.type == "string") def.data_type = 0;
        else if (req.type == "bool") def.data_type = 5;
        else if (req.type == "double") def.data_type = 3;
        
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
    
    // 设置处理器
    ApiDefinition& handle(std::function<HttpResponse(const HttpRequest&)> h) {
        handler = h;
        return *this;
    }
};

// ========== 类型定义函数 ==========

inline Required Int() { return Required("int"); }
inline Required String() { return Required("string"); }
inline Required Bool() { return Required("bool"); }
inline Required Double() { return Required("double"); }

inline Optional Int(int default_value) { return Optional("int", std::to_string(default_value)); }
inline Optional String(std::string default_value = "") { return Optional("string", default_value); }
inline Optional Bool(bool default_value) { return Optional("bool", default_value ? "true" : "false"); }
inline Optional Double(double default_value) { return Optional("double", std::to_string(default_value)); }

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
    
    void applyTo(Api& api) {
        for (const auto& api_def : apis_) {
            api.get(api_def.path, api_def.handler);
        }
    }
};

} // namespace declarative
} // namespace uvapi

#endif // DECLARATIVE_DSL_H