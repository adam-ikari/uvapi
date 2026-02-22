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
        .param("page", Int(1)).range(1, 1000)
        .param("limit", Int(10)).range(1, 100)
        .param("status", String("active")).oneOf({"active", "inactive", "pending"})
        .param("search", String(""))
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
        });
    
    // 用户详情 API
    api.get("/api/users/:id")
        .pathParam("id", Int()).range(1, INT_MAX)
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
        });
    
    // 创建用户 API
    api.post("/api/users")
        .param("username", String()).length(3, 20)
        .param("email", String()).pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
        .param("age", Int(18)).range(18, 120)
        .param("active", Bool(true))
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(201).json("{\"code\":201,\"message\":\"Created\"}");
        });
    
    // 删除用户 API
    api.del("/api/users/:id")
        .pathParam("id", Int()).range(1, INT_MAX)
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