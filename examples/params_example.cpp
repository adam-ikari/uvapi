/**
 * @file params_example.cpp
 * @brief 参数声明 DSL 使用示例
 */

#include <iostream>
#include "../include/framework.h"

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
    
    // 示例 2: 参数验证
    std::cout << "\n=== 示例 2: 参数验证 ===" << std::endl;
    
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