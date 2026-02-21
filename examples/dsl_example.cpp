/**
 * @file dsl_example.cpp
 * @brief DSL 语法糖使用示例
 * 
 * 演示如何使用 DSL 语法糖和便捷方法来快速构建 RESTful API
 */

#include "framework.h"
#include "dsl.h"
#include <iostream>

using namespace uvapi;
using namespace restful;

// ========== 示例数据模型 ==========

struct User {
    int64_t id;
    std::string username;
    std::string email;
    bool active;
    
    // 定义 Schema
    class UserSchema : public DslBodySchema<User> {
    public:
        void define() override {
            field(createField("id", types::integer(), offsetof(User, id)).required());
            field(createField("username", types::string(), offsetof(User, username)).required().minLength(3).maxLength(20));
            field(createField("email", types::string(), offsetof(User, email)).required().pattern("^[\\w\\.-]+@[\\w\\.-]+\\.\\w+$"));
            field(createField("active", types::boolean(), offsetof(User, active)).required());
        }
    };
    
    BodySchemaBase* schema() const {
        static UserSchema instance;
        return &instance;
    }
};

struct CreateUserRequest {
    std::string username;
    std::string email;
    
    class CreateUserRequestSchema : public DslBodySchema<CreateUserRequest> {
    public:
        void define() override {
            field(createField("username", types::string(), offsetof(CreateUserRequest, username)).required().minLength(3).maxLength(20));
            field(createField("email", types::string(), offsetof(CreateUserRequest, email)).required().pattern("^[\\w\\.-]+@[\\w\\.-]+\\.\\w+$"));
        }
    };
    
    BodySchemaBase* schema() const {
        static CreateUserRequestSchema instance;
        return &instance;
    }
};

struct UpdateUserRequest {
    uvapi::optional<std::string> username;
    uvapi::optional<std::string> email;
    uvapi::optional<bool> active;
    
    class UpdateUserRequestSchema : public DslBodySchema<UpdateUserRequest> {
    public:
        void define() override {
            field(createField("username", types::string(), offsetof(UpdateUserRequest, username)).optional().minLength(3).maxLength(20).useOptional());
            field(createField("email", types::string(), offsetof(UpdateUserRequest, email)).optional().pattern("^[\\w\\.-]+@[\\w\\.-]+\\.\\w+$").useOptional());
            field(createField("active", types::boolean(), offsetof(UpdateUserRequest, active)).optional().useOptional());
        }
    };
    
    BodySchemaBase* schema() const {
        static UpdateUserRequestSchema instance;
        return &instance;
    }
};

// ========== 内存存储（模拟数据库）==========

struct UserRepository {
    std::map<int64_t, User> users;
    int64_t next_id = 1;
    
    User create(const CreateUserRequest& req) {
        User user;
        user.id = next_id++;
        user.username = req.username;
        user.email = req.email;
        user.active = true;
        users[user.id] = user;
        return user;
    }
    
    User* find(int64_t id) {
        auto it = users.find(id);
        return (it != users.end()) ? &it->second : nullptr;
    }
    
    bool update(int64_t id, const UpdateUserRequest& req) {
        auto it = users.find(id);
        if (it == users.end()) return false;
        
        if (req.username.has_value()) it->second.username = req.username.value();
        if (req.email.has_value()) it->second.email = req.email.value();
        if (req.active.has_value()) it->second.active = req.active.value();
        
        return true;
    }
    
    bool remove(int64_t id) {
        return users.erase(id) > 0;
    }
    
    std::vector<User> list() {
        std::vector<User> result;
        for (const auto& pair : users) {
            result.push_back(pair.second);
        }
        return result;
    }
};

// ========== 使用 DSL 语法糖的 API ==========

