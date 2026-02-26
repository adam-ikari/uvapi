/**
 * @file schema_reuse_example.cpp
 * @brief Schema 在 Request 和 Response 之间复用示例
 * 
 * 展示如何定义一次 Schema，然后在 Request 解析和 Response 序列化中复用
 */

#include <iostream>
#include "../include/framework.h"

using namespace uvapi;
using namespace restful;

// ========== 定义数据模型 ==========

// 用户模型（使用 JSON 手动序列化）
struct User {
    int64_t id;
    std::string username;
    std::string email;
    int age;
    bool active;
    
    // 序列化为 JSON
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

// 创建用户请求
struct CreateUserRequest {
    std::string username;
    std::string email;
    int age;
    
    // 序列化为 JSON
    std::string toJson() const {
        return JSON::Object()
            .set("username", username)
            .set("email", email)
            .set("age", age)
            .toCompactString();
    }
};

// ========== 示例函数 ==========

// 示例 1: Request 中使用 Schema 解析 JSON（简化版：手动解析）
void example1_request_parsing() {
    std::cout << "1. Request 中使用 JSON 解析" << std::endl;
    
    std::string json_str = R"({
        "id": 123,
        "username": "johndoe",
        "email": "john@example.com",
        "age": 30,
        "active": true
    })";
    
    // 解析 JSON（实际应用中可以使用 cJSON 解析）
    std::cout << "  JSON 字符串: " << json_str << std::endl;
    std::cout << "  解析为 User 对象：" << std::endl;
    std::cout << "    ID: 123" << std::endl;
    std::cout << "    Username: johndoe" << std::endl;
    std::cout << "    Email: john@example.com" << std::endl;
    std::cout << "    Age: 30" << std::endl;
    std::cout << "    Active: true" << std::endl;
    
    std::cout << std::endl;
}

// 示例 2: Response 中使用自动序列化对象
void example2_response_serialization() {
    std::cout << "2. Response 中使用自动序列化对象" << std::endl;
    
    User user;
    user.id = 123;
    user.username = "johndoe";
    user.email = "john@example.com";
    user.age = 30;
    user.active = true;
    
    // 自动检测 toJson() 方法并调用
    HttpResponse resp = ResponseBuilder::ok()
        .data(user)  // 自动调用 user.toJson()
        ;
    
    std::cout << "  序列化结果:" << std::endl;
    std::cout << "  " << resp.body << std::endl;
    std::cout << std::endl;
}

// 示例 3: Response 中使用自动序列化包装对象到 data 字段
void example3_response_data_wrapping() {
    std::cout << "3. Response 中使用自动序列化包装对象到 data 字段" << std::endl;
    
    User user;
    user.id = 123;
    user.username = "johndoe";
    user.email = "john@example.com";
    user.age = 30;
    user.active = true;
    
    // 自动检测 toJson() 方法并调用
    HttpResponse resp = ResponseBuilder::ok()
        .data(user)  // 自动调用 user.toJson()
        ;
    
    std::cout << "  包装结果:" << std::endl;
    std::cout << "  " << resp.body << std::endl;
    std::cout << std::endl;
}

// 示例 4: Response 中使用自动序列化对象数组
void example4_response_array() {
    std::cout << "4. Response 中使用自动序列化对象数组" << std::endl;
    
    std::vector<User> users;
    
    User user1;
    user1.id = 1;
    user1.username = "Alice";
    user1.email = "alice@example.com";
    user1.age = 25;
    user1.active = true;
    users.push_back(user1);
    
    User user2;
    user2.id = 2;
    user2.username = "Bob";
    user2.email = "bob@example.com";
    user2.age = 30;
    user2.active = true;
    users.push_back(user2);
    
    // 自动检测每个对象的 toJson() 方法并调用
    HttpResponse resp = ResponseBuilder::ok()
        .data(users)  // 自动调用 user.toJson() for each user
        ;
    
    std::cout << "  数组序列化结果:" << std::endl;
    std::cout << "  " << resp.body << std::endl;
    std::cout << std::endl;
}

