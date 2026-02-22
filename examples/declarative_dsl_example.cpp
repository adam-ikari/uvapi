/**
 * @file declarative_dsl_example.cpp
 * @brief 真正的声明式 DSL 使用示例
 * 
 * 展示如何使用类似配置文件的声明式风格定义参数
 */

#include <iostream>
#include "../include/declarative_dsl.h"

using namespace uvapi;
using namespace uvapi::declarative;

int main() {
    std::cout << "=== 真正的声明式 DSL 示例 ===" << std::endl;
    std::cout << std::endl;
    
    // ========== 示例 1: 用户列表 API 参数 ==========
    std::cout << "示例 1: 用户列表 API 参数" << std::endl;
    std::cout << std::endl;
    
    // 【真正的声明式 DSL - 类似配置文件】
    // 使用初始化列表一次性声明所有参数
    ParamGroup userListParams = {
        range(Int("page", false, 1), 1, 1000),
        range(Int("limit", false, 10), 1, 100),
        oneOf(String("status", false, "active"), {"active", "inactive", "pending"}),
        String("search", false, "")
    };
    
    std::cout << "参数声明（类似配置文件）：" << std::endl;
    std::cout << "ParamGroup userListParams = {" << std::endl;
    std::cout << "    range(Int(\"page\", false, 1), 1, 1000)," << std::endl;
    std::cout << "    range(Int(\"limit\", false, 10), 1, 100)," << std::endl;
    std::cout << "    oneOf(String(\"status\", false, \"active\"), {\"active\", \"inactive\", \"pending\"})," << std::endl;
    std::cout << "    String(\"search\", false, \"\")" << std::endl;
    std::cout << "};" << std::endl;
    std::cout << std::endl;
    
    // ========== 示例 2: 用户创建 API 参数 ==========
    std::cout << "示例 2: 用户创建 API 参数" << std::endl;
    std::cout << std::endl;
    
    // 【真正的声明式 DSL】
    ParamGroup createUserParams = {
        length(String("username", true), 3, 20),
        pattern(String("email", true), "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"),
        range(Int("age", false, 18), 18, 120),
        Bool("active", false, true)
    };
    
    std::cout << "参数声明（类似配置文件）：" << std::endl;
    std::cout << "ParamGroup createUserParams = {" << std::endl;
    std::cout << "    length(String(\"username\", true), 3, 20)," << std::endl;
    std::cout << "    pattern(String(\"email\", true), \"^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$\")," << std::endl;
    std::cout << "    range(Int(\"age\", false, 18), 18, 120)," << std::endl;
    std::cout << "    Bool(\"active\", false, true)" << std::endl;
    std::cout << "};" << std::endl;
    std::cout << std::endl;
    
    // ========== 示例 3: 产品列表 API 参数 ==========
    std::cout << "示例 3: 产品列表 API 参数" << std::endl;
    std::cout << std::endl;
    
    // 【真正的声明式 DSL】
    ParamGroup productListParams = {
        range(Int("page", false, 1), 1, 1000),
        range(Int("limit", false, 20), 1, 100),
        oneOf(String("sort", false, "id"), {"id", "name", "price", "created_at"}),
        oneOf(String("order", false, "asc"), {"asc", "desc"}),
        range(Int("min_price", false, 0), 0, 1000000),
        range(Int("max_price", false, 1000000), 0, 1000000),
        oneOf(String("category", false, ""), {"electronics", "clothing", "books", "food", "other"})
    };
    
    std::cout << "参数声明（类似配置文件）：" << std::endl;
    std::cout << "ParamGroup productListParams = {" << std::endl;
    std::cout << "    range(Int(\"page\", false, 1), 1, 1000)," << std::endl;
    std::cout << "    range(Int(\"limit\", false, 20), 1, 100)," << std::endl;
    std::cout << "    oneOf(String(\"sort\", false, \"id\"), {\"id\", \"name\", \"price\", \"created_at\"})," << std::endl;
    std::cout << "    oneOf(String(\"order\", false, \"asc\"), {\"asc\", \"desc\"})," << std::endl;
    std::cout << "    range(Int(\"min_price\", false, 0), 0, 1000000)," << std::endl;
    std::cout << "    range(Int(\"max_price\", false, 1000000), 0, 1000000)," << std::endl;
    std::cout << "    oneOf(String(\"category\", false, \"\"), {\"electronics\", \"clothing\", \"books\", \"food\", \"other\"})" << std::endl;
    std::cout << "};" << std::endl;
    std::cout << std::endl;
    
    // ========== 对比：命令式 vs 声明式 ==========
    std::cout << "========== 命令式 vs 声明式对比 ==========" << std::endl;
    std::cout << std::endl;
    
    std::cout << "【命令式 DSL（链式调用）】" << std::endl;
    std::cout << "queryParam<int>(\"page\")" << std::endl;
    std::cout << "    .optional()" << std::endl;
    std::cout << "    .defaultValue(1)" << std::endl;
    std::cout << "    .range(1, 1000);" << std::endl;
    std::cout << std::endl;
    
    std::cout << "【声明式 DSL（类似配置文件）】" << std::endl;
    std::cout << "range(Int(\"page\", false, 1), 1, 1000)" << std::endl;
    std::cout << std::endl;
    
    std::cout << "【完整声明式 DSL（初始化列表）】" << std::endl;
    std::cout << "ParamGroup params = {" << std::endl;
    std::cout << "    range(Int(\"page\", false, 1), 1, 1000)," << std::endl;
    std::cout << "    range(Int(\"limit\", false, 10), 1, 100)" << std::endl;
    std::cout << "};" << std::endl;
    std::cout << std::endl;
    
    // ========== 声明式 DSL 的特点 ==========
    std::cout << "========== 声明式 DSL 的特点 ==========" << std::endl;
    std::cout << std::endl;
    std::cout << "1. 类似配置文件：使用初始化列表，一次性声明所有参数" << std::endl;
    std::cout << "2. 函数式组合：使用 range(), length(), pattern() 等函数组合参数定义" << std::endl;
    std::cout << "3. 不可变性：参数定义后不可修改，符合声明式原则" << std::endl;
    std::cout << "4. 易于阅读：参数声明清晰，一目了然" << std::endl;
    std::cout << "5. 易于维护：添加/删除参数只需修改初始化列表" << std::endl;
    std::cout << std::endl;
    
    // ========== 参数定义函数说明 ==========
    std::cout << "========== 参数定义函数 ==========" << std::endl;
    std::cout << std::endl;
    std::cout << "Int(name, required, default_value)" << std::endl;
    std::cout << "  - name: 参数名" << std::endl;
    std::cout << "  - required: 是否必需" << std::endl;
    std::cout << "  - default_value: 默认值" << std::endl;
    std::cout << std::endl;
    std::cout << "String(name, required, default_value)" << std::endl;
    std::cout << "Bool(name, required, default_value)" << std::endl;
    std::cout << "Double(name, required, default_value)" << std::endl;
    std::cout << std::endl;
    
    // ========== 验证规则函数说明 ==========
    std::cout << "========== 验证规则函数 ==========" << std::endl;
    std::cout << std::endl;
    std::cout << "range(ParamDef, min, max)" << std::endl;
    std::cout << "  - 整数范围验证" << std::endl;
    std::cout << std::endl;
    std::cout << "length(ParamDef, min, max)" << std::endl;
    std::cout << "  - 字符串长度验证" << std::endl;
    std::cout << std::endl;
    std::cout << "pattern(ParamDef, regex)" << std::endl;
    std::cout << "  - 正则表达式验证" << std::endl;
    std::cout << std::endl;
    std::cout << "oneOf(ParamDef, {value1, value2, ...})" << std::endl;
    std::cout << "  - 枚举值验证" << std::endl;
    std::cout << std::endl;
    
    std::cout << "声明式 DSL 示例完成！" << std::endl;
    
    return 0;
}