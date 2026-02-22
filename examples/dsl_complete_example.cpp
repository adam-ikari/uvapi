/**
 * @file dsl_complete_example.cpp
 * @brief 完整的 DSL 使用示例
 * 
 * 展示如何使用模板 DSL 声明各种类型的参数
 */

#include <iostream>
#include "../include/framework.h"
#include "../include/params_dsl.h"

using namespace uvapi;
using namespace restful;

int main() {
    std::cout << "=== 完整的 DSL 使用示例 ===" << std::endl;
    
    // ========== 1. 查询参数声明 ==========
    std::cout << "\n=== 1. 查询参数声明 ===" << std::endl;
    
    // 整数参数（必需）
    auto userIdParam = queryParam<int>("user_id")
        .required()
        .range(1, 1000000);
    
    std::cout << "必需整数参数: queryParam<int>(\"user_id\").required().range(1, 1000000)" << std::endl;
    
    // 整数参数（可选，带默认值）
    auto pageParam = queryParam<int>("page")
        .optional()
        .defaultValue(1)
        .range(1, 1000);
    
    std::cout << "可选整数参数: queryParam<int>(\"page\").optional().defaultValue(1).range(1, 1000)" << std::endl;
    
    // 字符串参数（必需）
    auto usernameParam = queryParam<std::string>("username")
        .required()
        .length(3, 20);
    
    std::cout << "必需字符串参数: queryParam<std::string>(\"username\").required().length(3, 20)" << std::endl;
    
    // 字符串参数（可选，带默认值）
    auto searchParam = queryParam<std::string>("search")
        .optional()
        .defaultValue("")
        .pattern("^[a-zA-Z0-9 ]+$");
    
    std::cout << "可选字符串参数: queryParam<std::string>(\"search\").optional().defaultValue(\"\").pattern(\"^[a-zA-Z0-9 ]+$\")" << std::endl;
    
    // 布尔参数（可选，带默认值）
    auto activeParam = queryParam<bool>("active")
        .optional()
        .defaultValue(true);
    
    std::cout << "可选布尔参数: queryParam<bool>(\"active\").optional().defaultValue(true)" << std::endl;
    
    // 浮点数参数（可选，带默认值）
    auto priceParam = queryParam<double>("price")
        .optional()
        .defaultValue(0.0)
        .range(0.0, 1000000.0);
    
    std::cout << "可选浮点参数: queryParam<double>(\"price\").optional().defaultValue(0.0).range(0.0, 1000000.0)" << std::endl;
    
    // 枚举参数（可选）
    auto statusParam = queryParam<std::string>("status")
        .optional()
        .defaultValue("active")
        .oneOf({"active", "inactive", "pending", "deleted"});
    
    std::cout << "可选枚举参数: queryParam<std::string>(\"status\").optional().defaultValue(\"active\").oneOf({\"active\", \"inactive\", \"pending\", \"deleted\"})" << std::endl;
    
    // ========== 2. 路径参数声明 ==========
    std::cout << "\n=== 2. 路径参数声明 ===" << std::endl;
    
    // 路径整数参数（必需）
    auto idPathParam = pathParam<int>("id")
        .required()
        .range(1, INT_MAX);
    
    std::cout << "必需路径参数: pathParam<int>(\"id\").required().range(1, INT_MAX)" << std::endl;
    
    // 路径字符串参数（必需）
    auto categoryPathParam = pathParam<std::string>("category")
        .required()
        .pattern("^[a-z]+$");
    
    std::cout << "必需路径参数: pathParam<std::string>(\"category\").required().pattern(\"^[a-z]+$\")" << std::endl;
    
    // ========== 3. 完整的路由示例 ==========
    std::cout << "\n=== 3. 完整的路由示例 ===" << std::endl;
    
    std::cout << "\n示例 1: 用户列表 API" << std::endl;
    std::cout << "GET /api/users" << std::endl;
    std::cout << "参数:" << std::endl;
    std::cout << "  - page: int, optional, default=1, range=[1, 1000]" << std::endl;
    std::cout << "  - limit: int, optional, default=10, range=[1, 100]" << std::endl;
    std::cout << "  - status: string, optional, default='active', enum=[active, inactive, pending]" << std::endl;
    std::cout << "  - search: string, optional, default=''" << std::endl;
    
    std::cout << "\nDSL 声明:" << std::endl;
    std::cout << "route.get(\"/api/users\"" << std::endl;
    std::cout << "    .queryParam<int>(\"page\").optional().defaultValue(1).range(1, 1000)" << std::endl;
    std::cout << "    .queryParam<int>(\"limit\").optional().defaultValue(10).range(1, 100)" << std::endl;
    std::cout << "    .queryParam<std::string>(\"status\").optional().defaultValue(\"active\").oneOf({\"active\", \"inactive\", \"pending\"})" << std::endl;
    std::cout << "    .queryParam<std::string>(\"search\").optional().defaultValue(\"\")" << std::endl;
    std::cout << ")" << std::endl;
    
    std::cout << "\n示例 2: 用户详情 API" << std::endl;
    std::cout << "GET /api/users/{id}" << std::endl;
    std::cout << "参数:" << std::endl;
    std::cout << "  - id: int, required, range=[1, INT_MAX]" << std::endl;
    
    std::cout << "\nDSL 声明:" << std::endl;
    std::cout << "route.get(\"/api/users/{id}\"" << std::endl;
    std::cout << "    .pathParam<int>(\"id\").required().range(1, INT_MAX)" << std::endl;
    std::cout << ")" << std::endl;
    
    std::cout << "\n示例 3: 用户创建 API" << std::endl;
    std::cout << "POST /api/users" << std::endl;
    std::cout << "参数:" << std::endl;
    std::cout << "  - username: string, required, length=[3, 20]" << std::endl;
    std::cout << "  - email: string, required, pattern=email" << std::endl;
    std::cout << "  - age: int, optional, default=18, range=[18, 120]" << std::endl;
    std::cout << "  - active: bool, optional, default=true" << std::endl;
    
    std::cout << "\nDSL 声明:" << std::endl;
    std::cout << "route.post(\"/api/users\"" << std::endl;
    std::cout << "    .queryParam<std::string>(\"username\").required().length(3, 20)" << std::endl;
    std::cout << "    .queryParam<std::string>(\"email\").required().pattern(\"email\")" << std::endl;
    std::cout << "    .queryParam<int>(\"age\").optional().defaultValue(18).range(18, 120)" << std::endl;
    std::cout << "    .queryParam<bool>(\"active\").optional().defaultValue(true)" << std::endl;
    std::cout << ")" << std::endl;
    
    // ========== 4. Handler 使用示例 ==========
    std::cout << "\n=== 4. Handler 使用示例 ===" << std::endl;
    
    std::cout << "\n// DSL 声明参数" << std::endl;
    std::cout << "auto page = queryParam<int>(\"page\").optional().defaultValue(1);" << std::endl;
    std::cout << "auto limit = queryParam<int>(\"limit\").optional().defaultValue(10);" << std::endl;
    std::cout << "auto search = queryParam<std::string>(\"search\").optional().defaultValue(\"\");" << std::endl;
    std::cout << "auto active = queryParam<bool>(\"active\").optional().defaultValue(true);" << std::endl;
    
    std::cout << "\n// Handler 中使用" << std::endl;
    std::cout << ".handler([](const HttpRequest& req) -> HttpResponse {" << std::endl;
    std::cout << "    auto params = req.params();" << std::endl;
    std::cout << "    " << std::endl;
    std::cout << "    // 获取参数（类型由 DSL 决定）" << std::endl;
    std::cout << "    auto page = params.getInt(\"page\");" << std::endl;
    std::cout << "    auto limit = params.getInt(\"limit\");" << std::endl;
    std::cout << "    auto search = params.getString(\"search\");" << std::endl;
    std::cout << "    auto active = params.getBool(\"active\");" << std::endl;
    std::cout << "    " << std::endl;
    std::cout << "    // 直接使用，框架已自动应用默认值" << std::endl;
    std::cout << "    int page_num = page.value();" << std::endl;
    std::cout << "    int limit_num = limit.value();" << std::endl;
    std::cout << "    bool is_active = active.value();" << std::endl;
    std::cout << "    " << std::endl;
    std::cout << "    // 可选参数（无默认值）需要检查" << std::endl;
    std::cout << "    if (search.hasValue()) {" << std::endl;
    std::cout << "        std::string keyword = search.value();" << std::endl;
    std::cout << "        // 处理搜索" << std::endl;
    std::cout << "    }" << std::endl;
    std::cout << "    " << std::endl;
    std::cout << "    return HttpResponse(200).json(/* ... */);" << std::endl;
    std::cout << "})" << std::endl;
    
    // ========== 5. DSL 特性总结 ==========
    std::cout << "\n=== 5. DSL 特性总结 ===" << std::endl;
    
    std::cout << "\n支持的类型:" << std::endl;
    std::cout << "  - int: 32位整数" << std::endl;
    std::cout << "  - int64_t: 64位整数" << std::endl;
    std::cout << "  - double: 双精度浮点数" << std::endl;
    std::cout << "  - float: 单精度浮点数" << std::endl;
    std::cout << "  - bool: 布尔值" << std::endl;
    std::cout << "  - std::string: 字符串" << std::endl;
    
    std::cout << "\n参数类型:" << std::endl;
    std::cout << "  - required(): 必需参数" << std::endl;
    std::cout << "  - optional(): 可选参数" << std::endl;
    std::cout << "  - defaultValue(value): 设置默认值（仅可选参数）" << std::endl;
    
    std::cout << "\n验证规则:" << std::endl;
    std::cout << "  - range(min, max): 范围验证（整数/浮点数）" << std::endl;
    std::cout << "  - length(min, max): 长度验证（字符串）" << std::endl;
    std::cout << "  - pattern(regex): 正则验证（字符串）" << std::endl;
    std::cout << "  - oneOf(values): 枚举验证（所有类型）" << std::endl;
    
    std::cout << "\n便捷函数:" << std::endl;
    std::cout << "  - queryParam<T>(name): 创建查询参数" << std::endl;
    std::cout << "  - pathParam<T>(name): 创建路径参数" << std::endl;
    
    std::cout << "\n优势:" << std::endl;
    std::cout << "  ✓ 类型自动推导，无需手动调用 asInt()/asString()" << std::endl;
    std::cout << "  ✓ 类型安全，编译时检查" << std::endl;
    std::cout << "  ✓ 更简洁的 API" << std::endl;
    std::cout << "  ✓ 零运行时开销（编译期优化）" << std::endl;
    
    std::cout << "\n完整 DSL 示例完成！" << std::endl;
    
    return 0;
}
