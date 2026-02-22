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
    auto page_opt = test_req.query<int>("page");
    if (page_opt.hasValue()) {
        std::cout << page_opt.value() << std::endl;
    } else {
        std::cout << "not provided" << std::endl;
    }
    
    std::cout << "missing (optional<int>): ";
    auto missing_opt = test_req.query<int>("missing");
    if (missing_opt.hasValue()) {
        std::cout << missing_opt.value() << std::endl;
    } else {
        std::cout << "not provided" << std::endl;
    }
    
    // 示例 3: 自动类型推导（更简洁）
    std::cout << "\n=== 示例 3: 自动类型推导 ===" << std::endl;
    
    // 通过默认值自动推导类型
    int page_auto = test_req.query("page", 1);  // 自动推导为 int
    int limit_auto = test_req.query("limit", 10);  // 自动推导为 int
    std::string sort_auto = test_req.query("sort", std::string("id"));  // 自动推导为 std::string
    bool active_auto = test_req.query("active", false);  // 自动推导为 bool
    
    std::cout << "page (auto-deduced): " << page_auto << std::endl;
    std::cout << "limit (auto-deduced): " << limit_auto << std::endl;
    std::cout << "sort (auto-deduced): " << sort_auto << std::endl;
    std::cout << "active (auto-deduced): " << (active_auto ? "true" : "false") << std::endl;
    
    // 对比：旧方式 vs 新方式
    std::cout << "\n对比：" << std::endl;
    std::cout << "可选参数: req.query<int>(\"page\")  // 返回 optional<int>" << std::endl;
    std::cout << "带默认值: req.query(\"page\", 1)  // 返回 int，类型自动推导" << std::endl;
    
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
    
    // 示例 4: DSL 中声明默认值，handler 中无需再次处理
    std::cout << "\n=== 示例 4: DSL 默认值自动应用 ===" << std::endl;
    
    // 模拟 DSL 中声明的参数（包含默认值）
    ParamDefinition pageParam("page", ParamType::QUERY);
    pageParam.validation.required = false;
    pageParam.default_value = "1";  // DSL 中声明默认值
    
    ParamDefinition limitParam("limit", ParamType::QUERY);
    limitParam.validation.required = false;
    limitParam.default_value = "10";  // DSL 中声明默认值
    
    // 模拟框架自动应用默认值后的请求
    HttpRequest req_with_defaults = test_req;
    
    // 框架会自动为不存在的可选参数应用默认值
    if (req_with_defaults.query_params.find("page") == req_with_defaults.query_params.end() && !pageParam.default_value.empty()) {
        req_with_defaults.query_params["page"] = pageParam.default_value;
    }
    if (req_with_defaults.query_params.find("limit") == req_with_defaults.query_params.end() && !limitParam.default_value.empty()) {
        req_with_defaults.query_params["limit"] = limitParam.default_value;
    }
    
    // Handler 中直接访问参数，无需处理默认值
    int page_auto = req_with_defaults.query<int>("page");
    int limit_auto = req_with_defaults.query<int>("limit");
    
    std::cout << "page (auto-applied default): " << page_auto << std::endl;
    std::cout << "limit (auto-applied default): " << limit_auto << std::endl;
    
    // 示例 5: 必需参数和可选参数的区别
    std::cout << "\n=== 示例 5: 必需参数 vs 可选参数 ===" << std::endl;
    
    // 必需参数：如果未提供，框架会返回 400 错误
    ParamDefinition userIdParam("user_id", ParamType::PATH);
    userIdParam.validation.required = true;
    
    // 可选参数：如果未提供，框架会自动应用默认值
    ParamDefinition statusParam("status", ParamType::QUERY);
    statusParam.validation.required = false;
    statusParam.default_value = "active";
    
    std::cout << "user_id (required): must be provided, otherwise 400 error" << std::endl;
    std::cout << "status (optional with default): auto-applied to 'active' if not provided" << std::endl;
    
    // 示例 4: 路径参数的可选参数 API
    std::cout << "\n=== 示例 4: 路径参数可选 API ===" << std::endl;
    
    test_req.path_params["id"] = "123";
    test_req.path_params["category"] = "electronics";
    
    auto id = test_req.path<int>("id");
    if (id.hasValue()) {
        std::cout << "id: " << id.value() << std::endl;
    } else {
        std::cout << "id: not provided" << std::endl;
    }
    
    std::string category = test_req.path("category", "default");
    std::cout << "category: " << category << std::endl;
    
    auto missing_path = test_req.path<std::string>("missing");
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