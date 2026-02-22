/**
 * @file schema_dsl_example.cpp
 * @brief Schema DSL 示例
 * 
 * 展示如何使用 Schema 定义 Request Body 并在 CRUD API 中复用
 */

#include <iostream>
#include "../include/framework.h"
#include "../include/declarative_dsl.h"

using namespace uvapi;
using namespace restful;
using namespace uvapi::declarative;

// ========== 数据模型 ==========

struct User {
    int id;
    std::string username;
    std::string email;
    int age;
    bool active;
    
    // 实现 schema() 方法（用于 Body 解析）
    static BodySchemaBase* schema() {
        static BodySchema<User> schema_;
        return &schema_;
    }
    
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
// ========== Schema 定义（可复用）==========

auto userSchema = Schema<User>()
    .field("username", Required<std::string>()).length(3, 20)
    .field("email", Required<std::string>()).pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
    .field("age", OptionalWithDefault<int>(18)).range(18, 120)
    .field("active", OptionalWithDefault<bool>(true));

// ========== API 处理器 ==========

// 创建用户
HttpResponse createUser(const HttpRequest& req) {
    auto user = req.parseBody<User>();
    
    // 验证必填字段
    if (!user.hasValue()) {
        return HttpResponse(400)
            .setHeader("Content-Type", "application/json")
            .body("{\"code\":400,\"message\":\"Invalid request body\"}");
    }
    
    User new_user = user.value();
    new_user.id = 1;  // 模拟生成 ID
    
    return HttpResponse(201)
        .setHeader("Content-Type", "application/json")
        .body("{\"code\":201,\"message\":\"User created\",\"data\":" + new_user.toJson() + "}");
}

// 更新用户
HttpResponse updateUser(const HttpRequest& req) {
    auto id = req.path<int>("id");
    auto user = req.parseBody<User>();
    
    if (!id.hasValue()) {
        return HttpResponse(400)
            .setHeader("Content-Type", "application/json")
            .body("{\"code\":400,\"message\":\"User ID is required\"}");
    }
    
    if (!user.hasValue()) {
        return HttpResponse(400)
            .setHeader("Content-Type", "application/json")
            .body("{\"code\":400,\"message\":\"Invalid request body\"}");
    }
    
    User updated_user = user.value();
    updated_user.id = id.value();
    
    return HttpResponse(200)
        .setHeader("Content-Type", "application/json")
        .body("{\"code\":200,\"message\":\"User updated\",\"data\":" + updated_user.toJson() + "}");
}

// ========== 主函数 ==========

int main(int argc, char* argv[]) {
    std::cout << "=== UVAPI Schema DSL 示例 ===" << std::endl;
    std::cout << std::endl;
    
    std::cout << "1. 定义 Schema（可复用）：" << std::endl;
    std::cout << "   auto userSchema = Schema<User>()" << std::endl;
    std::cout << "       .field(\"username\", Required<std::string>()).length(3, 20)" << std::endl;
    std::cout << "       .field(\"email\", Required<std::string>()).pattern(\"...\")" << std::endl;
    std::cout << "       .field(\"age\", OptionalWithDefault<int>(18)).range(18, 120)" << std::endl;
    std::cout << "       .field(\"active\", OptionalWithDefault<bool>(true));" << std::endl;
    std::cout << std::endl;
    
    std::cout << "2. 在创建 API 中使用 Schema：" << std::endl;
    std::cout << "   api.post(\"/api/users\")" << std::endl;
    std::cout << "       .body(userSchema)" << std::endl;
    std::cout << "       .handle([](const HttpRequest& req) -> HttpResponse {" << std::endl;
    std::cout << "           auto user = req.body<User>();  // 类型自动推导" << std::endl;
    std::cout << "           return HttpResponse(201).json(user.toJson());" << std::endl;
    std::cout << "       });" << std::endl;
    std::cout << std::endl;
    
    std::cout << "3. 在更新 API 中复用相同 Schema：" << std::endl;
    std::cout << "   api.put(\"/api/users/:id\")" << std::endl;
    std::cout << "       .pathParam(\"id\", Required<int>())" << std::endl;
    std::cout << "       .body(userSchema)  // 复用相同的 Schema" << std::endl;
    std::cout << "       .handle([](const HttpRequest& req) -> HttpResponse {" << std::endl;
    std::cout << "           auto id = req.path<int>(\"id\");" << std::endl;
    std::cout << "           auto user = req.body<User>();" << std::endl;
    std::cout << "           return HttpResponse(200).json(user.toJson());" << std::endl;
    std::cout << "       });" << std::endl;
    std::cout << std::endl;
    
    std::cout << "核心特点：" << std::endl;
    std::cout << "  - Schema 可复用：在多个 API 中共享" << std::endl;
    std::cout << "  - 类型自动推导：req.body<User>() 自动返回正确的类型" << std::endl;
    std::cout << "  - 校验规则：支持 .length()、.pattern()、.range()、.oneOf()" << std::endl;
    std::cout << "  - 与参数声明一致：使用相同的 Required<T> 和 OptionalWithDefault<T>" << std::endl;
    std::cout << std::endl;
    
    std::cout << "示例请求：" << std::endl;
    std::cout << "  curl -X POST http://localhost:8080/api/users \\" << std::endl;
    std::cout << "       -H \"Content-Type: application/json\" \\" << std::endl;
    std::cout << "       -d '{\"username\":\"alice\",\"email\":\"alice@example.com\",\"age\":25}'" << std::endl;
    std::cout << std::endl;
    std::cout << "  curl -X PUT http://localhost:8080/api/users/1 \\" << std::endl;
    std::cout << "       -H \"Content-Type: application/json\" \\" << std::endl;
    std::cout << "       -d '{\"username\":\"alice\",\"email\":\"alice@newdomain.com\",\"age\":26}'" << std::endl;
    std::cout << std::endl;
    
    return 0;
}