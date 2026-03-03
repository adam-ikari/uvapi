/**
 * @file declarative_dsl_auto_parse.cpp
 * @brief 声明式 DSL 自动解析示例
 *
 * 展示如何使用 DSL 的自动参数解析和验证功能
 */

#include <iostream>
#include "../include/declarative_dsl.h"

using namespace uvapi;
using namespace uvapi::declarative;

int main() {
    std::cout << "=== 声明式 DSL 自动解析示例 ===" << std::endl;

    ApiBuilder api;

    // 示例 1: 用户列表 API - 自动解析分页和搜索参数
    api.get("/api/users")
        .pagination(PageParam().page(1).limit(20))
        .search(SearchParam())
        .sort(SortParam().field("created_at").order("desc"))
        .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) -> HttpResponse {
            (void)params;  // 使用新的参数访问方式

            // 使用 operator[] 进行类型转换和错误检查
            auto page = req.queryParam["page"];
            auto limit = req.queryParam["limit"];
            auto search = req.queryParam["search"];
            auto sort = req.queryParam["sort"];
            auto order = req.queryParam["order"];
            
            // 检查类型转换是否成功
            if (page.hasError()) {
                return HttpResponse(400)
                    .json("{\"code\":400,\"message\":\"Invalid page parameter: " + page.errorMessage() + "\"}");
            }
            
            if (limit.hasError()) {
                return HttpResponse(400)
                    .json("{\"code\":400,\"message\":\"Invalid limit parameter: " + limit.errorMessage() + "\"}");
            }

            int page_value = page;
            int limit_value = limit;

            std::cout << "解析的参数: page=" << page_value << ", limit=" << limit_value
                      << ", search=" << search.value() << ", sort=" << sort.value() << ", order=" << order.value() << std::endl;

            // 构造响应
            std::string body = "{"
                "\"code\":200,"
                "\"message\":\"Success\","
                "\"data\":{"
                    "\"page\":" + std::to_string(page_value) + ","
                    "\"limit\":" + std::to_string(limit_value) + ","
                    "\"search\":\"" + search.value() + "\","
                    "\"sort\":\"" + sort.value() + "\","
                    "\"order\":\"" + order.value() + "\","
                    "\"total\":100,"
                    "\"users\":[]"
                "}"
                "}";

            return HttpResponse(200).json(body);
        });

    // 示例 2: 用户详情 API - 自动解析路径参数
    api.get("/api/users/:id")
        .pathParam("id", Required<int>()).range(1, INT_MAX)
        .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) -> HttpResponse {
            (void)req;

            // 路径参数已经自动解析，使用 operator[] 进行类型转换
            auto id_param = req.pathParam["id"];
            
            // 检查类型转换是否成功
            if (id_param.hasError()) {
                return HttpResponse(400)
                    .json("{\"code\":400,\"message\":\"Invalid user ID: " + id_param.errorMessage() + "\"}");
            }
            
            int user_id = id_param;

            std::cout << "解析的用户 ID: " << user_id << std::endl;

            std::string body = "{"
                "\"code\":200,"
                "\"message\":\"Success\","
                "\"data\":{"
                    "\"id\":" + std::to_string(user_id) + ","
                    "\"name\":\"John Doe\","
                    "\"email\":\"john@example.com\""
                "}"
                "}";

            return HttpResponse(200).json(body);
        });

    // 示例 3: 创建用户 API - 自动解析查询参数并验证
    api.post("/api/users")
        .param("username", Required<std::string>()).length(3, 20)
        .param("email", Required<std::string>()).pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
        .param("age", OptionalWithDefault<int>(18)).range(18, 120)
        .param("active", OptionalWithDefault<bool>(true))
        .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) -> HttpResponse {
            (void)params;  // 使用新的参数访问方式

            // 使用 operator[] 进行类型转换和错误检查
            auto username = req.queryParam["username"];
            auto email = req.queryParam["email"];
            auto age = req.queryParam["age"];
            auto active = req.queryParam["active"];
            
            // 检查类型转换是否成功
            if (age.hasError()) {
                return HttpResponse(400)
                    .json("{\"code\":400,\"message\":\"Invalid age parameter: " + age.errorMessage() + "\"}");
            }
            
            if (active.hasError()) {
                return HttpResponse(400)
                    .json("{\"code\":400,\"message\":\"Invalid active parameter: " + active.errorMessage() + "\"}");
            }

            // 参数已经自动验证，无需重复验证
            int age_value = age;
            bool active_value = active;

            std::cout << "创建用户: username=" << username.value() << ", email=" << email.value()
                      << ", age=" << age_value << ", active=" << active_value << std::endl;

            std::string body = "{"
                "\"code\":201,"
                "\"message\":\"User created successfully\","
                "\"data\":{"
                    "\"id\":123,"
                    "\"username\":\"" + username.value() + "\","
                    "\"email\":\"" + email.value() + "\","
                    "\"age\":" + std::to_string(age_value) + ","
                    "\"active\":" + (active_value ? "true" : "false") +
                "}"
                "}";

            return HttpResponse(201).json(body);
        });

    // 示例 4: 订单列表 API - 时间范围和状态筛选
    api.get("/api/orders")
        .pagination(PageParam().page(1).limit(20))
        .dateRange("start_date", "end_date")
        .statusFilter({"pending", "paid", "shipped", "completed", "cancelled"}, "pending")
        .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) -> HttpResponse {
            (void)params;  // 使用新的参数访问方式

            // 使用 operator[] 进行类型转换和错误检查
            auto page = req.queryParam["page"];
            auto limit = req.queryParam["limit"];
            auto start_date = req.queryParam["start_date"];
            auto end_date = req.queryParam["end_date"];
            auto status = req.queryParam["status"];
            
            // 检查类型转换是否成功
            if (page.hasError()) {
                return HttpResponse(400)
                    .json("{\"code\":400,\"message\":\"Invalid page parameter: " + page.errorMessage() + "\"}");
            }
            
            if (limit.hasError()) {
                return HttpResponse(400)
                    .json("{\"code\":400,\"message\":\"Invalid limit parameter: " + limit.errorMessage() + "\"}");
            }

            int page_value = page;
            int limit_value = limit;

            std::cout << "查询订单: page=" << page_value << ", limit=" << limit_value
                      << ", start_date=" << start_date.value() << ", end_date=" << end_date.value()
                      << ", status=" << status.value() << std::endl;

            std::string body = "{"
                "\"code\":200,"
                "\"message\":\"Success\","
                "\"data\":{"
                    "\"page\":" + std::to_string(page_value) + ","
                    "\"limit\":" + std::to_string(limit_value) + ","
                    "\"start_date\":\"" + start_date.value() + "\","
                    "\"end_date\":\"" + end_date.value() + "\","
                    "\"status\":\"" + status.value() + "\","
                    "\"orders\":[]"
                "}"
                "}";

            return HttpResponse(200).json(body);
        });

    std::cout << "\n=== 自动解析功能说明 ===" << std::endl;
    std::cout << "1. 自动提取参数 - 从 URL 查询参数和路径参数中提取" << std::endl;
    std::cout << "2. 自动验证参数 - 执行类型、范围、长度、正则等验证规则" << std::endl;
    std::cout << "3. 自动应用默认值 - 可选参数使用默认值" << std::endl;
    std::cout << "4. 验证失败自动返回 400 - 无需手动处理验证错误" << std::endl;
    std::cout << "5. 处理器接收解析后的参数 - 直接使用，无需手动解析" << std::endl;

    std::cout << "\nAPI 定义完成！" << std::endl;
    std::cout << "  GET /api/users - 用户列表（自动解析分页、搜索、排序）" << std::endl;
    std::cout << "  GET /api/users/:id - 用户详情（自动解析路径参数）" << std::endl;
    std::cout << "  POST /api/users - 创建用户（自动验证参数）" << std::endl;
    std::cout << "  GET /api/orders - 订单列表（自动解析时间范围和状态）" << std::endl;

    return 0;
}