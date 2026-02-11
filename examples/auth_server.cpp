/**
 * @file auth_server.cpp
 * @brief 认证服务器示例
 * 
 * 演示如何使用 UVAPI 中间件实现认证和授权
 */

#include "framework.h"
#include "middleware.h"
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <random>
#include <cctype>

using namespace uvapi;

// ========== 认证相关工具 ==========

// Base64 编码
std::string base64_encode(const std::string& input) {
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int val = 0, valb = -6;
    
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    
    if (valb > -6) {
        result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    
    while (result.size() % 4) {
        result.push_back('=');
    }
    
    return result;
}

// 生成随机 Token
std::string generate_token(size_t length = 32) {
    static const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, chars.size() - 1);
    
    std::string result;
    for (size_t i = 0; i < length; ++i) {
        result += chars[dis(gen)];
    }
    return result;
}

// ========== 数据模型 ==========

// 用户模型
struct User {
    int64_t id;
    std::string username;
    std::string password;  // 实际应用中应该存储哈希值
    std::string email;
    std::string role;  // admin, user, guest
    bool active;
};

// 登录请求
struct LoginRequest {
    std::string username;
    std::string password;
    
    static BodySchema<LoginRequest>* schema() {
        static LoginRequestSchema instance;
        return &instance;
    }
};

class LoginRequestSchema : public DslBodySchema<LoginRequest> {
public:
    void define() override {
        REQUIRED_STRING(username, username).minLength(3);
        REQUIRED_STRING(password, password).minLength(6);
    }
};

// 登录响应
struct LoginResponse {
    std::string token;
    std::string username;
    std::string role;
    int64_t expires_in;
};

// 用户信息响应
struct UserInfoResponse {
    int64_t id;
    std::string username;
    std::string email;
    std::string role;
    bool active;
};

// ========== 数据存储 ==========

class UserRepository {
private:
    std::unordered_map<std::string, User> users_by_username_;
    std::unordered_map<std::string, User> users_by_token_;
    std::unordered_map<std::string, std::string> tokens_;  // token -> username
    int64_t next_id_;
    
public:
    UserRepository() : next_id_(1) {
        // 创建默认管理员账户
        User admin;
        admin.id = next_id_++;
        admin.username = "admin";
        admin.password = "admin123";  // 实际应用中应该使用哈希
        admin.email = "admin@example.com";
        admin.role = "admin";
        admin.active = true;
        users_by_username_[admin.username] = admin;
        
        // 创建普通用户账户
        User user;
        user.id = next_id_++;
        user.username = "user";
        user.password = "user123";
        user.email = "user@example.com";
        user.role = "user";
        user.active = true;
        users_by_username_[user.username] = user;
    }
    
    // 验证用户凭据
    uvapi::optional<User> authenticate(const std::string& username, const std::string& password) {
        auto it = users_by_username_.find(username);
        if (it == users_by_username_.end()) {
            return uvapi::optional<User>();
        }
        
        const User& user = it->second;
        if (user.password != password) {  // 实际应用中应该使用密码哈希比较
            return uvapi::optional<User>();
        }
        
        if (!user.active) {
            return uvapi::optional<User>();
        }
        
        return user;
    }
    
    // 创建 Token
    std::string create_token(const std::string& username) {
        std::string token = generate_token();
        tokens_[token] = username;
        return token;
    }
    
    // 验证 Token
    uvapi::optional<User> get_user_by_token(const std::string& token) {
        auto it = tokens_.find(token);
        if (it == tokens_.end()) {
            return uvapi::optional<User>();
        }
        
        std::string username = it->second;
        auto user_it = users_by_username_.find(username);
        if (user_it == users_by_username_.end()) {
            return uvapi::optional<User>();
        }
        
        return user_it->second;
    }
    
    // 撤销 Token
    bool revoke_token(const std::string& token) {
        return tokens_.erase(token) > 0;
    }
    
    // 获取所有用户（仅管理员）
    std::vector<User> get_all_users() {
        std::vector<User> result;
        for (auto& pair : users_by_username_) {
            result.push_back(pair.second);
        }
        return result;
    }
};

UserRepository user_repo;

// ========== 认证中间件 ==========

