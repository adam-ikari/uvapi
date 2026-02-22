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
    
    // 示例 3: Handler 中使用 optional<T>（推荐方式）
    std::cout << "\n=== 示例 3: Handler 中使用 optional<T> ===" << std::endl;
    
    // DSL 中声明参数：
    // 1. 纯可选参数：param.optional()  // 不提供默认值
    // 2. 带默认值：param.optional().defaultValue(1)  // 可选 + 默认值
    // 3. 必需参数：param.required()  // 必须提供，不能设置默认值
    
    // Handler 中直接使用 optional<T>，无需处理默认值
    auto page = test_req.query<int>("page");
    auto limit = test_req.query<int>("limit");
    auto sort = test_req.query<std::string>("sort");
    
    // 框架已经自动应用了 DSL 中声明的默认值
    std::cout << "page: " << page.value() << " (auto-applied default if not provided)" << std::endl;
    std::cout << "limit: " << limit.value() << " (auto-applied default if not provided)" << std::endl;
    std::cout << "sort: " << sort.value() << " (auto-applied default if not provided)" << std::endl;
    
    std::cout << "\n推荐做法：" << std::endl;
    std::cout << "1. 在 DSL 中声明参数和默认值" << std::endl;
    std::cout << "2. Handler 中使用 req.query<int>(\"key\") 获取 optional<T>" << std::endl;
    std::cout << "3. 直接使用 .value()，框架已自动应用默认值" << std::endl;
    std::cout << "4. Handler 中无需手动处理默认值" << std::endl;
    
    std::cout << "\n参数声明类型：" << std::endl;
    std::cout << "- 纯可选：param.optional()  // 无默认值，返回 empty optional" << std::endl;
    std::cout << "- 带默认值：param.optional().defaultValue(1)  // 可选参数 + 默认值" << std::endl;
    std::cout << "- 必需参数：param.required()  // 必须提供，不能设置默认值" << std::endl;
    
    // 示例 4: 纯可选参数（无默认值）的使用
    std::cout << "\n=== 示例 4: 纯可选参数 ===" << std::endl;
    
    auto keyword = test_req.query<std::string>("keyword");
    if (keyword.hasValue()) {
        std::cout << "keyword: " << keyword.value() << std::endl;
    } else {
        std::cout << "keyword: not provided (no default value)" << std::endl;
    }
    
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
    
    auto category = test_req.path<std::string>("category");
    if (category.hasValue()) {
        std::cout << "category: " << category.value() << std::endl;
    } else {
        std::cout << "category: not provided (DSL default would be applied)" << std::endl;
    }
    
    auto missing_path = test_req.path<std::string>("missing");
    if (missing_path.hasValue()) {
        std::cout << "missing_path: " << missing_path.value() << std::endl;
    } else {
        std::cout << "missing_path: not provided (DSL default would be applied)" << std::endl;
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