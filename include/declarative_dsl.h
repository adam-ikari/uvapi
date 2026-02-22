/**
 * @file declarative_dsl.h
 * @brief 声明式参数 DSL
 * 
 * 采用真正的声明式风格，一次性声明所有参数信息
 */

#ifndef DECLARATIVE_DSL_H
#define DECLARATIVE_DSL_H

#include <string>
#include <vector>
#include <functional>
#include "framework.h"

namespace uvapi {
namespace declarative {

// ========== 参数类型 ==========

enum class ParamType {
    INT,
    INT64,
    DOUBLE,
    FLOAT,
    BOOL,
    STRING
};

// ========== 参数配置 ==========

template<typename T>
struct ParamConfig {
    std::string name;
    bool required;
    T default_value;
    bool has_default;
    
    // 整数范围验证
    int min_value;
    int max_value;
    bool has_range;
    
    // 字符串长度验证
    size_t min_length;
    size_t max_length;
    bool has_length;
    
    // 正则验证
    std::string pattern;
    bool has_pattern;
    
    // 枚举验证
    std::vector<std::string> enum_values;
    bool has_enum;
    
    ParamConfig() 
        : required(true)
        , has_default(false)
        , min_value(0)
        , max_value(0)
        , has_range(false)
        , min_length(0)
        , max_length(0)
        , has_length(false)
        , has_pattern(false)
        , has_enum(false) {}
};

// ========== 声明式参数定义 ==========

// 必需整数参数
inline ParamConfig<int> RequiredInt(const std::string& name) {
    ParamConfig<int> config;
    config.name = name;
    config.required = true;
    return config;
}

// 可选整数参数（带默认值）
inline ParamConfig<int> OptionalInt(const std::string& name, int default_value) {
    ParamConfig<int> config;
    config.name = name;
    config.required = false;
    config.default_value = default_value;
    config.has_default = true;
    return config;
}

// 必需字符串参数
inline ParamConfig<std::string> RequiredString(const std::string& name) {
    ParamConfig<std::string> config;
    config.name = name;
    config.required = true;
    return config;
}

// 可选字符串参数（带默认值）
inline ParamConfig<std::string> OptionalString(const std::string& name, const std::string& default_value = "") {
    ParamConfig<std::string> config;
    config.name = name;
    config.required = false;
    config.default_value = default_value;
    config.has_default = true;
    return config;
}

// 必需布尔参数
inline ParamConfig<bool> RequiredBool(const std::string& name) {
    ParamConfig<bool> config;
    config.name = name;
    config.required = true;
    return config;
}

// 可选布尔参数（带默认值）
inline ParamConfig<bool> OptionalBool(const std::string& name, bool default_value) {
    ParamConfig<bool> config;
    config.name = name;
    config.required = false;
    config.default_value = default_value;
    config.has_default = true;
    return config;
}

// 必需双精度浮点参数
inline ParamConfig<double> RequiredDouble(const std::string& name) {
    ParamConfig<double> config;
    config.name = name;
    config.required = true;
    return config;
}

// 可选双精度浮点参数（带默认值）
inline ParamConfig<double> OptionalDouble(const std::string& name, double default_value) {
    ParamConfig<double> config;
    config.name = name;
    config.required = false;
    config.default_value = default_value;
    config.has_default = true;
    return config;
}

// ========== 验证规则修饰符 ==========

// 范围验证（整数）
inline ParamConfig<int> Range(ParamConfig<int> config, int min_val, int max_val) {
    config.min_value = min_val;
    config.max_value = max_val;
    config.has_range = true;
    return config;
}

// 范围验证（双精度浮点）
inline ParamConfig<double> Range(ParamConfig<double> config, double min_val, double max_val) {
    config.min_value = static_cast<int>(min_val);
    config.max_value = static_cast<int>(max_val);
    config.has_range = true;
    return config;
}

// 长度验证（字符串）
inline ParamConfig<std::string> Length(ParamConfig<std::string> config, size_t min_len, size_t max_len) {
    config.min_length = min_len;
    config.max_length = max_len;
    config.has_length = true;
    return config;
}

// 正则验证（字符串）
inline ParamConfig<std::string> Pattern(ParamConfig<std::string> config, const std::string& regex) {
    config.pattern = regex;
    config.has_pattern = true;
    return config;
}

// 枚举验证（字符串）
inline ParamConfig<std::string> OneOf(ParamConfig<std::string> config, const std::vector<std::string>& values) {
    config.enum_values = values;
    config.has_enum = true;
    return config;
}

// ========== 参数组 ==========

class ParamGroup {
private:
    std::vector<restful::ParamDefinition> params_;
    
