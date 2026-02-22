/**
 * @file declarative_dsl.h
 * @brief 声明式 DSL
 * 
 * 整体式 API 声明
 */

#ifndef DECLARATIVE_DSL_H
#define DECLARATIVE_DSL_H

#include <string>
#include <vector>
#include <initializer_list>
#include <functional>
#include "framework.h"

namespace uvapi {
namespace declarative {

// ========== API 定义 ==========

struct ApiDefinition {
    std::string path;
    HttpMethod method;
    std::string description;
    std::vector<restful::ParamDefinition> params;
    std::function<HttpResponse(const HttpRequest&)> handler;
    
    ApiDefinition(const std::string& p, HttpMethod m) 
        : path(p), method(m) {}
    
    // 添加参数（属性风格）
    ApiDefinition& param(const std::string& name, std::string type, bool required, std::string default_value = "") {
        restful::ParamDefinition def(name, restful::ParamType::QUERY);
        def.validation.required = required;
        def.default_value = default_value;
        
        if (type == "int") def.data_type = 1;
        else if (type == "string") def.data_type = 0;
        else if (type == "bool") def.data_type = 5;
        else if (type == "double") def.data_type = 3;
        
        params.push_back(def);
        return *this;
    }
    
    // 添加路径参数
    ApiDefinition& pathParam(const std::string& name, std::string type, bool required = true) {
        restful::ParamDefinition def(name, restful::ParamType::PATH);
        def.validation.required = required;
        
        if (type == "int") def.data_type = 1;
        else if (type == "string") def.data_type = 0;
        else if (type == "bool") def.data_type = 5;
        else if (type == "double") def.data_type = 3;
        
        params.push_back(def);
        return *this;
    }
    
    // 验证规则（添加到最后一个参数）
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

// ========== API 构建器 ==========

class ApiBuilder {
private:
    std::vector<ApiDefinition> apis_;
    
public:
    ApiBuilder() {}
    
    // 定义 GET API
    ApiDefinition& get(const std::string& path) {
        apis_.emplace_back(path, HttpMethod::GET);
        return apis_.back();
    }
    
    // 定义 POST API
    ApiDefinition& post(const std::string& path) {
        apis_.emplace_back(path, HttpMethod::POST);
        return apis_.back();
    }
    
    // 定义 PUT API
    ApiDefinition& put(const std::string& path) {
        apis_.emplace_back(path, HttpMethod::PUT);
        return apis_.back();
    }
    
    // 定义 DELETE API
    ApiDefinition& del(const std::string& path) {
        apis_.emplace_back(path, HttpMethod::DELETE);
        return apis_.back();
    }
    
    // 定义 PATCH API
    ApiDefinition& patch(const std::string& path) {
        apis_.emplace_back(path, HttpMethod::PATCH);
        return apis_.back();
    }
    
    // 应用到 Api 实例
    void applyTo(Api& api) {
        for (const auto& api_def : apis_) {
            // 注册路由和参数
            api.get(api_def.path, api_def.handler);
        }
    }
};

} // namespace declarative
} // namespace uvapi

#endif // DECLARATIVE_DSL_H