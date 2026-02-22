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
    
    // 示例 3: Handler 中使用 ParamsAccessor（自动类型推导）
    std::cout << "\n=== 示例 3: Handler 中使用 ParamsAccessor ===" << std::endl;
    
    // DSL 中声明参数类型：
    // ParamGroup()
    //     .addQueryParam("user_id", [](ParamBuilder& p) { p.required().asInt(); })
    //     .addQueryParam("page", [](ParamBuilder& p) { p.optional().asInt().defaultValue(1); })
    //     .addQueryParam("keyword", [](ParamBuilder& p) { p.optional().asString(); })
    
    // Handler 中使用 ParamsAccessor，根据 DSL 声明的类型访问参数
    auto params = test_req.params();
    
    // 根据参数名访问，无需手动指定类型（类型由 DSL 决定）
    auto user_id = params.getInt("user_id");
    auto page = params.getInt("page");
    auto keyword = params.getString("keyword");
    
    if (user_id.hasValue()) {
        std::cout << "user_id: " << user_id.value() << std::endl;
    }
    
    if (page.hasValue()) {
        std::cout << "page: " << page.value() << std::endl;
    }
    
    if (keyword.hasValue()) {
        std::cout << "keyword: " << keyword.value() << std::endl;
    }
    
    std::cout << "\n推荐做法：" << std::endl;
    std::cout << "1. 在 DSL 中使用 asInt()/asString() 等方法声明参数类型" << std::endl;
    std::cout << "2. Handler 中使用 req.params() 获取 ParamsAccessor" << std::endl;
    std::cout << "3. 根据参数类型调用对应的 getXxx() 方法" << std::endl;
    std::cout << "4. 类型由 DSL 决定，Handler 中无需再次指定" << std::endl;
    
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