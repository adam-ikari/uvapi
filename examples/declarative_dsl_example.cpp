/**
 * @file declarative_dsl_example.cpp
 * @brief 声明式 DSL 使用示例
 * 
 * 展示如何使用声明式风格的 DSL 定义参数
 */

#include <iostream>
#include "../include/declarative_dsl.h"

using namespace uvapi;
using namespace uvapi::declarative;

int main() {
    std::cout << "=== 声明式 DSL 示例 ===" << std::endl;
    std::cout << std::endl;
    
    // ========== 示例 1: 用户列表 API 参数 ==========
    std::cout << "示例 1: 用户列表 API 参数" << std::endl;
    std::cout << std::endl;
    
    // 声明式定义参数（一次性声明所有信息）
    auto page = OptionalInt("page", 1);
    auto limit = OptionalInt("limit", 10);
    auto status = OptionalString("status", "active");
    auto search = OptionalString("search", "");
    
    // 应用验证规则
    page = Range(page, 1, 1000);
    limit = Range(limit, 1, 100);
    status = OneOf(status, {"active", "inactive", "pending"});
    
    // 创建参数组
    ParamGroup userListParams;
    userListParams.add(page).add(limit).add(status).add(search);
    
    std::cout << "参数定义:" << std::endl;
    std::cout << "  page: " << (page.required ? "required" : "optional") << ", default=" << page.default_value;
    if (page.has_range) std::cout << ", range=[" << page.min_value << ", " << page.max_value << "]";
    std::cout << std::endl;
    
    std::cout << "  limit: " << (limit.required ? "required" : "optional") << ", default=" << limit.default_value;
    if (limit.has_range) std::cout << ", range=[" << limit.min_value << ", " << limit.max_value << "]";
    std::cout << std::endl;
    
    std::cout << "  status: " << (status.required ? "required" : "optional") << ", default=" << status.default_value;
    if (status.has_enum) {
        std::cout << ", enum=[";
        for (size_t i = 0; i < status.enum_values.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << status.enum_values[i];
        }
        std::cout << "]";
    }
    std::cout << std::endl;
    
    std::cout << "  search: " << (search.required ? "required" : "optional") << ", default='" << search.default_value << "'";
    std::cout << std::endl;
    std::cout << std::endl;
    
    // ========== 示例 2: 用户创建 API 参数 ==========
    std::cout << "示例 2: 用户创建 API 参数" << std::endl;
    std::cout << std::endl;
    
    // 声明式定义参数
    auto username = RequiredString("username");
    auto email = RequiredString("email");
    auto age = OptionalInt("age", 18);
    auto active = OptionalBool("active", true);
    
    // 应用验证规则
    username = Length(username, 3, 20);
    email = Pattern(email, "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    age = Range(age, 18, 120);
    
    ParamGroup createUserParams;
    createUserParams.add(username).add(email).add(age).add(active);
    
    std::cout << "参数定义:" << std::endl;
    std::cout << "  username: " << (username.required ? "required" : "optional");
    if (username.has_length) std::cout << ", length=[" << username.min_length << ", " << username.max_length << "]";
    std::cout << std::endl;
    
    std::cout << "  email: " << (email.required ? "required" : "optional");
    if (email.has_pattern) std::cout << ", pattern=" << email.pattern;
    std::cout << std::endl;
    
    std::cout << "  age: " << (age.required ? "required" : "optional") << ", default=" << age.default_value;
    if (age.has_range) std::cout << ", range=[" << age.min_value << ", " << age.max_value << "]";
    std::cout << std::endl;
    
    std::cout << "  active: " << (active.required ? "required" : "optional") << ", default=" << (active.default_value ? "true" : "false");
    std::cout << std::endl;
    std::cout << std::endl;
    
    // ========== 示例 3: 完整的路由配置 ==========
    std::cout << "示例 3: 完整的路由配置" << std::endl;
    std::cout << std::endl;
    
    std::cout << "// 路由配置示例" << std::endl;
    std::cout << "api.get(\"/api/users\", [](const HttpRequest& req) -> HttpResponse {" << std::endl;
    std::cout << "    return getUsers(req);" << std::endl;
    std::cout << "}, QueryParams(page, limit, status, search));" << std::endl;
    std::cout << std::endl;
    
    // ========== 对比：命令式 vs 声明式 ==========
    std::cout << "========== 命令式 vs 声明式对比 ==========" << std::endl;
    std::cout << std::endl;
    
    std::cout << "【命令式 DSL（链式调用）】" << std::endl;
    std::cout << "queryParam<int>(\"page\")" << std::endl;
    std::cout << "    .optional()" << std::endl;
    std::cout << "    .defaultValue(1)" << std::endl;
    std::cout << "    .range(1, 1000);" << std::endl;
    std::cout << std::endl;
    
    std::cout << "【声明式 DSL】" << std::endl;
    std::cout << "auto page = OptionalInt(\"page\", 1);" << std::endl;
    std::cout << "page = Range(page, 1, 1000);" << std::endl;
    std::cout << std::endl;
    
    std::cout << "或更简洁的方式（推荐）：" << std::endl;
    std::cout << "auto page = Range(OptionalInt(\"page\", 1), 1, 1000);" << std::endl;
    std::cout << std::endl;
    
    // ========== 优势总结 ==========
    std::cout << "========== 声明式 DSL 优势 ==========" << std::endl;
    std::cout << std::endl;
    std::cout << "1. 一次性声明：所有参数信息在定义时就确定" << std::endl;
    std::cout << "2. 类型明确：通过函数名明确类型（OptionalInt, RequiredString 等）" << std::endl;
    std::cout << "3. 易于阅读：参数定义清晰，无需跟随链式调用" << std::endl;
    std::cout << "4. 易于测试：可以单独定义和测试每个参数" << std::endl;
    std::cout << "5. 声明式风格：符合\"声明在前，处理在后\"的设计哲学" << std::endl;
    std::cout << std::endl;
    
    std::cout << "声明式 DSL 示例完成！" << std::endl;
    
    return 0;
}
