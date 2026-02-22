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
    
    // 整体式 API 声明
    ApiBuilder api;
    
    api.get("/api/users")
        .param("page", "int", false, "1").range(1, 1000)
        .param("limit", "int", false, "10").range(1, 100)
        .param("status", "string", false, "active").oneOf({"active", "inactive", "pending"})
        .param("search", "string", false, "")
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
        });
    
    api.get("/api/users/:id")
        .pathParam("id", "int", true).range(1, INT_MAX)
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
        });
    
    api.post("/api/users")
        .param("username", "string", true).length(3, 20)
        .param("email", "string", true).pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
        .param("age", "int", false, "18").range(18, 120)
        .param("active", "bool", false, "true")
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(201).json("{\"code\":201,\"message\":\"Created\"}");
        });
    
    api.del("/api/users/:id")
        .pathParam("id", "int", true).range(1, INT_MAX)
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