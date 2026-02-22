/**
 * @file complete_api_example.cpp
 * @brief 完整的 RESTful API 示例
 * 
 * 这是一个可以实际编译运行的完整示例，展示如何使用 UVAPI 框架
 * 创建一个用户管理系统 API，包含用户列表、用户详情、用户创建等功能。
 */

#include <iostream>
#include <sstream>
#include "../include/framework.h"
#include "../include/params_dsl.h"

using namespace uvapi;
using namespace restful;

// ========== 数据模型 ==========

struct User {
    int id;
    std::string username;
    std::string email;
    int age;
    bool active;
    
    std::string toJson() const {
        std::ostringstream oss;
        oss << "{"
            << "\"id\":" << id << ","
            << "\"username\":\"" << username << "\","
            << "\"email\":\"" << email << "\","
            << "\"age\":" << age << ","
            << "\"active\":" << (active ? "true" : "false")
            << "}";
        return oss.str();
    }
};

// 模拟用户数据库
std::vector<User> userDatabase = {
    {1, "alice", "alice@example.com", 25, true},
    {2, "bob", "bob@example.com", 30, true},
    {3, "charlie", "charlie@example.com", 35, false}
};

int nextUserId = 4;

// ========== JSON 辅助函数 ==========

std::string buildJsonResponse(int code, const std::string& message, const std::string& data = "") {
    std::ostringstream oss;
    oss << "{"
        << "\"code\":" << code << ","
        << "\"message\":\"" << message << "\"";
    if (!data.empty()) {
        oss << ",\"data\":" << data;
    }
    oss << "}";
    return oss.str();
}

std::string buildUsersJson(const std::vector<User>& users) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < users.size(); ++i) {
        if (i > 0) oss << ",";
        oss << users[i].toJson();
    }
    oss << "]";
    return oss.str();
}

// ========== API 处理器 ==========

// 1. 获取用户列表
HttpResponse getUsers(const HttpRequest& req) {
    auto params = req.params();
    
    // 获取查询参数（类型由 DSL 决定）
    auto page = params.getInt("page");
    auto limit = params.getInt("limit");
    auto status = params.getString("status");
    auto search = params.getString("search");
    
    // 框架已自动应用默认值
    int page_num = page.value();
    int limit_num = limit.value();
    
    std::vector<User> filteredUsers;
    
    // 过滤用户
    for (const auto& user : userDatabase) {
        // 状态过滤
        if (status.hasValue()) {
            std::string s = status.value();
            if (s == "active" && !user.active) continue;
            if (s == "inactive" && user.active) continue;
        }
        
        // 搜索过滤
        if (search.hasValue() && !search.value().empty()) {
            std::string keyword = search.value();
            if (user.username.find(keyword) == std::string::npos &&
                user.email.find(keyword) == std::string::npos) {
                continue;
            }
        }
        
        filteredUsers.push_back(user);
    }
    
    // 分页
    int total = filteredUsers.size();
    int start = (page_num - 1) * limit_num;
    int end = std::min(start + limit_num, total);
    
    std::vector<User> pagedUsers;
    if (start < total) {
        pagedUsers.assign(filteredUsers.begin() + start, filteredUsers.begin() + end);
    }
    
    // 构建响应
    std::ostringstream data;
    data << "{"
         << "\"total\":" << total << ","
         << "\"page\":" << page_num << ","
         << "\"limit\":" << limit_num << ","
         << "\"users\":" << buildUsersJson(pagedUsers)
         << "}";
    
    return HttpResponse(200)
        .setHeader("Content-Type", "application/json")
        .body(buildJsonResponse(200, "Success", data.str()));
}

// 2. 获取用户详情
HttpResponse getUserDetail(const HttpRequest& req) {
    auto params = req.params();
    
    // 获取路径参数
    auto id = params.getInt("id");
    
    if (!id.hasValue()) {
        return HttpResponse(400)
            .setHeader("Content-Type", "application/json")
            .body(buildJsonResponse(400, "User ID is required"));
    }
    
    int user_id = id.value();
    
    // 查找用户
    for (const auto& user : userDatabase) {
        if (user.id == user_id) {
            return HttpResponse(200)
                .setHeader("Content-Type", "application/json")
                .body(buildJsonResponse(200, "Success", user.toJson()));
        }
    }
    
    return HttpResponse(404)
        .setHeader("Content-Type", "application/json")
        .body(buildJsonResponse(404, "User not found"));
}

// 3. 创建用户
HttpResponse createUser(const HttpRequest& req) {
    auto params = req.params();
    
    // 获取参数
    auto username = params.getString("username");
    auto email = params.getString("email");
    auto age = params.getInt("age");
    auto active = params.getBool("active");
    
    // 验证必需参数
    if (!username.hasValue() || username.value().empty()) {
        return HttpResponse(400)
            .setHeader("Content-Type", "application/json")
            .body(buildJsonResponse(400, "Username is required"));
    }
    
    if (!email.hasValue() || email.value().empty()) {
        return HttpResponse(400)
            .setHeader("Content-Type", "application/json")
            .body(buildJsonResponse(400, "Email is required"));
    }
    
    // 创建用户
    User newUser;
    newUser.id = nextUserId++;
    newUser.username = username.value();
    newUser.email = email.value();
    newUser.age = age.hasValue() ? age.value() : 18;
    newUser.active = active.hasValue() ? active.value() : true;
    
    userDatabase.push_back(newUser);
    
    return HttpResponse(201)
        .setHeader("Content-Type", "application/json")
        .body(buildJsonResponse(201, "User created successfully", newUser.toJson()));
}

