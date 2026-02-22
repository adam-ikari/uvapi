/**
 * @file declarative_dsl_example.cpp
 * @brief 声明式 DSL 使用示例
 */

#include <iostream>
#include "../include/declarative_dsl.h"

using namespace uvapi;
using namespace uvapi::declarative;

int main() {
    std::cout << "=== 声明式 DSL 示例 ===" << std::endl;
    
    // 示例 1: 用户列表 API 参数
    ParamGroup userListParams = {
        Int("page", false, 1).range(1, 1000),
        Int("limit", false, 10).range(1, 100),
        String("status", false, "active").oneOf({"active", "inactive", "pending"}),
        String("search", false, "")
    };
    
    std::cout << "用户列表 API 参数定义完成" << std::endl;
    std::cout << "  page: optional, default=1, range=[1, 1000]" << std::endl;
    std::cout << "  limit: optional, default=10, range=[1, 100]" << std::endl;
    std::cout << "  status: optional, default=active, enum=[active, inactive, pending]" << std::endl;
    std::cout << "  search: optional, default=''" << std::endl;
    
    // 示例 2: 用户创建 API 参数
    ParamGroup createUserParams = {
        String("username", true, "").length(3, 20),
        String("email", true, "").pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"),
        Int("age", false, 18).range(18, 120),
        Bool("active", false, true)
    };
    
    std::cout << "\n用户创建 API 参数定义完成" << std::endl;
    std::cout << "  username: required, length=[3, 20]" << std::endl;
    std::cout << "  email: required, pattern=email" << std::endl;
    std::cout << "  age: optional, default=18, range=[18, 120]" << std::endl;
    std::cout << "  active: optional, default=true" << std::endl;
    
    std::cout << "\n声明式 DSL 示例完成！" << std::endl;
    
    return 0;
}