// 创建认证中间件
Middleware create_auth_middleware() {
    return [](const HttpRequest& req, Handler next) -> HttpResponse {
        // 检查 Authorization 头
        auto it = req.headers.find("Authorization");
        if (it == req.headers.end()) {
            return HttpResponse(401, R"({"error": "Missing authorization header"})")
                .setHeader("Content-Type", "application/json");
        }
        
        std::string auth_header = it->second;
        
        // 检查 Bearer Token 格式
        if (auth_header.find("Bearer ") != 0) {
            return HttpResponse(401, R"({"error": "Invalid authorization format"})")
                .setHeader("Content-Type", "application/json");
        }
        
        std::string token = auth_header.substr(7);  // 移除 "Bearer "
        
        // 验证 Token
        uvapi::optional<User> user_opt = user_repo.get_user_by_token(token);
        if (!user_opt.has_value()) {
            return HttpResponse(401, R"({"error": "Invalid or expired token"})")
                .setHeader("Content-Type", "application/json");
        }
        
        // Token 有效，执行下一个处理器
        return next(req);
    };
}

// 角色检查中间件
Middleware require_role(const std::string& role) {
    return [=](const HttpRequest& req, Handler next) -> HttpResponse {
        // 获取 Token
        auto it = req.headers.find("Authorization");
        if (it == req.headers.end()) {
            return HttpResponse(401, R"({"error": "Unauthorized"})")
                .setHeader("Content-Type", "application/json");
        }
        
        std::string token = it->second.substr(7);  // 移除 "Bearer "
        
        // 获取用户信息
        uvapi::optional<User> user_opt = user_repo.get_user_by_token(token);
        if (!user_opt.has_value()) {
            return HttpResponse(401, R"({"error": "Unauthorized"})")
                .setHeader("Content-Type", "application/json");
        }
        
        const User& user = user_opt.value();
        
        // 检查角色
        if (user.role != role && user.role != "admin") {
            return HttpResponse(403, R"({"error": "Forbidden: insufficient permissions"})")
                .setHeader("Content-Type", "application/json");
        }
        
        // 角色验证通过，执行下一个处理器
        return next(req);
    };
}

// ========== 路由处理器 ==========

// POST /api/auth/login - 登录
Handler login = [](const HttpRequest& req) -> HttpResponse {
    try {
        LoginRequest login_req;
        if (!req.parseBody(login_req)) {
            return HttpResponse(400, R"({"error": "Invalid request body"})")
                .setHeader("Content-Type", "application/json");
        }
        
        // 验证用户凭据
        uvapi::optional<User> user_opt = user_repo.authenticate(
            login_req.username, login_req.password);
        
        if (!user_opt.has_value()) {
            return HttpResponse(401, R"({"error": "Invalid username or password"})")
                .setHeader("Content-Type", "application/json");
        }
        
        // 创建 Token
        std::string token = user_repo.create_token(login_req.username);
        
        // 构建响应
        std::ostringstream oss;
        oss << "{"
            << "\"token\":\"" << token << "\","
            << "\"username\":\"" << user_opt->username << "\","
            << "\"role\":\"" << user_opt->role << "\","
            << "\"expires_in\":3600"
            << "}";
        
        return HttpResponse(200, oss.str())
            .setHeader("Content-Type", "application/json");
    } catch (const std::exception& e) {
        return HttpResponse(500, R"({"error": "Login failed"})")
            .setHeader("Content-Type", "application/json");
    }
};

// POST /api/auth/logout - 登出
Handler logout = [](const HttpRequest& req) -> HttpResponse {
    try {
        // 获取 Token
        auto it = req.headers.find("Authorization");
        if (it == req.headers.end()) {
            return HttpResponse(200, R"({"message": "Logged out successfully"})")
                .setHeader("Content-Type", "application/json");
        }
        
        std::string token = it->second.substr(7);  // 移除 "Bearer "
        
        // 撤销 Token
        user_repo.revoke_token(token);
        
        return HttpResponse(200, R"({"message": "Logged out successfully"})")
            .setHeader("Content-Type", "application/json");
    } catch (const std::exception& e) {
        return HttpResponse(500, R"({"error": "Logout failed"})")
            .setHeader("Content-Type", "application/json");
    }
};

