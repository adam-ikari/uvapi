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
    
    // 示例 1: 使用命名参数定义用户列表 API
    api.get("/api/users")
        .pagination(PageParam().page(1).limit(20))  // 分页参数
        .search(SearchParam())                         // 搜索参数
        .sort(SortParam().field("created_at").order("desc"))  // 排序参数
        .statusFilter({"active", "inactive", "pending"}, "active")  // 状态筛选
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
        });
    
    // 示例 2: 使用命名参数定义产品列表 API
    api.get("/api/products")
        .pagination(PageParam().page(1).limit(20))
        .search(SearchParam())
        .sort(SortParam().field("created_at").order("desc"))
        .range("min_price", "max_price", RangeParam().min(0).max(1000000))  // 价格范围
        .statusFilter({"available", "out_of_stock", "discontinued"}, "available")
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
        });
    
    // 示例 3: 用户详情 API（路径参数）
    api.get("/api/users/:id")
        .pathParam("id", Required<int>()).range(1, INT_MAX)
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
        });
    
    // 示例 4: 创建用户 API
    api.post("/api/users")
        .param("username", Required<std::string>()).length(3, 20)
        .param("email", Required<std::string>()).pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
        .param("age", OptionalWithDefault<int>(18)).range(18, 120)
        .param("active", OptionalWithDefault<bool>(true))
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(201).json("{\"code\":201,\"message\":\"Created\"}");
        });
    
    // 示例 5: 订单列表 API（时间范围筛选）
    api.get("/api/orders")
        .pagination(PageParam().page(1).limit(20))
        .dateRange("start_date", "end_date")  // 时间范围
        .statusFilter({"pending", "paid", "shipped", "completed", "cancelled"}, "pending")
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
        });
    
    // 示例 6: 日志查询 API
    api.get("/api/logs")
        .pagination(PageParam().page(1).limit(100))
        .dateRange("start_time", "end_time")
        .search(SearchParam())
        .handle([](const HttpRequest& req) -> HttpResponse {
            return HttpResponse(200).json("{\"code\":200,\"message\":\"Success\"}");
        });
    
    std::cout << "API 定义完成" << std::endl;
    std::cout << "  GET /api/users - 用户列表（分页、搜索、排序、状态筛选）" << std::endl;
    std::cout << "  GET /api/products - 产品列表（分页、搜索、排序、价格范围）" << std::endl;
    std::cout << "  GET /api/users/:id - 用户详情" << std::endl;
    std::cout << "  POST /api/users - 创建用户" << std::endl;
    std::cout << "  GET /api/orders - 订单列表（时间范围筛选）" << std::endl;
    std::cout << "  GET /api/logs - 日志查询（时间范围）" << std::endl;
    
    std::cout << "\n声明式 DSL 示例完成！" << std::endl;
    
    return 0;
}
