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
    
    ApiBuilder api;
    
    // 用户列表 API
    api.get("/api/users")
        .param("page", Required<int>())                    // 必需整数
        .param("limit", OptionalWithDefault<int>(10))      // 可选整数，默认值 10
        .param("status", OptionalWithDefault<std::string>("active"))  // 可选字符串，默认值 "active"
        .param("search", OptionalWithDefault<std::string>(""))  // 可选字符串，默认值 ""
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
        });
    
    // 用户详情 API
    api.get("/api/users/:id")
        .pathParam("id", Required<int>())                // 路径参数，必需整数
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
        });
    
    // 创建用户 API
    api.post("/api/users")
        .param("username", Required<std::string>())           // 必需字符串
        .param("email", Required<std::string>())               // 必需字符串
        .param("age", OptionalWithDefault<int>(18))           // 可选整数，默认值 18
        .param("active", OptionalWithDefault<bool>(true))      // 可选布尔，默认值 true
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(201).json("{\"code\":201,\"message\":\"Created\"}");
        });
    
    // 删除用户 API
    api.del("/api/users/:id")
        .pathParam("id", Required<int>())
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(200).json("{\"code\":200,\"message\":\"Deleted\"}");
        });
    
    std::cout << "API 定义完成" << std::endl;
    std::cout << "  GET /api/users - 用户列表" << std::endl;
    std::cout << "  GET /api/users/:id - 用户详情" << std::endl;
    std::cout << "  POST /api/users - 创建用户" << std::endl;
    std::cout << "  DELETE /api/users/:id - 删除用户" << std::endl;
    
    std::cout << "\n声明式 DSL 示例完成！" << std::endl;
    
    return 0;
}
