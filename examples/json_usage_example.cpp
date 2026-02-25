/**
 * @file json_usage_example.cpp
 * @brief JSON 库使用示例
 *
 * 展示如何使用 JSON 类来构造和解析 JSON
 */

#include <iostream>
#include "../include/framework.h"

using namespace uvapi;
using namespace restful;

int main() {
    std::cout << "=== JSON 库使用示例 ===" << std::endl << std::endl;

    // ========== 1. 构造 JSON 对象 ==========
    std::cout << "1. 构造 JSON 对象" << std::endl;

    // 基本对象
    std::string basic_json = JSON::Object()
        .set("name", "John Doe")
        .set("age", 30)
        .set("active", true)
        .setNull("deleted")
        .toString();

    std::cout << "基本对象: " << basic_json << std::endl << std::endl;

    // 嵌套对象
    std::string nested_json = JSON::Object()
        .set("code", 200)
        .set("message", "Success")
        .set("data", JSON::Object()
            .set("id", 123)
            .set("name", "John Doe")
            .set("email", "john@example.com")
            .set("tags", JSON::Array()
                .append("user")
                .append("admin"))
            .toString())
        .toString();

    std::cout << "嵌套对象: " << nested_json << std::endl << std::endl;

    // ========== 2. 构造 JSON 数组 ==========
    std::cout << "2. 构造 JSON 数组" << std::endl;

    std::string array_json = JSON::Array()
        .append("Apple")
        .append("Banana")
        .append(42)
        .append(true)
        .append(3.14)
        .toString();

    std::cout << "简单数组: " << array_json << std::endl << std::endl;

    // 对象数组
    std::string object_array_json = JSON::Array()
        .append(JSON::Object()
            .set("id", 1)
            .set("name", "Alice")
            .toString())
        .append(JSON::Object()
            .set("id", 2)
            .set("name", "Bob")
            .toString())
        .toString();

    std::cout << "对象数组: " << object_array_json << std::endl << std::endl;

    // ========== 3. 快速构造响应 ==========
    std::cout << "3. 快速构造响应" << std::endl;

    std::string success_response = JSON::success("操作成功");
    std::cout << "成功响应: " << success_response << std::endl;

    std::string error_response = JSON::error("参数错误");
    std::cout << "错误响应: " << error_response << std::endl;

    std::string data_response = JSON::data(R"({"total":100,"items":[]})");
    std::cout << "数据响应: " << data_response << std::endl << std::endl;

    // ========== 4. 在 HTTP 响应中使用 ==========
    std::cout << "4. 在 HTTP 响应中使用" << std::endl;

    HttpResponse resp(200);
    resp.header("Content-Type", "application/json");

    // 使用 JSON 构造响应体
    std::string body = JSON::Object()
        .set("code", 200)
        .set("message", "User created successfully")
        .set("data", JSON::Object()
            .set("id", 123)
            .set("username", "johndoe")
            .set("email", "john@example.com")
            .toString())
        .toString();

    resp.setBody(body);
    std::cout << "HTTP 响应体: " << resp.body << std::endl << std::endl;

    // ========== 5. 在声明式 DSL 中使用 ==========
    std::cout << "5. 在声明式 DSL 中使用" << std::endl;

    // 模拟一个 handler，使用 JSON 构造响应
    auto user_list_handler = [](const HttpRequest& req) -> HttpResponse {
        (void)req;

        std::string body = JSON::Object()
            .set("code", 200)
            .set("message", "Success")
            .set("data", JSON::Object()
                .set("page", 1)
                .set("limit", 20)
                .set("total", 100)
                .set("users", JSON::Array()
                    .append(JSON::Object()
                        .set("id", 1)
                        .set("name", "Alice")
                        .toString())
                    .append(JSON::Object()
                        .set("id", 2)
                        .set("name", "Bob")
                        .toString())
                    .toString())
                .toString())
            .toString();

        return HttpResponse(200).json(body);
    };

    std::cout << "用户列表响应示例: " << std::endl;
    HttpResponse result = user_list_handler(HttpRequest());
    std::cout << result.body << std::endl << std::endl;

    // ========== 6. 紧凑 JSON ==========
    std::cout << "6. 紧凑 JSON（用于网络传输）" << std::endl;

    std::string compact_json = JSON::Object()
        .set("code", 200)
        .set("message", "Success")
        .set("data", JSON::Object()
            .set("id", 123)
            .set("name", "John Doe")
            .toString())
        .toCompactString();

    std::cout << "紧凑格式: " << compact_json << std::endl << std::endl;

    std::cout << "=== JSON 使用示例完成 ===" << std::endl;

    return 0;
}