int main() {
    std::cout << "=== DSL 语法糖示例 ===" << std::endl;
    
    uv_loop_t* loop = uv_default_loop();
    Api app(loop);
    
    UserRepository repo;
    
    // ========== 使用资源路由（RESTful CRUD）==========
    
    app.get("/api/users", [&repo](const HttpRequest& req) -> HttpResponse {
        // 获取所有用户
        std::vector<User> users = repo.list();
        
        // 手动构建 JSON 数组
        std::string json_array = "[";
        for (size_t i = 0; i < users.size(); ++i) {
            if (i > 0) json_array += ",";
            json_array += uvapi::toJson(users[i]);
        }
        json_array += "]";
        
        Response resp(200);
        resp.headers["Content-Type"] = "application/json";
        resp.body = uvapi::jsonSuccess(json_array);
        return resp;
    });
    
    app.post("/api/users", [&repo](const HttpRequest& req) -> HttpResponse {
        // 创建用户（使用 parseBody 便捷函数）
        auto result = parseBody<CreateUserRequest>(req);
        if (!result.success) {
            HttpResponse resp(400);
            resp.headers["Content-Type"] = "application/json";
            resp.body = uvapi::jsonError(result.error);
            return resp;
        }
        
        User user = repo.create(result.instance);
        HttpResponse resp(200);
        resp.headers["Content-Type"] = "application/json";
        resp.body = uvapi::jsonSuccess(uvapi::toJson(user));
        return resp;
    });
    
    app.get("/api/users/:id", [&repo](const HttpRequest& req) -> HttpResponse {
        // 获取单个用户
        auto it = req.path_params.find("id");
        if (it == req.path_params.end()) {
            HttpResponse resp(400);
            resp.headers["Content-Type"] = "application/json";
            resp.body = uvapi::jsonError("Missing ID parameter");
            return resp;
        }
        
        int64_t id;
        try {
            id = std::stoll(it->second);
        } catch (...) {
            HttpResponse resp(400);
            resp.headers["Content-Type"] = "application/json";
            resp.body = uvapi::jsonError("Invalid ID format");
            return resp;
        }
        
        User* user = repo.find(id);
        if (!user) {
            HttpResponse resp(404);
            resp.headers["Content-Type"] = "application/json";
            resp.body = uvapi::jsonError("User not found");
            return resp;
        }
        
        Response resp(200);
        resp.headers["Content-Type"] = "application/json";
        resp.body = uvapi::jsonSuccess(uvapi::toJson(*user));
        return resp;
    });
    
    app.put("/api/users/:id", [&repo](const HttpRequest& req) -> HttpResponse {
        // 更新用户
        auto it = req.path_params.find("id");
        if (it == req.path_params.end()) {
            HttpResponse resp(400);
            resp.headers["Content-Type"] = "application/json";
            resp.body = uvapi::jsonError("Missing ID parameter");
            return resp;
        }
        
        int64_t id;
        try {
            id = std::stoll(it->second);
        } catch (...) {
            HttpResponse resp(400);
            resp.headers["Content-Type"] = "application/json";
            resp.body = uvapi::jsonError("Invalid ID format");
            return resp;
        }
        
        auto result = parseBody<UpdateUserRequest>(req);
        if (!result.success) {
            HttpResponse resp(400);
            resp.headers["Content-Type"] = "application/json";
            resp.body = uvapi::jsonError(result.error);
            return resp;
        }
        
        if (!repo.update(id, result.instance)) {
            HttpResponse resp(404);
            resp.headers["Content-Type"] = "application/json";
            resp.body = uvapi::jsonError("User not found");
            return resp;
        }
        
        User* user = repo.find(id);
        HttpResponse resp(200);
        resp.headers["Content-Type"] = "application/json";
        resp.body = uvapi::jsonSuccess(uvapi::toJson(*user));
        return resp;
    });
    
    app.delete_("/api/users/:id", [&repo](const HttpRequest& req) -> HttpResponse {
        // 删除用户
        auto it = req.path_params.find("id");
        if (it == req.path_params.end()) {
            HttpResponse resp(400);
            resp.headers["Content-Type"] = "application/json";
            resp.body = uvapi::jsonError("Missing ID parameter");
            return resp;
        }
        
        int64_t id;
        try {
            id = std::stoll(it->second);
        } catch (...) {
            HttpResponse resp(400);
            resp.headers["Content-Type"] = "application/json";
            resp.body = uvapi::jsonError("Invalid ID format");
            return resp;
        }
        
        if (!repo.remove(id)) {
            HttpResponse resp(404);
            resp.headers["Content-Type"] = "application/json";
            resp.body = uvapi::jsonError("User not found");
            return resp;
        }
        
        HttpResponse resp(200);
        resp.headers["Content-Type"] = "application/json";
        resp.body = uvapi::jsonSuccess("{}");
        return resp;
    });
    
    // ========== 基础路由 ==========
    
    app.get("/api/ping", [](const HttpRequest& req) -> HttpResponse {
        Response resp(200);
        resp.headers["Content-Type"] = "application/json";
        resp.body = uvapi::jsonSuccess("pong");
        return resp;
    });
    
    app.get("/api/health", [](const HttpRequest& req) -> HttpResponse {
        Response resp(200);
        resp.headers["Content-Type"] = "application/json";
        resp.body = uvapi::jsonSuccess(R"({"status":"healthy"})");
        return resp;
    });
    
    app.get("/api/info", [](const HttpRequest& req) -> HttpResponse {
        Response resp(200);
        resp.headers["Content-Type"] = "application/json";
        resp.body = uvapi::jsonSuccess(R"({"name":"uvapi","version":"1.0.0"})");
        return resp;
    });
    
    // ========== 使用参数验证（便捷函数）==========
    
    app.get("/api/search", [](const HttpRequest& req) -> HttpResponse {
        // 直接访问查询参数
        std::string query = "";
        auto it = req.query_params.find("q");
        if (it != req.query_params.end()) {
            query = it->second;
        }
        
        int page = 1;
        auto page_it = req.query_params.find("page");
        if (page_it != req.query_params.end()) {
            try {
                page = std::stoi(page_it->second);
            } catch (...) {}
        }
        
        int limit = 10;
        auto limit_it = req.query_params.find("limit");
        if (limit_it != req.query_params.end()) {
            try {
                limit = std::stoi(limit_it->second);
            } catch (...) {}
        }
        
        if (query.empty()) {
            HttpResponse resp(400);
            resp.headers["Content-Type"] = "application/json";
            resp.body = uvapi::jsonError("Query parameter 'q' is required");
            return resp;
        }
        
        // 返回搜索结果
        std::string result = R"({
            "query": ")" + query + R"(",
            "page": )" + std::to_string(page) + R"(,
            "limit": )" + std::to_string(limit) + R"(,
            "results": []
        })";
        
        HttpResponse resp(200);
        resp.headers["Content-Type"] = "application/json";
        resp.body = uvapi::jsonSuccess(result);
        return resp;
    });
    
    // ========== 错误测试 ==========
    
    app.get("/api/error", [](const HttpRequest& req) -> HttpResponse {
        // 模拟错误
        throw std::runtime_error("Something went wrong!");
        Response resp(200);
        resp.headers["Content-Type"] = "application/json";
        resp.body = uvapi::jsonSuccess("This should never be reached");
        return resp;
    });
    
    // ========== 启动服务器 ==========
    
    std::cout << "服务器启动中..." << std::endl;
    std::cout << "访问 http://127.0.0.1:8080" << std::endl;
    std::cout << "\n可用路由：" << std::endl;
    std::cout << "  GET    /api/ping" << std::endl;
    std::cout << "  GET    /api/health" << std::endl;
    std::cout << "  GET    /api/info" << std::endl;
    std::cout << "  GET    /api/users" << std::endl;
    std::cout << "  POST   /api/users" << std::endl;
    std::cout << "  GET    /api/users/:id" << std::endl;
    std::cout << "  PUT    /api/users/:id" << std::endl;
    std::cout << "  DELETE /api/users/:id" << std::endl;
    std::cout << "  GET    /api/search?q=keyword" << std::endl;
    std::cout << "  GET    /api/error" << std::endl;
    
    if (!app.run("0.0.0.0", 8080)) {
        std::cerr << "服务器启动失败" << std::endl;
        return 1;
    }
    
    return 0;
}
