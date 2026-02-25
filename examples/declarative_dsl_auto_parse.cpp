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
            (void)req;

            // 参数已经自动解析和验证，可以直接使用
            int page = std::stoi(params.at("page"));
            int limit = std::stoi(params.at("limit"));
            std::string search = params.at("search");
            std::string sort = params.at("sort");
            std::string order = params.at("order");

            std::cout << "解析的参数: page=" << page << ", limit=" << limit
                      << ", search=" << search << ", sort=" << sort << ", order=" << order << std::endl;

            // 构造响应
            std::string body = "{"
                "\"code\":200,"
                "\"message\":\"Success\","
                "\"data\":{"
                    "\"page\":" + std::to_string(page) + ","
                    "\"limit\":" + std::to_string(limit) + ","
                    "\"search\":\"" + search + "\","
                    "\"sort\":\"" + sort + "\","
                    "\"order\":\"" + order + "\","
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

            // 路径参数已经自动解析
            int user_id = std::stoi(params.at("id"));

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
            (void)req;

            // 参数已经自动验证，无需重复验证
            std::string username = params.at("username");
            std::string email = params.at("email");
            int age = std::stoi(params.at("age"));
            bool active = (params.at("active") == "true");

            std::cout << "创建用户: username=" << username << ", email=" << email
                      << ", age=" << age << ", active=" << active << std::endl;

            std::string body = "{"
                "\"code\":201,"
                "\"message\":\"User created successfully\","
                "\"data\":{"
                    "\"id\":123,"
                    "\"username\":\"" + username + "\","
                    "\"email\":\"" + email + "\","
                    "\"age\":" + std::to_string(age) + ","
                    "\"active\":" + (active ? "true" : "false") +
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
            (void)req;

            int page = std::stoi(params.at("page"));
            int limit = std::stoi(params.at("limit"));
            std::string start_date = params.at("start_date");
            std::string end_date = params.at("end_date");
            std::string status = params.at("status");

            std::cout << "查询订单: page=" << page << ", limit=" << limit
                      << ", start_date=" << start_date << ", end_date=" << end_date
                      << ", status=" << status << std::endl;

            std::string body = "{"
                "\"code\":200,"
                "\"message\":\"Success\","
                "\"data\":{"
                    "\"page\":" + std::to_string(page) + ","
                    "\"limit\":" + std::to_string(limit) + ","
                    "\"start_date\":\"" + start_date + "\","
                    "\"end_date\":\"" + end_date + "\","
                    "\"status\":\"" + status + "\","
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