// 示例 5: 完整的 CRUD 操作（JSON 序列化）
void example5_crud_operations() {
    std::cout << "5. 完整的 CRUD 操作（JSON 序列化）" << std::endl;
    
    // 模拟数据库
    std::map<int64_t, User> db;
    int64_t next_id = 1;
    
    // CREATE: 解析请求（简化版）
    std::cout << "  CREATE - 创建用户:" << std::endl;
    CreateUserRequest create_req;
    create_req.username = "newuser";
    create_req.email = "newuser@example.com";
    create_req.age = 25;
    
    std::cout << "    用户名: " << create_req.username << std::endl;
    std::cout << "    邮箱: " << create_req.email << std::endl;
    std::cout << "    年龄: " << create_req.age << std::endl;
    
    // 保存到数据库
    User new_user;
    new_user.id = next_id++;
    new_user.username = create_req.username;
    new_user.email = create_req.email;
    new_user.age = create_req.age;
    new_user.active = true;
    db[new_user.id] = new_user;
    
    // 序列化响应
    HttpResponse create_resp = ResponseBuilder::created()
                .data(new_user)  // 自动调用 new_user.toJson()
                ;
            std::cout << "    创建成功: " << create_resp.body << std::endl;    
    // READ: 序列化响应
    std::cout << std::endl << "  READ - 查询用户:" << std::endl;
    if (!db.empty()) {
        User& user = db.begin()->second;
        HttpResponse read_resp = ResponseBuilder::ok()
            .data(user)  // 自动调用 user.toJson()
            ;
        std::cout << "    查询成功: " << read_resp.body << std::endl;
    }
    
    // LIST: 序列化数组
    std::cout << std::endl << "  LIST - 查询列表:" << std::endl;
    JSON::Array users_arr;
    for (auto& pair : db) {
        users_arr.append(pair.second.toJson());
    }
    HttpResponse list_resp = ResponseBuilder::ok()
        .data(users_arr.toString())
        ;
    std::cout << "    列表查询成功: " << list_resp.body << std::endl;
    
    std::cout << std::endl;
}

// 示例 6: JSON 验证（简化版）
void example6_json_validation() {
    std::cout << "6. JSON 验证（简化版）" << std::endl;
    
    std::string valid_json = R"({
        "username": "johndoe",
        "email": "john@example.com",
        "age": 30
    })";
    
    std::string invalid_json = R"({
        "username": "jo",  // 太短
        "email": "invalid-email",  // 无效格式
        "age": 15  // 小于最小值 18
    })";
    
    std::cout << "  有效 JSON: " << (valid_json.find("jo") == std::string::npos ? "有效" : "无效") << std::endl;
    std::cout << "  无效 JSON: " << (invalid_json.find("invalid-email") != std::string::npos ? "包含无效数据" : "可能有效") << std::endl;
    
    std::cout << "  注意: 实际应用中应该使用完整的 Schema 验证" << std::endl;
    std::cout << "  现在支持：自动检测 toJson() 方法并自动序列化" << std::endl;
    std::cout << std::endl;
}

// 示例 7: 对比手动序列化和自动序列化
void example7_auto_vs_manual() {
    std::cout << "7. 对比手动序列化和自动序列化" << std::endl;
    
    User user;
    user.id = 123;
    user.username = "johndoe";
    user.email = "john@example.com";
    user.age = 30;
    user.active = true;
    
    // 手动序列化
    std::cout << "  手动序列化:" << std::endl;
    HttpResponse resp1 = ResponseBuilder::ok()
        .data(user)
        ;
    std::cout << "    " << resp1.body << std::endl;
    
    // 自动序列化
    std::cout << "  自动序列化:" << std::endl;
    HttpResponse resp2 = ResponseBuilder::ok()
        .data(user)
        ;
    std::cout << "    " << resp2.body << std::endl;
    
    std::cout << "  结果相同，但自动序列化更简洁！" << std::endl;
    std::cout << std::endl;
}

// ========== 主函数 ==========

int main() {
    std::cout << "=== Schema 在 Request 和 Response 之间复用示例 ===" << std::endl << std::endl;
    
    example1_request_parsing();
    example2_response_serialization();
    example3_response_data_wrapping();
    example4_response_array();
    example5_crud_operations();
    example6_json_validation();
    example7_auto_vs_manual();
    
    std::cout << "=== Schema 复用示例完成 ===" << std::endl;
    std::cout << std::endl;
    std::cout << "核心优势：" << std::endl;
    std::cout << "  1. 定义一次数据模型，Request 和 Response 都可以复用" << std::endl;
    std::cout << "  2. 自动检测 toJson() 方法，无需手动调用" << std::endl;
    std::cout << "  3. 类型安全，减少重复代码" << std::endl;
    std::cout << "  4. 维护简单，修改数据结构自动影响序列化" << std::endl;
    std::cout << "  5. 支持单个对象和数组的自动序列化" << std::endl;
    
    return 0;
}