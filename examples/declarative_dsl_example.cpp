/**
 * @file declarative_dsl_example.cpp
 * @brief 声明式 DSL 使用示例
 */

#include <iostream>
#include "../include/params_dsl.h"

using namespace uvapi;
using namespace restful;

int main() {
    std::cout << "=== 声明式 DSL 示例 ===" << std::endl;
    
    // 示例 1: 用户列表 API 参数
    auto page = queryParam<int>("page")
        .optional()
        .defaultValue(1)
        .range(1, 1000);
    
    auto limit = queryParam<int>("limit")
        .optional()
        .defaultValue(10)
        .range(1, 100);
    
    auto status = queryParam<std::string>("status")
        .optional()
        .defaultValue("active")
        .oneOf({"active", "inactive", "pending"});
    
    auto search = queryParam<std::string>("search")
        .optional()
        .defaultValue("");
    
    std::cout << "用户列表 API 参数定义完成" << std::endl;
    std::cout << "  page: optional, default=1, range=[1, 1000]" << std::endl;
    std::cout << "  limit: optional, default=10, range=[1, 100]" << std::endl;
    std::cout << "  status: optional, default=active, enum=[active, inactive, pending]" << std::endl;
    std::cout << "  search: optional, default=''" << std::endl;
    
    // 示例 2: 用户创建 API 参数
    auto username = queryParam<std::string>("username")
        .required()
        .length(3, 20);
    
    auto email = queryParam<std::string>("email")
        .required()
        .pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    
    auto age = queryParam<int>("age")
        .optional()
        .defaultValue(18)
        .range(18, 120);
    
    auto active = queryParam<bool>("active")
        .optional()
        .defaultValue(true);
    
    std::cout << "\n用户创建 API 参数定义完成" << std::endl;
    std::cout << "  username: required, length=[3, 20]" << std::endl;
    std::cout << "  email: required, pattern=email" << std::endl;
    std::cout << "  age: optional, default=18, range=[18, 120]" << std::endl;
    std::cout << "  active: optional, default=true" << std::endl;
    
    std::cout << "\n声明式 DSL 示例完成！" << std::endl;
    
    return 0;
}