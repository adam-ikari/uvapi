/**
 * @file array_support.h
 * @brief 数组类型序列化/反序列化支持
 * 
 * 为 UVAPI DSL 添加完整的数组类型支持
 */

#ifndef ARRAY_SUPPORT_H
#define ARRAY_SUPPORT_H

#include <vector>
#include <string>
#include <cJSON.h>
#include <stdexcept>

namespace uvapi {

// 数组元素类型枚举（用于运行时类型识别）
enum class ArrayElementType {
    STRING,
    INT,
    INT64,
    FLOAT,
    DOUBLE,
    BOOL
};

// 数组元素序列化/反序列化辅助函数
namespace array_utils {

// 从 JSON 数组元素转换为指定类型
template<typename T>
T fromJsonElement(const cJSON* json);

// 从指定类型转换为 JSON 数组元素
template<typename T>
cJSON* toJsonElement(const T& value);

// 字符串特化
template<>
inline std::string fromJsonElement<std::string>(const cJSON* json) {
    if (!cJSON_IsString(json)) {
        throw std::runtime_error("Expected string element");
    }
    return json->valuestring;
}

template<>
inline cJSON* toJsonElement<std::string>(const std::string& value) {
    return cJSON_CreateString(value.c_str());
}

// int 特化
template<>
inline int fromJsonElement<int>(const cJSON* json) {
    if (!cJSON_IsNumber(json)) {
        throw std::runtime_error("Expected number element");
    }
    return static_cast<int>(json->valuedouble);
}

template<>
inline cJSON* toJsonElement<int>(const int& value) {
    return cJSON_CreateNumber(value);
}

// int64_t 特化
template<>
inline int64_t fromJsonElement<int64_t>(const cJSON* json) {
    if (!cJSON_IsNumber(json)) {
        throw std::runtime_error("Expected number element");
    }
    return static_cast<int64_t>(json->valuedouble);
}

template<>
inline cJSON* toJsonElement<int64_t>(const int64_t& value) {
    return cJSON_CreateNumber(static_cast<double>(value));
}

// float 特化
template<>
inline float fromJsonElement<float>(const cJSON* json) {
    if (!cJSON_IsNumber(json)) {
        throw std::runtime_error("Expected number element");
    }
    return static_cast<float>(json->valuedouble);
}

template<>
inline cJSON* toJsonElement<float>(const float& value) {
    return cJSON_CreateNumber(value);
}

// double 特化
template<>
inline double fromJsonElement<double>(const cJSON* json) {
    if (!cJSON_IsNumber(json)) {
        throw std::runtime_error("Expected number element");
    }
    return json->valuedouble;
}

template<>
inline cJSON* toJsonElement<double>(const double& value) {
    return cJSON_CreateNumber(value);
}

// bool 特化
template<>
inline bool fromJsonElement<bool>(const cJSON* json) {
    if (!cJSON_IsBool(json)) {
        throw std::runtime_error("Expected boolean element");
    }
    return cJSON_IsTrue(json);
}

template<>
inline cJSON* toJsonElement<bool>(const bool& value) {
    return cJSON_CreateBool(value);
}

// 通用数组序列化函数
template<typename T>
cJSON* serializeArray(const std::vector<T>& vec) {
    cJSON* array = cJSON_CreateArray();
    if (!array) {
        return nullptr;
    }
    
    for (const auto& item : vec) {
        cJSON* element = toJsonElement(item);
        if (!element) {
            cJSON_Delete(array);
            return nullptr;
        }
        cJSON_AddItemToArray(array, element);
    }
    
    return array;
}

// 通用数组反序列化函数
template<typename T>
bool deserializeArray(const cJSON* json, std::vector<T>& vec) {
    if (!cJSON_IsArray(json)) {
        return false;
    }
    
    vec.clear();
    cJSON* element = json->child;
    while (element) {
        try {
            T value = fromJsonElement<T>(element);
            vec.push_back(value);
        } catch (const std::exception&) {
            return false;
        }
        element = element->next;
    }
    
    return true;
}

// std::optional<std::vector<T>> 序列化
template<typename T>
cJSON* serializeOptionalArray(const uvapi::optional<std::vector<T>>& opt_vec) {
    if (!opt_vec.has_value()) {
        return cJSON_CreateNull();
    }
    return serializeArray(opt_vec.value());
}

// std::optional<std::vector<T>> 反序列化
template<typename T>
bool deserializeOptionalArray(const cJSON* json, uvapi::optional<std::vector<T>>& opt_vec) {
    if (cJSON_IsNull(json)) {
        opt_vec.reset();
        return true;
    }
    
    std::vector<T> vec;
    if (!deserializeArray(json, vec)) {
        return false;
    }
    
    opt_vec = std::move(vec);
    return true;
}

} // namespace array_utils

} // namespace uvapi

#endif // ARRAY_SUPPORT_H