/**
 * @file crud_server.cpp
 * @brief CRUD 服务器示例
 * 
 * 演示如何使用 UVAPI 实现完整的 CRUD 操作
 */

#include "framework.h"
#include "middleware.h"
#include <unordered_map>
#include <algorithm>
#include <cctype>

using namespace uvapi;

// ========== 数据模型 ==========

// 前向声明
class UserSchema;

// 用户模型
struct User {
    int64_t id;
    std::string username;
    std::string email;
    int age;
    std::vector<std::string> tags;  // 标签数组
    bool active;
    
    // Schema 定义
    static BodySchema<User>* schema() {
        static UserSchema instance;
        return &instance;
    }
};

// 用户 Schema
class UserSchema : public DslBodySchema<User> {
public:
    void define() override {
        REQUIRED_INT64(id, id);
        REQUIRED_STRING(username, username).minLength(3).maxLength(50);
        REQUIRED_STRING(email, email).pattern("^[^@]+@[^@]+$");
        REQUIRED_INT(age, age).range(18, 120);
        OPTIONAL_ARRAY(tags, tags);
        REQUIRED_BOOL(active, active);
    }
};

// 请求模型

// 创建用户请求
struct CreateUserRequest {
    std::string username;
    std::string email;
    int age;
    std::vector<std::string> tags;
    
    static BodySchema<CreateUserRequest>* schema() {
        static CreateUserRequestSchema instance;
        return &instance;
    }
};

class CreateUserRequestSchema : public DslBodySchema<CreateUserRequest> {
public:
    void define() override {
        REQUIRED_STRING(username, username).minLength(3).maxLength(50);
        REQUIRED_STRING(email, email).pattern("^[^@]+@[^@]+$");
        REQUIRED_INT(age, age).range(18, 120);
        OPTIONAL_ARRAY(tags, tags);
    }
};

// 更新用户请求
struct UpdateUserRequest {
    std::string username;
    std::string email;
    int age;
    std::vector<std::string> tags;
    bool active;
    
    static BodySchema<UpdateUserRequest>* schema() {
        static UpdateUserRequestSchema instance;
        return &instance;
    }
};

class UpdateUserRequestSchema : public DslBodySchema<UpdateUserRequest> {
public:
    void define() override {
        OPTIONAL_STRING(username, username).minLength(3).maxLength(50);
        OPTIONAL_STRING(email, email).pattern("^[^@]+@[^@]+$");
        OPTIONAL_INT(age, age).range(18, 120);
        OPTIONAL_ARRAY(tags, tags);
        OPTIONAL_BOOL(active, active);
    }
};

// 响应模型

// 用户列表响应
struct UserListResponse {
    std::vector<User> users;
    int total;
    
    static BodySchema<UserListResponse>* schema() {
        static UserListResponseSchema instance;
        return &instance;
    }
};

class UserListResponseSchema : public DslBodySchema<UserListResponse> {
public:
    void define() override {
        // 注意：数组类型需要在主 Schema 中特殊处理
        // 这里仅作为示例，实际实现需要扩展 DslBodySchema
    }
};

// ========== 数据存储 ==========

class UserRepository {
private:
    std::unordered_map<int64_t, User> users_;
    int64_t next_id_;
    
public:
    UserRepository() : next_id_(1) {}
    
    // 创建用户
    User create(const CreateUserRequest& req) {
        User user;
        user.id = next_id_++;
        user.username = req.username;
        user.email = req.email;
        user.age = req.age;
        user.tags = req.tags;
        user.active = true;
        
        users_[user.id] = user;
        return user;
    }
    
    // 获取用户
    uvapi::optional<User> findById(int64_t id) {
        auto it = users_.find(id);
        if (it != users_.end()) {
            return it->second;
        }
        return uvapi::optional<User>();
    }
    
    // 获取所有用户
    std::vector<User> findAll() {
        std::vector<User> result;
        for (auto& pair : users_) {
            result.push_back(pair.second);
        }
        return result;
    }
    
    // 更新用户
    uvapi::optional<User> update(int64_t id, const UpdateUserRequest& req) {
        auto it = users_.find(id);
        if (it == users_.end()) {
            return uvapi::optional<User>();
        }
        
        User& user = it->second;
        
        // 只更新提供的字段
        if (!req.username.empty()) {
            user.username = req.username;
        }
        if (!req.email.empty()) {
            user.email = req.email;
        }
        if (req.age > 0) {
            user.age = req.age;
        }
        if (!req.tags.empty()) {
            user.tags = req.tags;
        }
        
        // active 字段总是更新（因为它有默认值 false）
        user.active = req.active;
        
        return user;
    }
    
    // 删除用户
    bool remove(int64_t id) {
        return users_.erase(id) > 0;
    }
    
    // 获取用户总数
    size_t count() const {
        return users_.size();
    }
};

// ========== 路由处理器 ==========

UserRepository user_repo;

