/**
 * @file response_builder_external.cpp
 * @brief Response Builder 外部声明示例
 * 
 * 演示如何在 handler 函数外部声明 Response 结构，实现 Response DSL 的复用
 * 
 * 核心概念：
 * 1. 使用 ResponseBuilder 在 handler 外部声明响应结构
 * 2. ResponseBuilder 可以复用，减少重复代码
 * 3. 支持链式设置状态码、消息、头部等
 * 4. 自动序列化对象（通过 toJson() 方法）
 */

#include "framework.h"
#include <iostream>

using namespace uvapi;
using namespace uvapi::restful;

// ========== 数据模型 ==========

struct User {
    int64_t id;
    std::string username;
    std::string email;
    int age;
    bool active;
    
    // 自动序列化方法
    std::string toJson() const {
        return JSON::Object()
            .set("id", id)
            .set("username", username)
            .set("email", email)
            .set("age", age)
            .set("active", active)
            .toCompactString();
    }
};

struct Post {
    int64_t id;
    int64_t user_id;
    std::string title;
    std::string content;
    
    std::string toJson() const {
        return JSON::Object()
            .set("id", id)
            .set("user_id", user_id)
            .set("title", title)
            .set("content", content)
            .toCompactString();
    }
};

// ========== 在 handler 外部声明 Response 结构 ==========

// 注意：框架已提供以下工厂函数，无需重复定义：
// - makeSuccessResponse()
// - makeCreatedResponse()
// - makeErrorResponse()
// - makeNotFoundResponse()
// - makeListResponse()

// 这些函数在 include/framework.h 中定义，直接使用即可

// ========== Handler 函数 ==========

// Handler 1: 创建用户（使用隐式转换）
auto create_user_handler = [](const HttpRequest& req) -> HttpResponse {
    // 解析请求数据
    JSON::Parser parser(req.body);
    
    User user;
    user.id = 1;
    user.username = parser.getString("username", "newuser");
    user.email = parser.getString("email", "user@example.com");
    user.age = parser.getInt("age", 25);
    user.active = true;
    
    // 使用声明式风格：描述响应属性
    // 方法 1：显式调用 toHttpResponse()
    // return ResponseBuilder::created()
    //     .message("User created successfully")
    //     .data(user)
    //     ;
    
    // 方法 2：隐式转换（自动调用）
    return ResponseBuilder::created()
        .message("User created successfully")
        .data(user);  // 自动转换为 HttpResponse
};

// Handler 2: 获取用户（使用隐式转换）
auto get_user_handler = [](const HttpRequest& req) -> HttpResponse {
    User user;
    user.id = 1;
    user.username = "alice";
    user.email = "alice@example.com";
    user.age = 30;
    user.active = true;
    
    // 使用工厂函数 + 隐式转换
    return makeSuccessResponse()
        .data(user);  // 自动转换为 HttpResponse
};

// Handler 3: 获取用户列表（使用隐式转换）
auto list_users_handler = [](const HttpRequest& req) -> HttpResponse {
    std::vector<User> users = {
        {1, "alice", "alice@example.com", 25, true},
        {2, "bob", "bob@example.com", 30, true},
        {3, "charlie", "charlie@example.com", 35, false}
    };
    
    // 使用工厂函数 + 隐式转换
    return makeListResponse()
        .data(users);  // 自动转换为 HttpResponse
};

// Handler 4: 获取帖子（使用显式转换，保持代码可读性）
auto get_post_handler = [](const HttpRequest& req) -> HttpResponse {
    Post post;
    post.id = 1;
    post.user_id = 1;
    post.title = "Hello World";
    post.content = "This is my first post";
    
    // 使用声明式风格 + 显式转换（提高可读性）
    return ResponseBuilder::ok()
        .cacheControl("no-cache")
        .data(post)
        ;  // 显式转换，明确意图
};

// Handler 5: 动态构建响应（声明式风格 + 隐式转换）
auto dynamic_response_handler = [](const HttpRequest& req) -> HttpResponse {
    // 描述响应具有的属性
    auto response = ResponseBuilder::ok("Dynamic response")
        .header("X-Custom-Header", "custom-value")
        .requestId("12345");
    
    User user;
    user.id = 1;
    user.username = "dynamic_user";
    user.email = "dynamic@example.com";
    user.age = 28;
    user.active = true;
    
    return response.data(user);  // 隐式转换
};

// Handler 6: 错误响应（使用显式转换，提高可读性）
auto error_handler = [](const HttpRequest& /*req*/) -> HttpResponse {
    // 使用工厂函数 + 显式转换
    return makeErrorResponse()
        .data("{\"error\":\"Invalid input\"}")
        ;  // 显式转换，明确这是错误响应
};

// Handler 7: 未找到响应（使用隐式转换）
auto not_found_handler = [](const HttpRequest& /*req*/) -> HttpResponse {
    // 使用工厂函数 + 隐式转换
    return makeNotFoundResponse()
        .data("{\"error\":\"Resource not found\"}");  // 隐式转换
};

// ========== 主程序 ==========

