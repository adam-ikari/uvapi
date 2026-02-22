/**
 * @file validation_example.cpp
 * @brief 示例：演示自动验证功能，包括正则表达式和枚举值验证
 */

#include "framework.h"
#include <iostream>

using namespace uvapi;

// 定义用户数据模型
struct User {
    std::string username;
    std::string email;
    std::string status;
};

// 定义用户 Schema
class UserSchema : public DslBodySchema<User> {
public:
    void define() override {
        // username: 必填，3-20字符，仅允许字母和数字
        string("username", FIELD_OFFSET(User, username))
            .required()
            .length(3, 20)
            .pattern("^[a-zA-Z0-9]+$");

        // email: 必填，使用内置邮箱格式验证
        email("email", FIELD_OFFSET(User, email))
            .required();

        // status: 必填，枚举值验证
        string("status", FIELD_OFFSET(User, status))
            .required()
            .enumValues({"active", "inactive", "pending"});
    }
};

// 验证用户数据的处理函数
HttpResponse validateUserHandler(const HttpRequest& req) {
    // 解析 JSON 请求体
    User user;
    UserSchema schema;

    std::string validation_error = schema.fromJson(req.body, &user);
    if (!validation_error.empty()) {
        return HttpResponse::json_error(400, "Invalid JSON: " + validation_error);
    }

    // 验证数据
    validation_error = schema.validate(cJSON_Parse(req.body.c_str()));
    if (!validation_error.empty()) {
        return HttpResponse::json_error(400, "Validation failed: " + validation_error);
    }

    // 验证通过，返回成功响应
    cJSON* response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "message", "User validated successfully");
    cJSON_AddStringToObject(response, "username", user.username.c_str());
    cJSON_AddStringToObject(response, "email", user.email.c_str());
    cJSON_AddStringToObject(response, "status", user.status.c_str());

    char* json_str = cJSON_Print(response);
    std::string result(json_str);
    free(json_str);
    cJSON_Delete(response);

    return HttpResponse::json(200, result);
}

int main() {
    std::cout << "=== Validation Example ===" << std::endl;
    std::cout << "This example demonstrates automatic validation including:" << std::endl;
    std::cout << "1. Regex pattern validation (username: letters and numbers only)" << std::endl;
    std::cout << "2. Enum validation (status: active, inactive, pending)" << std::endl;
    std::cout << "3. String length validation (username: 3-20 characters)" << std::endl;
    std::cout << "4. Built-in email format validation" << std::endl;
    std::cout << std::endl;

    // 测试用例 1: 有效数据
    std::cout << "Test 1: Valid user data" << std::endl;
    std::string valid_json = R"({
        "username": "john123",
        "email": "john@example.com",
        "status": "active"
    })";

    User user1;
    UserSchema schema1;
    std::string error1 = schema1.fromJson(valid_json, &user1);
    if (error1.empty()) {
        std::string validation_error1 = schema1.validate(cJSON_Parse(valid_json.c_str()));
        if (validation_error1.empty()) {
            std::cout << "✓ Validation passed" << std::endl;
            std::cout << "  Username: " << user1.username << std::endl;
            std::cout << "  Email: " << user1.email << std::endl;
            std::cout << "  Status: " << user1.status << std::endl;
        } else {
            std::cout << "✗ Validation failed: " << validation_error1 << std::endl;
        }
    } else {
        std::cout << "✗ JSON parsing failed: " << error1 << std::endl;
    }
    std::cout << std::endl;

    // 测试用例 2: 无效的用户名（包含特殊字符）
    std::cout << "Test 2: Invalid username (contains special characters)" << std::endl;
    std::string invalid_username_json = R"({
        "username": "john@123",
        "email": "john@example.com",
        "status": "active"
    })";

    User user2;
    UserSchema schema2;
    std::string error2 = schema2.fromJson(invalid_username_json, &user2);
    if (error2.empty()) {
        std::string validation_error2 = schema2.validate(cJSON_Parse(invalid_username_json.c_str()));
        if (validation_error2.empty()) {
            std::cout << "✗ Validation should have failed" << std::endl;
        } else {
            std::cout << "✓ Validation correctly failed: " << validation_error2 << std::endl;
        }
    } else {
        std::cout << "✗ JSON parsing failed: " << error2 << std::endl;
    }
    std::cout << std::endl;

    // 测试用例 3: 无效的状态值（不在枚举列表中）
    std::cout << "Test 3: Invalid status (not in enum list)" << std::endl;
    std::string invalid_status_json = R"({
        "username": "john123",
        "email": "john@example.com",
        "status": "suspended"
    })";

    User user3;
    UserSchema schema3;
    std::string error3 = schema3.fromJson(invalid_status_json, &user3);
    if (error3.empty()) {
        std::string validation_error3 = schema3.validate(cJSON_Parse(invalid_status_json.c_str()));
        if (validation_error3.empty()) {
            std::cout << "✗ Validation should have failed" << std::endl;
        } else {
            std::cout << "✓ Validation correctly failed: " << validation_error3 << std::endl;
        }
    } else {
        std::cout << "✗ JSON parsing failed: " << error3 << std::endl;
    }
    std::cout << std::endl;

    // 测试用例 4: 用户名太短
    std::cout << "Test 4: Username too short" << std::endl;
    std::string short_username_json = R"({
        "username": "jo",
        "email": "john@example.com",
        "status": "active"
    })";

    User user4;
    UserSchema schema4;
    std::string error4 = schema4.fromJson(short_username_json, &user4);
    if (error4.empty()) {
        std::string validation_error4 = schema4.validate(cJSON_Parse(short_username_json.c_str()));
        if (validation_error4.empty()) {
            std::cout << "✗ Validation should have failed" << std::endl;
        } else {
            std::cout << "✓ Validation correctly failed: " << validation_error4 << std::endl;
        }
    } else {
        std::cout << "✗ JSON parsing failed: " << error4 << std::endl;
    }
    std::cout << std::endl;

    // 测试用例 5: 无效的邮箱格式
    std::cout << "Test 5: Invalid email format" << std::endl;
    std::string invalid_email_json = R"({
        "username": "john123",
        "email": "invalid-email",
        "status": "active"
    })";

    User user5;
    UserSchema schema5;
    std::string error5 = schema5.fromJson(invalid_email_json, &user5);
    if (error5.empty()) {
        std::string validation_error5 = schema5.validate(cJSON_Parse(invalid_email_json.c_str()));
        if (validation_error5.empty()) {
            std::cout << "✗ Validation should have failed" << std::endl;
        } else {
            std::cout << "✓ Validation correctly failed: " << validation_error5 << std::endl;
        }
    } else {
        std::cout << "✗ JSON parsing failed: " << error5 << std::endl;
    }
    std::cout << std::endl;

    std::cout << "=== All tests completed ===" << std::endl;

    return 0;
}