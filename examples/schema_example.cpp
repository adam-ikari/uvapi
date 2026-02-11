/**
 * @file schema_example.cpp
 * @brief Schema DSL 示例 - 演示自动计算字段偏移量
 */

#include <iostream>
#include "schema_dsl.h"

using namespace uvapi;

// ========== 用户模型 ==========

struct User {
    int64_t id;
    std::string username;
    std::string email;
    std::string website;
    std::string user_id;  // UUID
    std::string birth_date;  // DATE
    std::string created_at;  // DATETIME
    int age;
    bool active;
};

// ========== Schema 定义 ==========

class UserSchema : public DslBodySchema<User> {
public:
    void define() override {
        // 基础类型
        this->field(&User::id, "id").asInt64().required();
        this->field(&User::username, "username").asString().required()
            .minLength(3).maxLength(20);
        this->field(&User::age, "age").asInt32().required()
            .range(18, 120);
        this->field(&User::active, "active").asBool().required();
        
        // 高级数据类型
        this->field(&User::email, "email").asEmail().required();
        this->field(&User::website, "website").asUrl().optional();
        this->field(&User::user_id, "user_id").asUuid().required();
        this->field(&User::birth_date, "birth_date").asDate().required();
        this->field(&User::created_at, "created_at").asDatetime().required();
    }
};

// ========== 测试函数 ==========

void test_schema_features() {
    std::cout << "=== Schema DSL 特性测试 ===" << std::endl;
    
    std::cout << "自动偏移量计算优势:" << std::endl;
    std::cout << "  1. 无需手动计算 offsetof" << std::endl;
    std::cout << "  2. 编译器保证类型安全" << std::endl;
    std::cout << "  3. 重构时自动更新偏移量" << std::endl;
    std::cout << "  4. 代码更简洁易读" << std::endl;
    
std::cout << "支持的验证规则:" << std::endl;
    std::cout << "  - required() / optional()" << std::endl;
    std::cout << "  - minLength() / maxLength() / length()" << std::endl;
    std::cout << "  - min() / max() / range()" << std::endl;
    std::cout << "  - pattern()" << std::endl;
    std::cout << "  - oneOf()" << std::endl;
    
    std::cout << "\n支持的高级数据类型:" << std::endl;
    std::cout << "  - date() - 日期（YYYY-MM-DD）" << std::endl;
    std::cout << "  - datetime() - 日期时间（YYYY-MM-DD HH:MM:SS）" << std::endl;
    std::cout << "  - email() - 邮箱" << std::endl;
    std::cout << "  - url() - URL" << std::endl;
    std::cout << "  - uuid() - UUID" << std::endl;
}

void test_functionality() {
    std::cout << "\n=== 功能测试 ===" << std::endl;
    
    User user;
    UserSchema schema;
    schema.define();  // 初始化字段定义
    
    // 测试序列化
    user.id = 1;
    user.username = "testuser";
    user.email = "test@example.com";
    user.website = "https://example.com";
    user.user_id = "550e8400-e29b-41d4-a716-446655440000";
    user.birth_date = "1990-01-15";
    user.created_at = "2024-01-01 12:00:00";
    user.age = 34;
    user.active = true;
    
    std::string json = schema.toJson(&user);
    std::cout << "序列化结果: " << json << std::endl;
    
    // 测试验证（通过序列化反序列化）
    std::string json_str = schema.toJson(&user);
    User user2;
    bool success = schema.fromJson(json_str, &user2);
    std::cout << "序列化/反序列化: " << (success ? "成功" : "失败") << std::endl;
    if (success) {
        std::cout << "用户名: " << user2.username << std::endl;
        std::cout << "邮箱: " << user2.email << std::endl;
    }
    
    // 测试字段验证
    std::cout << "\n字段测试:" << std::endl;
    std::cout << "有效用户名长度: 3-20 字符" << std::endl;
    std::cout << "  'testuser' (" << user.username.length() << " 字符): " 
              << (user.username.length() >= 3 && user.username.length() <= 20 ? "有效" : "无效") << std::endl;
    
    user.username = "ab";  // 太短
    std::cout << "  'ab' (" << user.username.length() << " 字符): " 
              << (user.username.length() >= 3 && user.username.length() <= 20 ? "有效" : "无效") << std::endl;
    
    std::cout << "\n高级数据类型测试:" << std::endl;
    std::cout << "邮箱: " << user.email << std::endl;
    std::cout << "URL: " << user.website << std::endl;
    std::cout << "UUID: " << user.user_id << std::endl;
    std::cout << "出生日期: " << user.birth_date << std::endl;
    std::cout << "创建时间: " << user.created_at << std::endl;
}

int main() {
    std::cout << "Schema DSL 示例" << std::endl;
    std::cout << "================" << std::endl;
    
    test_schema_features();
    test_functionality();
    
    return 0;
}