int main() {
    std::cout << "=== Response Builder 外部声明示例（已修复） ===" << std::endl << std::endl;
    std::cout << "修复内容：" << std::endl;
    std::cout << "1. 移除 static 全局变量，使用工厂函数返回局部对象" << std::endl;
    std::cout << "2. 添加错误处理机制（捕获 toJson() 异常）" << std::endl;
    std::cout << "3. 消除代码冗余，提取公共逻辑到私有方法" << std::endl;
    std::cout << "4. 优化性能（链式调用，减少拷贝）" << std::endl;
    std::cout << "5. 纯粹的声明式风格：描述响应属性，而非执行动作" << std::endl;
    std::cout << "6. 常用头部快捷方法：requestId(), traceId()" << std::endl;
    std::cout << "7. 隐式转换： 可选，自动转换为 HttpResponse" << std::endl << std::endl;
    
    // 1. 创建用户
    std::cout << "1. 创建用户（使用工厂函数模板）:" << std::endl;
    HttpRequest create_req;
    create_req.body = "{\"username\":\"alice\",\"email\":\"alice@example.com\",\"age\":25}";
    HttpResponse create_resp = create_user_handler(create_req);
    std::cout << "  响应: " << create_resp.body << std::endl;
    std::cout << "  状态码: " << create_resp.status_code << std::endl;
    std::cout << "  Content-Type: " << create_resp.headers["Content-Type"] << std::endl;
    std::cout << "  Cache-Control: " << create_resp.headers["Cache-Control"] << std::endl << std::endl;
    
    // 2. 获取用户
    std::cout << "2. 获取用户（使用工厂函数模板）:" << std::endl;
    HttpRequest get_req;
    HttpResponse get_resp = get_user_handler(get_req);
    std::cout << "  响应: " << get_resp.body << std::endl;
    std::cout << "  状态码: " << get_resp.status_code << std::endl << std::endl;
    
    // 3. 获取用户列表
    std::cout << "3. 获取用户列表（使用工厂函数模板）:" << std::endl;
    HttpRequest list_req;
    HttpResponse list_resp = list_users_handler(list_req);
    std::cout << "  响应: " << list_resp.body << std::endl;
    std::cout << "  状态码: " << list_resp.status_code << std::endl;
    std::cout << "  Cache-Control: " << list_resp.headers["Cache-Control"] << std::endl << std::endl;
    
    // 4. 获取帖子（复用 success_response 模板）
    std::cout << "4. 获取帖子（复用工厂函数模板）:" << std::endl;
    HttpRequest post_req;
    HttpResponse post_resp = get_post_handler(post_req);
    std::cout << "  响应: " << post_resp.body << std::endl;
    std::cout << "  状态码: " << post_resp.status_code << std::endl << std::endl;
    
    // 5. 动态构建响应
    std::cout << "5. 动态构建响应（不使用预定义模板）:" << std::endl;
    HttpRequest dynamic_req;
    HttpResponse dynamic_resp = dynamic_response_handler(dynamic_req);
    std::cout << "  响应: " << dynamic_resp.body << std::endl;
    std::cout << "  状态码: " << dynamic_resp.status_code << std::endl;
    std::cout << "  X-Custom-Header: " << dynamic_resp.headers["X-Custom-Header"] << std::endl;
    std::cout << "  X-Request-ID: " << dynamic_resp.headers["X-Request-ID"] << std::endl << std::endl;
    
    // 6. 错误响应
    std::cout << "6. 错误响应（使用工厂函数模板）:" << std::endl;
    HttpRequest error_req;
    HttpResponse error_resp = error_handler(error_req);
    std::cout << "  响应: " << error_resp.body << std::endl;
    std::cout << "  状态码: " << error_resp.status_code << std::endl << std::endl;
    
    // 7. 未找到响应
    std::cout << "7. 未找到响应（使用工厂函数模板）:" << std::endl;
    HttpRequest not_found_req;
    HttpResponse not_found_resp = not_found_handler(not_found_req);
    std::cout << "  响应: " << not_found_resp.body << std::endl;
    std::cout << "  状态码: " << not_found_resp.status_code << std::endl << std::endl;
    
    std::cout << "=== 核心优势 ===" << std::endl;
    std::cout << "1. Response 结构可以在 handler 外部声明，实现复用" << std::endl;
    std::cout << "2. 使用工厂函数返回局部对象，符合零全局变量原则" << std::endl;
    std::cout << "3. 纯粹的声明式风格：描述响应属性，而非执行动作" << std::endl;
    std::cout << "4. 自动序列化对象（通过 toJson() 方法）" << std::endl;
    std::cout << "5. 与 Request DSL 风格统一，学习成本低" << std::endl;
    std::cout << "6. 支持动态构建响应（不使用预定义模板）" << std::endl;
    std::cout << "7. 类型安全，编译期检查" << std::endl;
    std::cout << "8. 错误处理：捕获 toJson() 异常，返回错误响应" << std::endl;
    std::cout << "9. 性能优化：链式调用，减少拷贝" << std::endl;
    std::cout << "10. 常用头部快捷方法：requestId(), traceId()" << std::endl;
    std::cout << "11. 隐式转换： 可选，自动转换为 HttpResponse" << std::endl;
    
    std::cout << "\n=== Response Builder 外部声明示例完成 ===" << std::endl;
    std::cout << "所有问题已修复，符合 DSL 设计哲学！" << std::endl;
    
    return 0;
}