// GET /api/auth/me - 获取当前用户信息
Handler get_current_user = [](const HttpRequest& req) -> HttpResponse {
    try {
        // 获取 Token
        auto it = req.headers.find("Authorization");
        if (it == req.headers.end()) {
            return HttpResponse(401, R"({"error": "Unauthorized"})")
                .setHeader("Content-Type", "application/json");
        }
        
        std::string token = it->second.substr(7);  // 移除 "Bearer "
        
        // 获取用户信息
        uvapi::optional<User> user_opt = user_repo.get_user_by_token(token);
        if (!user_opt.has_value()) {
            return HttpResponse(401, R"({"error": "Unauthorized"})")
                .setHeader("Content-Type", "application/json");
        }
        
        const User& user = user_opt.value();
        
        // 构建响应
        std::ostringstream oss;
        oss << "{"
            << "\"id\":" << user.id << ","
            << "\"username\":\"" << user.username << "\","
            << "\"email\":\"" << user.email << "\","
            << "\"role\":\"" << user.role << "\","
            << "\"active\":" << (user.active ? "true" : "false")
            << "}";
        
        return HttpResponse(200, oss.str())
            .setHeader("Content-Type", "application/json");
    } catch (const std::exception& e) {
        return HttpResponse(500, R"({"error": "Failed to get user info"})")
            .setHeader("Content-Type", "application/json");
    }
};

// GET /api/admin/users - 获取所有用户（仅管理员）
Handler get_all_users = [](const HttpRequest& req) -> HttpResponse {
    try {
        std::vector<User> users = user_repo.get_all_users();
        
        // 构建响应
        std::ostringstream oss;
        oss << "{\"users\":[";
        
        bool first = true;
        for (const auto& user : users) {
            if (!first) {
                oss << ",";
            }
            first = false;
            
            oss << "{"
                << "\"id\":" << user.id << ","
                << "\"username\":\"" << user.username << "\","
                << "\"email\":\"" << user.email << "\","
                << "\"role\":\"" << user.role << "\","
                << "\"active\":" << (user.active ? "true" : "false")
                << "}";
        }
        
        oss << "],\"total\":" << users.size() << "}";
        
        return HttpResponse(200, oss.str())
            .setHeader("Content-Type", "application/json");
    } catch (const std::exception& e) {
        return HttpResponse(500, R"({"error": "Failed to get users"})")
            .setHeader("Content-Type", "application/json");
    }
};

// 公开路由
Handler public_endpoint = [](const HttpRequest& req) -> HttpResponse {
    return HttpResponse(200, R"({"message": "This is a public endpoint"})")
        .setHeader("Content-Type", "application/json");
};

// 受保护路由
Handler protected_endpoint = [](const HttpRequest& req) -> HttpResponse {
    return HttpResponse(200, R"({"message": "This is a protected endpoint"})")
        .setHeader("Content-Type", "application/json");
};

// 管理员路由
Handler admin_endpoint = [](const HttpRequest& req) -> HttpResponse {
    return HttpResponse(200, R"({"message": "This is an admin-only endpoint"})")
        .setHeader("Content-Type", "application/json");
};

// ========== 主函数 ==========

int main() {
    // 创建事件循环
    uv_loop_t* loop = uv_default_loop();
    
    // 创建服务器
    uvapi::server::Server app(loop);
    
    // 添加全局中间件
    app.use(cors());
    app.use(responseTime());
    app.use(errorHandler());
    
    // 注册公开路由（无需认证）
    app.post("/api/auth/login", login);
    app.post("/api/auth/logout", logout);
    app.get("/public", public_endpoint);
    
    // 注册受保护路由（需要认证）
    app.get("/api/auth/me", create_auth_middleware(), get_current_user);
    app.get("/protected", create_auth_middleware(), protected_endpoint);
    
    // 注册管理员路由（需要认证 + 管理员角色）
    app.get("/api/admin/users", create_auth_middleware(), require_role("admin"), get_all_users);
    app.get("/admin", create_auth_middleware(), require_role("admin"), admin_endpoint);
    
    // 启动服务器
    std::cout << "Auth Server starting on http://localhost:8080" << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  POST   /api/auth/login   - Login" << std::endl;
    std::cout << "  POST   /api/auth/logout  - Logout" << std::endl;
    std::cout << "  GET    /api/auth/me     - Get current user (auth required)" << std::endl;
    std::cout << "  GET    /public          - Public endpoint (no auth)" << std::endl;
    std::cout << "  GET    /protected       - Protected endpoint (auth required)" << std::endl;
    std::cout << "  GET    /api/admin/users - List all users (admin only)" << std::endl;
    std::cout << "  GET    /admin           - Admin endpoint (admin only)" << std::endl;
    std::cout << std::endl;
    std::cout << "Default accounts:" << std::endl;
    std::cout << "  Admin: username=admin, password=admin123" << std::endl;
    std::cout << "  User:  username=user,  password=user123" << std::endl;
    
    app.listen("0.0.0.0", 8080);
    
    // 运行事件循环
    uv_run(loop, UV_RUN_DEFAULT);
    
    return 0;
}