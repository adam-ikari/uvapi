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

// ========== API 定义 ==========

struct ApiDefinition {
    std::string path;
    HttpMethod method;
    std::vector<restful::ParamDefinition> params;
    std::function<HttpResponse(const HttpRequest&)> handler;
    
    ApiDefinition(const std::string& p, HttpMethod m) 
        : path(p), method(m) {}
    
    // 添加必需参数
    template<typename T>
    ApiDefinition& param(const std::string& name, const Required<T>& req) {
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
    
    // 添加可选参数（带默认值）
    template<typename T>
    ApiDefinition& param(const std::string& name, const OptionalWithDefault<T>& opt) {
        restful::ParamDefinition def(name, restful::ParamType::QUERY);
        def.validation.required = false;
        
        if (std::is_same<T, bool>::value) {
            def.default_value = opt.default_value ? "true" : "false";
        } else {
            def.default_value = std::to_string(opt.default_value);
        }
        
        if (std::is_same<T, int>::value) def.data_type = 1;
        else if (std::is_same<T, int64_t>::value) def.data_type = 2;
        else if (std::is_same<T, double>::value) def.data_type = 3;
        else if (std::is_same<T, float>::value) def.data_type = 4;
        else if (std::is_same<T, bool>::value) def.data_type = 5;
        else if (std::is_same<T, std::string>::value) def.data_type = 0;
        
        params.push_back(def);
        return *this;
    }
    
    // 添加路径参数
    template<typename T>
    ApiDefinition& pathParam(const std::string& name, const Required<T>& req) {
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
    
    // 设置处理器
    ApiDefinition& handle(std::function<HttpResponse(const HttpRequest&)> h) {
        handler = h;
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
    
    void applyTo(Api& api) {
        for (const auto& api_def : apis_) {
            api.get(api_def.path, api_def.handler);
        }
    }
};

} // namespace declarative
} // namespace uvapi

#endif // DECLARATIVE_DSL_H