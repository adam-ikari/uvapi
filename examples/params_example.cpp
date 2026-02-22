/**
 * @file params_example.cpp
 * @brief 参数声明 DSL 使用示例
 * 
 * 展示如何使用可选参数 API（optional<T>）和自动类型推导
 */

#include <iostream>
#include "../include/framework.h"
#include "../include/params_dsl.h"

using namespace uvapi;
using namespace restful;

int main() {
    std::cout << "=== 参数声明 DSL 示例 ===" << std::endl;
    
    // 示例 1: 使用 ParamAccessor 访问参数
    std::cout << "\n=== 示例 1: ParamAccessor 使用 ===" << std::endl;
    
    HttpRequest test_req;
    test_req.query_params["page"] = "2";
    test_req.query_params["limit"] = "20";
    test_req.query_params["search"] = "keyword";
    test_req.query_params["active"] = "true";
    
    ParamAccessor p(test_req);
    
    std::cout << "page (int): " << p.getQueryInt("page", 1) << std::endl;
    std::cout << "limit (int): " << p.getQueryInt("limit", 10) << std::endl;
    std::cout << "search (string): " << p.getQueryString("search", "") << std::endl;
    std::cout << "active (bool): " << (p.getQueryBool("active", false) ? "true" : "false") << std::endl;
    std::cout << "missing (int): " << p.getQueryInt("missing", 99) << std::endl;
    
    // 示例 2: 可选参数 API（返回 optional<T>）
    std::cout << "\n=== 示例 2: 可选参数 API ===" << std::endl;
    
    std::cout << "page (optional<int>): ";
    auto page_opt = test_req.queryOpt<int>("page");
    if (page_opt.hasValue()) {
        std::cout << page_opt.value() << std::endl;
    } else {
        std::cout << "not provided" << std::endl;
    }
    
    std::cout << "missing (optional<int>): ";
    auto missing_opt = test_req.queryOpt<int>("missing");
    if (missing_opt.hasValue()) {
        std::cout << missing_opt.value() << std::endl;
    } else {
        std::cout << "not provided" << std::endl;
    }
    
    // 使用 value_or() 方法提供默认值
    int limit = test_req.queryOpt<int>("limit").value_or(10);
    std::cout << "limit (with default): " << limit << std::endl;
    
    std::string search = test_req.queryOpt<std::string>("search").value_or("");
    std::cout << "search (with default): \"" << search << "\"" << std::endl;
    
    // 示例 3: 自动类型推导 + 可选参数
    std::cout << "\n=== 示例 3: 自动类型推导 ===" << std::endl;
    
    // 必需参数（有默认值）
    int page_default = test_req.query<int>("page", 1);
    std::cout << "page (default=1): " << page_default << std::endl;
    
    // 可选参数（无默认值）
    auto sort_by = test_req.queryOpt<std::string>("sort_by");
    if (sort_by.hasValue()) {
        std::cout << "sort_by: " << sort_by.value() << std::endl;
    } else {
        std::cout << "sort_by: not provided (will use server default)" << std::endl;
    }
    
    // 使用 value_or() 简化代码
    std::string order = test_req.queryOpt<std::string>("order").value_or("asc");
    std::cout << "order: " << order << std::endl;
    
    // 示例 4: 路径参数的可选参数 API
    std::cout << "\n=== 示例 4: 路径参数可选 API ===" << std::endl;
    
    test_req.path_params["id"] = "123";
    test_req.path_params["category"] = "electronics";
    
    auto id = test_req.pathOpt<int>("id");
    if (id.hasValue()) {
        std::cout << "id: " << id.value() << std::endl;
    } else {
        std::cout << "id: not provided" << std::endl;
    }
    
    auto category = test_req.pathOpt<std::string>("category");
    std::cout << "category: " << category.value_or("default") << std::endl;
    
    auto missing_path = test_req.pathOpt<std::string>("missing");
    if (missing_path.hasValue()) {
        std::cout << "missing_path: " << missing_path.value() << std::endl;
    } else {
        std::cout << "missing_path: not provided" << std::endl;
    }
    
    // 示例 5: 参数验证
    std::cout << "\n=== 示例 5: 参数验证 ===" << std::endl;
    
    ParamDefinition ageParam("age", ParamType::QUERY);
    ageParam.validation.required = true;
    ageParam.validation.min_value = 18;
    ageParam.validation.max_value = 120;
    ageParam.validation.has_min = true;
    ageParam.validation.has_max = true;
    
    std::string result1 = ParamValidator::validate(ageParam, "25");
    std::cout << "验证年龄 '25': " << (result1.empty() ? "通过" : result1) << std::endl;
    
    std::string result2 = ParamValidator::validate(ageParam, "15");
    std::cout << "验证年龄 '15': " << (result2.empty() ? "通过" : result2) << std::endl;
    
    std::string result3 = ParamValidator::validate(ageParam, "");
    std::cout << "验证年龄 '': " << (result3.empty() ? "通过" : result3) << std::endl;
    
    std::cout << "\n参数 DSL 示例完成！" << std::endl;
    
    return 0;
}