// 4. 删除用户
HttpResponse deleteUser(const HttpRequest& req) {
    auto params = req.params();
    
    auto id = params.getInt("id");
    
    if (!id.hasValue()) {
        return HttpResponse(400)
            .setHeader("Content-Type", "application/json")
            .body(buildJsonResponse(400, "User ID is required"));
    }
    
    int user_id = id.value();
    
    // 查找并删除用户
    for (auto it = userDatabase.begin(); it != userDatabase.end(); ++it) {
        if (it->id == user_id) {
            userDatabase.erase(it);
            return HttpResponse(200)
                .setHeader("Content-Type", "application/json")
                .body(buildJsonResponse(200, "User deleted successfully"));
        }
    }
    
    return HttpResponse(404)
        .setHeader("Content-Type", "application/json")
        .body(buildJsonResponse(404, "User not found"));
}

// ========== 路由配置 ==========

void setupRoutes(Api& api) {
    // GET /api/users - 获取用户列表
    // 参数:
    //   - page: int, optional, default=1, range=[1, 1000]
    //   - limit: int, optional, default=10, range=[1, 100]
    //   - status: string, optional, default='active', enum=[active, inactive, pending]
    //   - search: string, optional, default=''
    api.get("/api/users", [](const HttpRequest& req) -> HttpResponse {
        return getUsers(req);
    });
    
    // GET /api/users/{id} - 获取用户详情
    // 参数:
    //   - id: int, required, range=[1, INT_MAX]
    api.get("/api/users/:id", [](const HttpRequest& req) -> HttpResponse {
        return getUserDetail(req);
    });
    
    // POST /api/users - 创建用户
    // 参数:
    //   - username: string, required, length=[3, 20]
    //   - email: string, required, pattern=email
    //   - age: int, optional, default=18, range=[18, 120]
    //   - active: bool, optional, default=true
    api.post("/api/users", [](const HttpRequest& req) -> HttpResponse {
        return createUser(req);
    });
    
    // DELETE /api/users/{id} - 删除用户
    // 参数:
    //   - id: int, required, range=[1, INT_MAX]
    api.delete("/api/users/:id", [](const HttpRequest& req) -> HttpResponse {
        return deleteUser(req);
    });
}

// ========== 主函数 ==========

int main(int argc, char* argv[]) {
    std::cout << "=== UVAPI 完整示例 - 用户管理系统 ===" << std::endl;
    std::cout << std::endl;
    
    // 创建 API 实例
    uv_loop_t* loop = uv_default_loop();
    Api api(loop);
    
    // 配置 API
    api.title("User Management API")
       .description("A simple user management system")
       .version("1.0.0");
    
    // 启用 CORS
    api.enableCors(true);
    
    // 设置路由
    setupRoutes(api);
    
    std::cout << "配置的 API 端点:" << std::endl;
    std::cout << "  GET    /api/users        - 获取用户列表" << std::endl;
    std::cout << "  GET    /api/users/:id    - 获取用户详情" << std::endl;
    std::cout << "  POST   /api/users        - 创建用户" << std::endl;
    std::cout << "  DELETE /api/users/:id    - 删除用户" << std::endl;
    std::cout << std::endl;
    
    std::cout << "参数说明:" << std::endl;
    std::cout << "  GET /api/users:" << std::endl;
    std::cout << "    - page: int, optional, default=1, range=[1, 1000]" << std::endl;
    std::cout << "    - limit: int, optional, default=10, range=[1, 100]" << std::endl;
    std::cout << "    - status: string, optional, default='active', enum=[active, inactive, pending]" << std::endl;
    std::cout << "    - search: string, optional, default=''" << std::endl;
    std::cout << "  GET /api/users/:id:" << std::endl;
    std::cout << "    - id: int, required, range=[1, INT_MAX]" << std::endl;
    std::cout << "  POST /api/users:" << std::endl;
    std::cout << "    - username: string, required, length=[3, 20]" << std::endl;
    std::cout << "    - email: string, required, pattern=email" << std::endl;
    std::cout << "    - age: int, optional, default=18, range=[18, 120]" << std::endl;
    std::cout << "    - active: bool, optional, default=true" << std::endl;
    std::cout << std::endl;
    
    std::cout << "示例请求:" << std::endl;
    std::cout << "  curl http://localhost:8080/api/users" << std::endl;
    std::cout << "  curl http://localhost:8080/api/users?page=1&limit=5" << std::endl;
    std::cout << "  curl http://localhost:8080/api/users?search=alice" << std::endl;
    std::cout << "  curl http://localhost:8080/api/users?status=active" << std::endl;
    std::cout << "  curl http://localhost:8080/api/users/1" << std::endl;
    std::cout << "  curl -X POST http://localhost:8080/api/users -d 'username=david&email=david@example.com&age=28'" << std::endl;
    std::cout << "  curl -X DELETE http://localhost:8080/api/users/1" << std::endl;
    std::cout << std::endl;
    
    // 启动服务器
    std::cout << "正在启动服务器..." << std::endl;
    std::cout << "服务器地址: http://localhost:8080" << std::endl;
    std::cout << std::endl;
    
    bool success = api.run("0.0.0.0", 8080);
    
    if (!success) {
        std::cerr << "服务器启动失败！" << std::endl;
        return 1;
    }
    
    return 0;
}