    // 转换 ParamConfig 到 ParamDefinition
    restful::ParamDefinition convert(const ParamConfig<int>& config) {
        restful::ParamDefinition def(config.name, restful::ParamType::QUERY);
        def.validation.required = config.required;
        def.default_value = config.has_default ? std::to_string(config.default_value) : "";
        def.data_type = 1;  // int
        if (config.has_range) {
            def.validation.min_value = config.min_value;
            def.validation.max_value = config.max_value;
            def.validation.has_min = true;
            def.validation.has_max = true;
        }
        return def;
    }
    
    restful::ParamDefinition convert(const ParamConfig<std::string>& config) {
        restful::ParamDefinition def(config.name, restful::ParamType::QUERY);
        def.validation.required = config.required;
        def.default_value = config.has_default ? config.default_value : "";
        def.data_type = 0;  // string
        if (config.has_length) {
            def.validation.min_length = config.min_length;
            def.validation.max_length = config.max_length;
            def.validation.has_min_length = true;
            def.validation.has_max_length = true;
        }
        if (config.has_pattern) {
            def.validation.pattern = config.pattern;
            def.validation.has_pattern = true;
        }
        if (config.has_enum) {
            def.validation.enum_values = config.enum_values;
            def.validation.has_enum = true;
        }
        return def;
    }
    
    restful::ParamDefinition convert(const ParamConfig<bool>& config) {
        restful::ParamDefinition def(config.name, restful::ParamType::QUERY);
        def.validation.required = config.required;
        def.default_value = config.has_default ? (config.default_value ? "true" : "false") : "";
        def.data_type = 5;  // bool
        return def;
    }
    
    restful::ParamDefinition convert(const ParamConfig<double>& config) {
        restful::ParamDefinition def(config.name, restful::ParamType::QUERY);
        def.validation.required = config.required;
        def.default_value = config.has_default ? std::to_string(config.default_value) : "";
        def.data_type = 3;  // double
        if (config.has_range) {
            def.validation.min_double = config.min_value;
            def.validation.max_double = config.max_value;
            def.validation.has_min = true;
            def.validation.has_max = true;
        }
        return def;
    }
    
public:
    ParamGroup() {}
    
    // 添加参数
    ParamGroup& add(const ParamConfig<int>& config) {
        params_.push_back(convert(config));
        return *this;
    }
    
    ParamGroup& add(const ParamConfig<std::string>& config) {
        params_.push_back(convert(config));
        return *this;
    }
    
    ParamGroup& add(const ParamConfig<bool>& config) {
        params_.push_back(convert(config));
        return *this;
    }
    
    ParamGroup& add(const ParamConfig<double>& config) {
        params_.push_back(convert(config));
        return *this;
    }
    
    // 获取所有参数定义
    const std::vector<restful::ParamDefinition>& getParams() const {
        return params_;
    }
};

// ========== 便捷函数 ==========

// 查询参数组
inline ParamGroup QueryParams(
    const ParamConfig<int>& p1,
    const ParamConfig<int>& p2 = ParamConfig<int>(),
    const ParamConfig<std::string>& p3 = ParamConfig<std::string>(),
    const ParamConfig<std::string>& p4 = ParamConfig<std::string>(),
    const ParamConfig<bool>& p5 = ParamConfig<bool>(),
    const ParamConfig<bool>& p6 = ParamConfig<bool>()
) {
    ParamGroup group;
    group.add(p1);
    if (p2.name != "") group.add(p2);
    if (p3.name != "") group.add(p3);
    if (p4.name != "") group.add(p4);
    if (p5.name != "") group.add(p5);
    if (p6.name != "") group.add(p6);
    return group;
}

} // namespace declarative
} // namespace uvapi

#endif // DECLARATIVE_DSL_H