/**
 * @file declarative_dsl.h
 * @brief 真正的声明式 DSL
 * 
 * 采用类似配置文件的声明式风格，使用初始化列表一次性声明所有参数信息
 */

#ifndef DECLARATIVE_DSL_H
#define DECLARATIVE_DSL_H

#include <string>
#include <vector>
#include <initializer_list>
#include "framework.h"

namespace uvapi {
namespace declarative {

// ========== 参数定义（真正的声明式）==========

struct ParamDef {
    std::string name;
    std::string type;  // "int", "string", "bool", "double"
    bool required;
    std::string default_value;
    
    // 验证规则
    int min_value;
    int max_value;
    bool has_range;
    
    size_t min_length;
    size_t max_length;
    bool has_length;
    
    std::string pattern;
    bool has_pattern;
    
    std::vector<std::string> enum_values;
    bool has_enum;
    
    ParamDef() 
        : required(false)
        , min_value(0)
        , max_value(0)
        , has_range(false)
        , min_length(0)
        , max_length(0)
        , has_length(false)
        , has_pattern(false)
        , has_enum(false) {}
};

// ========== 参数组（使用初始化列表）==========

class ParamGroup {
private:
    std::vector<restful::ParamDefinition> params_;
    
    restful::ParamDefinition convert(const ParamDef& def) {
        restful::ParamDefinition param_def(def.name, restful::ParamType::QUERY);
        param_def.validation.required = def.required;
        param_def.default_value = def.default_value;
        
        // 设置类型
        if (def.type == "int") {
            param_def.data_type = 1;
        } else if (def.type == "string") {
            param_def.data_type = 0;
        } else if (def.type == "bool") {
            param_def.data_type = 5;
        } else if (def.type == "double") {
            param_def.data_type = 3;
        }
        
        // 设置验证规则
        if (def.has_range) {
            param_def.validation.min_value = def.min_value;
            param_def.validation.max_value = def.max_value;
            param_def.validation.has_min = true;
            param_def.validation.has_max = true;
        }
        
        if (def.has_length) {
            param_def.validation.min_length = def.min_length;
            param_def.validation.max_length = def.max_length;
            param_def.validation.has_min_length = true;
            param_def.validation.has_max_length = true;
        }
        
        if (def.has_pattern) {
            param_def.validation.pattern = def.pattern;
            param_def.validation.has_pattern = true;
        }
        
        if (def.has_enum) {
            param_def.validation.enum_values = def.enum_values;
            param_def.validation.has_enum = true;
        }
        
        return param_def;
    }
    
public:
    ParamGroup() {}
    
    // 使用初始化列表声明参数（真正的声明式）
    ParamGroup(std::initializer_list<ParamDef> defs) {
        for (const auto& def : defs) {
            params_.push_back(convert(def));
        }
    }
    
    const std::vector<restful::ParamDefinition>& getParams() const {
        return params_;
    }
};

// ========== 声明式参数定义辅助函数 ==========

inline ParamDef Int(std::string name, bool required = false, int default_value = 0) {
    ParamDef def;
    def.name = name;
    def.type = "int";
    def.required = required;
    def.default_value = std::to_string(default_value);
    return def;
}

inline ParamDef String(std::string name, bool required = false, std::string default_value = "") {
    ParamDef def;
    def.name = name;
    def.type = "string";
    def.required = required;
    def.default_value = default_value;
    return def;
}

inline ParamDef Bool(std::string name, bool required = false, bool default_value = false) {
    ParamDef def;
    def.name = name;
    def.type = "bool";
    def.required = required;
    def.default_value = default_value ? "true" : "false";
    return def;
}

inline ParamDef Double(std::string name, bool required = false, double default_value = 0.0) {
    ParamDef def;
    def.name = name;
    def.type = "double";
    def.required = required;
    def.default_value = std::to_string(default_value);
    return def;
}

// ========== 验证规则设置函数 ==========

inline ParamDef range(ParamDef def, int min_val, int max_val) {
    def.min_value = min_val;
    def.max_value = max_val;
    def.has_range = true;
    return def;
}

inline ParamDef length(ParamDef def, size_t min_len, size_t max_len) {
    def.min_length = min_len;
    def.max_length = max_len;
    def.has_length = true;
    return def;
}

inline ParamDef pattern(ParamDef def, std::string regex) {
    def.pattern = regex;
    def.has_pattern = true;
    return def;
}

inline ParamDef oneOf(ParamDef def, std::initializer_list<std::string> values) {
    def.enum_values = values;
    def.has_enum = true;
    return def;
}

} // namespace declarative
} // namespace uvapi

#endif // DECLARATIVE_DSL_H