// POST /api/users - 创建用户
Handler create_user = [](const HttpRequest& req) -> HttpResponse {
    try {
        CreateUserRequest create_req;
        if (!req.parseBody(create_req)) {
            return HttpResponse(400, "Invalid request body");
        }
        
        User user = user_repo.create(create_req);
        return HttpResponse(201).json(user);
    } catch (const std::exception& e) {
        return HttpResponse(500, std::string("Failed to create user: ") + e.what());
    }
};

// GET /api/users - 获取用户列表
Handler get_users = [](const HttpRequest& req) -> HttpResponse {
    try {
        std::vector<User> users = user_repo.findAll();
        
        // 构建 JSON 响应
        cJSON* root = cJSON_CreateObject();
        cJSON* users_array = cJSON_CreateArray();
        
        for (const auto& user : users) {
            cJSON* user_obj = cJSON_CreateObject();
            cJSON_AddNumberToObject(user_obj, "id", user.id);
            cJSON_AddStringToObject(user_obj, "username", user.username.c_str());
            cJSON_AddStringToObject(user_obj, "email", user.email.c_str());
            cJSON_AddNumberToObject(user_obj, "age", user.age);
            cJSON_AddBoolToObject(user_obj, "active", user.active);
            
            // 标签数组
            cJSON* tags_array = cJSON_CreateArray();
            for (const auto& tag : user.tags) {
                cJSON_AddItemToArray(tags_array, cJSON_CreateString(tag.c_str()));
            }
            cJSON_AddItemToObject(user_obj, "tags", tags_array);
            
            cJSON_AddItemToArray(users_array, user_obj);
        }
        
        cJSON_AddItemToObject(root, "users", users_array);
        cJSON_AddNumberToObject(root, "total", static_cast<double>(users.size()));
        
        char* json_str = cJSON_Print(root);
        std::string result(json_str ? json_str : "{}");
        
        if (json_str) free(json_str);
        cJSON_Delete(root);
        
        return HttpResponse(200).body(result).setHeader("Content-Type", "application/json");
    } catch (const std::exception& e) {
        return HttpResponse(500, std::string("Failed to get users: ") + e.what());
    }
};

// GET /api/users/:id - 获取单个用户
Handler get_user = [](const HttpRequest& req) -> HttpResponse {
    try {
        int64_t id = req.path<int64_t>("id");
        uvapi::optional<User> user_opt = user_repo.findById(id);
        
        if (!user_opt.has_value()) {
            return HttpResponse(404, "User not found");
        }
        
        return HttpResponse(200).json(user_opt.value());
    } catch (const std::exception& e) {
        return HttpResponse(500, std::string("Failed to get user: ") + e.what());
    }
};

// PUT /api/users/:id - 更新用户
Handler update_user = [](const HttpRequest& req) -> HttpResponse {
    try {
        int64_t id = req.path<int64_t>("id");
        
        UpdateUserRequest update_req;
        if (!req.parseBody(update_req)) {
            return HttpResponse(400, "Invalid request body");
        }
        
        uvapi::optional<User> user_opt = user_repo.update(id, update_req);
        
        if (!user_opt.has_value()) {
            return HttpResponse(404, "User not found");
        }
        
        return HttpResponse(200).json(user_opt.value());
    } catch (const std::exception& e) {
        return HttpResponse(500, std::string("Failed to update user: ") + e.what());
    }
};

// DELETE /api/users/:id - 删除用户
Handler delete_user = [](const HttpRequest& req) -> HttpResponse {
    try {
        int64_t id = req.path<int64_t>("id");
        
        if (!user_repo.remove(id)) {
            return HttpResponse(404, "User not found");
        }
        
        return HttpResponse(204, "");  // No Content
    } catch (const std::exception& e) {
        return HttpResponse(500, std::string("Failed to delete user: ") + e.what());
    }
};

// ========== 主函数 ==========

int main() {
    // 创建事件循环
    uv_loop_t* loop = uv_default_loop();
    
    // 创建服务器
    uvapi::server::Server app(loop);
    
    // 添加中间件
    app.use(cors());
    app.use(responseTime());
    app.use(errorHandler());
    
    // 注册路由
    app.post("/api/users", create_user);
    app.get("/api/users", get_users);
    app.get("/api/users/:id", get_user);
    app.put("/api/users/:id", update_user);
    app.delete_("/api/users/:id", delete_user);
    
    // 启动服务器
    std::cout << "CRUD Server starting on http://localhost:8080" << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  POST   /api/users       - Create user" << std::endl;
    std::cout << "  GET    /api/users       - List users" << std::endl;
    std::cout << "  GET    /api/users/:id   - Get user" << std::endl;
    std::cout << "  PUT    /api/users/:id   - Update user" << std::endl;
    std::cout << "  DELETE /api/users/:id   - Delete user" << std::endl;
    
    app.listen("0.0.0.0", 8080);
    
    // 运行事件循环
    uv_run(loop, UV_RUN_DEFAULT);
    
    return 0;
}