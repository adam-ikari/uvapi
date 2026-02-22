/**
 * @file declarative_dsl_example.cpp
 * @brief 声明式 DSL 哲学说明
 * 
 * 解释什么是真正的声明式 DSL
 */

#include <iostream>
#include "../include/params_dsl.h"

using namespace uvapi;
using namespace restful;

int main() {
    std::cout << "=== 声明式 DSL 哲学 ===" << std::endl;
    std::cout << std::endl;
    
    // ========== 什么是声明式？==========
    std::cout << "========== 什么是声明式？ ==========" << std::endl;
    std::cout << std::endl;
    std::cout << "声明式的核心在于：描述「是什么」，而不是「怎么做」" << std::endl;
    std::cout << "关键在于：不要有动作（Action）" << std::endl;
    std::cout << std::endl;
    
    // ========== 命令式 vs 声明式 ==========
    std::cout << "========== 命令式 vs 声明式 ==========" << std::endl;
    std::cout << std::endl;
    
    std::cout << "【命令式 - 有动作】" << std::endl;
    std::cout << "param.setValue(1);      // 动作：设置值" << std::endl;
    std::cout << "param.setRange(1, 100); // 动作：设置范围" << std::endl;
    std::cout << "param.validate();       // 动作：执行验证" << std::endl;
    std::cout << std::endl;
    
    std::cout << "【声明式 - 只有描述】" << std::endl;
    std::cout << "queryParam<int>(\"page\")" << std::endl;
    std::cout << "    .optional()          // 描述：是可选的（不是动作）" << std::endl;
    std::cout << "    .defaultValue(1)     // 描述：默认值是1（不是动作）" << std::endl;
    std::cout << "    .range(1, 1000);     // 描述：范围是[1, 1000]（不是动作）" << std::endl;
    std::cout << std::endl;
    
    std::cout << "【声明式 - 初始化列表风格】" << std::endl;
    std::cout << "ParamGroup params = {" << std::endl;
    std::cout << "    range(Int(\"page\", false, 1), 1, 1000)," << std::endl;
    std::cout << "    range(Int(\"limit\", false, 10), 1, 100)" << std::endl;
    std::cout << "};" << std::endl;
    std::cout << std::endl;
    
    // ========== 链式调用也可以是声明式 ==========
    std::cout << "========== 链式调用也可以是声明式 ==========" << std::endl;
    std::cout << std::endl;
    std::cout << "误解：链式调用 = 命令式" << std::endl;
    std::cout << "正确：链式调用 ≠ 命令式" << std::endl;
    std::cout << std::endl;
    std::cout << "判断标准：是否包含动作？" << std::endl;
    std::cout << std::endl;
    
    std::cout << "【命令式链式调用】" << std::endl;
    std::cout << "builder.addParam(\"page\")    // 动作：添加参数" << std::endl;
    std::cout << "       .setOptional()        // 动作：设置为可选" << std::endl;
    std::cout << "       .setDefault(1);       // 动作：设置默认值" << std::endl;
    std::cout << std::endl;
    
    std::cout << "【声明式链式调用】" << std::endl;
    std::cout << "queryParam<int>(\"page\")" << std::endl;
    std::cout << "    .optional()          // 描述：是可选的" << std::endl;
    std::cout << "    .defaultValue(1);     // 描述：默认值是1" << std::endl;
    std::cout << std::endl;
    
    // ========== UVAPI 的 DSL 是声明式的 ==========
    std::cout << "========== UVAPI 的 DSL 是声明式的 ==========" << std::endl;
    std::cout << std::endl;
    
    std::cout << "【示例 1：用户列表 API】" << std::endl;
    std::cout << "// 声明参数（只有描述，没有动作）" << std::endl;
    std::cout << "auto page = queryParam<int>(\"page\")" << std::endl;
    std::cout << "    .optional()" << std::endl;
    std::cout << "    .defaultValue(1)" << std::endl;
    std::cout << "    .range(1, 1000);" << std::endl;
    std::cout << std::endl;
    std::cout << "// 这是在描述：page 是一个可选的整数参数，默认值1，范围[1, 1000]" << std::endl;
    std::cout << "// 不是在执行：添加参数、设置属性等动作" << std::endl;
    std::cout << std::endl;
    
    std::cout << "【示例 2：用户创建 API】" << std::endl;
    std::cout << "auto username = queryParam<std::string>(\"username\")" << std::endl;
    std::cout << "    .required()" << std::endl;
    std::cout << "    .length(3, 20);" << std::endl;
    std::cout << std::endl;
    std::cout << "// 这是在描述：username 是一个必需的字符串，长度[3, 20]" << std::endl;
    std::cout << std::endl;
    
    // ========== 声明式的优势 ==========
    std::cout << "========== 声明式的优势 ==========" << std::endl;
    std::cout << std::endl;
    std::cout << "1. 清晰性：一眼看出参数的属性" << std::endl;
    std::cout << "2. 可组合性：可以自由组合描述" << std::endl;
    std::cout << "3. 无副作用：描述不会产生副作用" << std::endl;
    std::cout << "4. 易于测试：描述可以独立测试" << std::endl;
    std::cout << "5. 符合直觉：「声明在前，处理在后」" << std::endl;
    std::cout << std::endl;
    
    // ========== 总结 ==========
    std::cout << "========== 总结 ==========" << std::endl;
    std::cout << std::endl;
    std::cout << "声明式不在于语法形式，而在于是否有动作" << std::endl;
    std::cout << std::endl;
    std::cout << "✓ 链式调用 + 无动作 = 声明式" << std::endl;
    std::cout << "✓ 初始化列表 + 无动作 = 声明式" << std::endl;
    std::cout << "✗ 链式调用 + 有动作 = 命令式" << std::endl;
    std::cout << "✗ 初始化列表 + 有动作 = 命令式" << std::endl;
    std::cout << std::endl;
    
    std::cout << "UVAPI 的模板 DSL（queryParam<int>().optional().defaultValue(1)）" << std::endl;
    std::cout << "是完全声明式的！" << std::endl;
    std::cout << std::endl;
    
    std::cout << "声明式 DSL 哲学说明完成！" << std::endl;
    
    return 0;
}
