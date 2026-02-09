/**
 * @file framework_http.h
 * @brief RESTful 低代码框架 HTTP 相关定义
 */

#ifndef FRAMEWORK_HTTP_H
#define FRAMEWORK_HTTP_H

#include <string>
#include <map>
#include <functional>
#include <variant>
#include <type_traits>
#include <vector>
#include <cJSON.h>
#include "framework_types.h"

namespace uvapi {

// HTTP 方法枚举（与 uvhttp_method_t 对应）
enum class HttpMethod {
    ANY = 0,
    GET = 1,
    POST = 2,
    PUT = 3,
    DELETE = 4,
    HEAD = 5,
    OPTIONS = 6,
    PATCH = 7
};

// 安全的 HTTP 方法转换函数
inline uvhttp_method_t toUvhttpMethod(HttpMethod method) {
    return static_cast<uvhttp_method_t>(static_cast<int>(method));
}

// ========== 响应数据构建器（高级 API，不直接操作 JSON）==========

class DataBuilder {
private:
    cJSON* root_;
    
public:
    DataBuilder() : root_(cJSON_CreateObject()) {}
    
    ~DataBuilder() {
        if (root_) {
            cJSON_Delete(root_);
        }
    }
    
    // 禁止拷贝
    DataBuilder(const DataBuilder&) = delete;
    DataBuilder& operator=(const DataBuilder&) = delete;
    
    // 基础类型添加
    DataBuilder& set(const std::string& key, const std::string& value) {
        if (root_) cJSON_AddStringToObject(root_, key.c_str(), value.c_str());
        return *this;
    }
    
    DataBuilder& set(const std::string& key, int value) {
        if (root_) cJSON_AddNumberToObject(root_, key.c_str(), value);
        return *this;
    }
    
    DataBuilder& set(const std::string& key, int64_t value) {
        if (root_) cJSON_AddNumberToObject(root_, key.c_str(), static_cast<double>(value));
        return *this;
    }
    
    DataBuilder& set(const std::string& key, double value) {
        if (root_) cJSON_AddNumberToObject(root_, key.c_str(), value);
        return *this;
    }
    
    DataBuilder& set(const std::string& key, bool value) {
        if (root_) cJSON_AddBoolToObject(root_, key.c_str(), value);
        return *this;
    }
    
    // 对象类型（从 DataBuilder 构建嵌套对象）
    DataBuilder& set(const std::string& key, const DataBuilder& builder) {
        if (root_ && builder.root_) {
            cJSON_AddItemToObject(root_, key.c_str(), cJSON_Duplicate(builder.root_, true));
        }
        return *this;
    }
    
    // 数组类型
    DataBuilder& add(const std::string& array_key, const std::string& value) {
        if (!root_) return *this;
        cJSON* array = ensureArray(array_key);
        if (array) cJSON_AddItemToArray(array, cJSON_CreateString(value.c_str()));
        return *this;
    }
    
    DataBuilder& add(const std::string& array_key, int value) {
        if (!root_) return *this;
        cJSON* array = ensureArray(array_key);
        if (array) cJSON_AddItemToArray(array, cJSON_CreateNumber(value));
        return *this;
    }
    
    DataBuilder& add(const std::string& array_key, double value) {
        if (!root_) return *this;
        cJSON* array = ensureArray(array_key);
        if (array) cJSON_AddItemToArray(array, cJSON_CreateNumber(value));
        return *this;
    }
    
    DataBuilder& add(const std::string& array_key, bool value) {
        if (!root_) return *this;
        cJSON* array = ensureArray(array_key);
        if (array) cJSON_AddItemToArray(array, cJSON_CreateBool(value));
        return *this;
    }
    
    DataBuilder& add(const std::string& array_key, const DataBuilder& builder) {
        if (!root_) return *this;
        cJSON* array = ensureArray(array_key);
        if (array && builder.root_) {
            cJSON_AddItemToArray(array, cJSON_Duplicate(builder.root_, true));
        }
        return *this;
    }
    
    // 批量添加对象到数组
    template<typename T>
    DataBuilder& addArray(const std::string& array_key, const std::vector<T>& items, 
                         std::function<void(DataBuilder&, const T&)> item_builder) {
        if (!root_) return *this;
        cJSON* array = ensureArray(array_key);
        if (!array) return *this;
        
        for (const auto& item : items) {
            DataBuilder item_builder_obj;
            item_builder(item_builder_obj, item);
            cJSON_AddItemToArray(array, cJSON_Duplicate(item_builder_obj.root_, true));
        }
        return *this;
    }
    
    // 获取 JSON 字符串
    std::string toJson() const {
        if (!root_) return "{}";
        char* json = cJSON_Print(root_);
        std::string result(json ? json : "{}");
        if (json) free(json);
        return result;
    }
    
    // 获取 cJSON 对象（仅供内部使用）
    cJSON* getRoot() const { return root_; }
    
private:
    cJSON* ensureArray(const std::string& key) {
        if (!root_) return nullptr;
        cJSON* array = cJSON_GetObjectItem(root_, key.c_str());
        if (!array) {
            array = cJSON_CreateArray();
            cJSON_AddItemToObject(root_, key.c_str(), array);
        }
        return cJSON_IsArray(array) ? array : nullptr;
    }
};

// ========== ResponseBuilder（高级 API，不直接操作 JSON）==========

class ResponseBuilder {
private:
    DataBuilder* data_builder_;
    int status_code_;
    std::string message_;
    std::map<std::string, std::string> headers_;
    bool has_error_;
    int error_code_;
    
public:
    ResponseBuilder(int status = 200) 
        : data_builder_(new DataBuilder()), status_code_(status), 
          has_error_(false), error_code_(0) {}
    
    ~ResponseBuilder() {
        delete data_builder_;
    }
    
    // 状态码
    ResponseBuilder& status(int code) {
        status_code_ = code;
        return *this;
    }
    
    // 响应头
    ResponseBuilder& header(const std::string& key, const std::string& value) {
        headers_[key] = value;
        return *this;
    }
    
    // 成功响应
    ResponseBuilder& success(const std::string& msg = "Success") {
        message_ = msg;
        has_error_ = false;
        return *this;
    }
    
    // 错误响应
    ResponseBuilder& error(const std::string& msg) {
        message_ = msg;
        has_error_ = true;
        error_code_ = -1;
        status_code_ = status_code_ == 200 ? 500 : status_code_;
        return *this;
    }
    
    ResponseBuilder& error(int code, const std::string& msg) {
        message_ = msg;
        has_error_ = true;
        error_code_ = code;
        status_code_ = code >= 400 ? code : 500;
        return *this;
    }
    
    // 数据操作（通过 DataBuilder）
    ResponseBuilder& set(const std::string& key, const std::string& value) {
        data_builder_->set(key, value);
        return *this;
    }
    
    ResponseBuilder& set(const std::string& key, int value) {
        data_builder_->set(key, value);
        return *this;
    }
    
    ResponseBuilder& set(const std::string& key, int64_t value) {
        data_builder_->set(key, value);
        return *this;
    }
    
    ResponseBuilder& set(const std::string& key, double value) {
        data_builder_->set(key, value);
        return *this;
    }
    
    ResponseBuilder& set(const std::string& key, bool value) {
        data_builder_->set(key, value);
        return *this;
    }
    
    ResponseBuilder& set(const std::string& key, const DataBuilder& builder) {
        data_builder_->set(key, builder);
        return *this;
    }
    
    // 数组操作
    ResponseBuilder& add(const std::string& array_key, const std::string& value) {
        data_builder_->add(array_key, value);
        return *this;
    }
    
    ResponseBuilder& add(const std::string& array_key, int value) {
        data_builder_->add(array_key, value);
        return *this;
    }
    
    ResponseBuilder& add(const std::string& array_key, double value) {
        data_builder_->add(array_key, value);
        return *this;
    }
    
    ResponseBuilder& add(const std::string& array_key, bool value) {
        data_builder_->add(array_key, value);
        return *this;
    }
    
    ResponseBuilder& add(const std::string& array_key, const DataBuilder& builder) {
        data_builder_->add(array_key, builder);
        return *this;
    }
    
    // 批量添加到数组
    template<typename T>
    ResponseBuilder& addArray(const std::string& array_key, const std::vector<T>& items,
                             std::function<void(DataBuilder&, const T&)> item_builder) {
        data_builder_->addArray(array_key, items, item_builder);
        return *this;
    }
    
    // 使用统一 Schema 序列化
    template<typename T>
    ResponseBuilder& schema(void* instance) {
        SchemaBase* schema = static_cast<T*>(instance)->schema();
        if (schema) {
            std::string json = schema->toJson(instance);
            cJSON* data_obj = cJSON_Parse(json.c_str());
            if (data_obj) {
                cJSON* root = data_builder_->getRoot();
                if (root) {
                    cJSON_AddItemToObject(root, "data", data_obj);
                } else {
                    cJSON_Delete(data_obj);
                }
            }
        }
        return *this;
    }
    
    ResponseBuilder& schema(SchemaBase* schema, void* instance) {
        if (schema) {
            std::string json = schema->toJson(instance);
            cJSON* data_obj = cJSON_Parse(json.c_str());
            if (data_obj) {
                cJSON* root = data_builder_->getRoot();
                if (root) {
                    cJSON_AddItemToObject(root, "data", data_obj);
                } else {
                    cJSON_Delete(data_obj);
                }
            }
        }
        return *this;
    }
    
    // 直接设置 data 字段（从 DataBuilder）
    ResponseBuilder& data(const DataBuilder& builder) {
        cJSON* root = data_builder_->getRoot();
        if (root && builder.getRoot()) {
            cJSON_AddItemToObject(root, "data", cJSON_Duplicate(builder.getRoot(), true));
        }
        return *this;
    }
    
    // 直接设置 data 字段（从 JSON 字符串）
    ResponseBuilder& data(const std::string& json_str) {
        cJSON* root = data_builder_->getRoot();
        if (root) {
            cJSON* data_obj = cJSON_Parse(json_str.c_str());
            if (data_obj) {
                cJSON_AddItemToObject(root, "data", data_obj);
            } else {
                cJSON_AddStringToObject(root, "data", json_str.c_str());
            }
        }
        return *this;
    }
    
    // 构建 HttpResponse
    HttpResponse build() const {
        HttpResponse response(status_code_);
        
        // 设置响应头
        for (const auto& [key, value] : headers_) {
            response.header(key, value);
        }
        
        // 默认设置 Content-Type
        if (response.headers.find("Content-Type") == response.headers.end()) {
            response.header("Content-Type", "application/json");
        }
        
        // 构建响应 JSON
        cJSON* root = cJSON_CreateObject();
        if (root) {
            // 设置 code
            if (has_error_) {
                cJSON_AddNumberToObject(root, "code", error_code_);
            } else {
                cJSON_AddStringToObject(root, "code", "0");
            }
            
            // 设置 message
            if (!message_.empty()) {
                cJSON_AddStringToObject(root, "message", message_.c_str());
            }
            
            // 添加 data 字段
            if (data_builder_ && data_builder_->getRoot()) {
                cJSON* data = cJSON_GetObjectItem(data_builder_->getRoot(), "data");
                if (data) {
                    cJSON_AddItemToObject(root, "data", cJSON_Duplicate(data, true));
                } else {
                    // 如果没有显式设置 data，但 data_builder 中有其他字段
                    cJSON* data_obj = cJSON_Duplicate(data_builder_->getRoot(), true);
                    if (data_obj) {
                        cJSON_AddItemToObject(root, "data", data_obj);
                    }
                }
            }
            
            // 序列化
            char* json = cJSON_Print(root);
            if (json) {
                response.body = json;
                free(json);
            }
            cJSON_Delete(root);
        }
        
        return response;
    }
    
    // 隐式转换
    operator HttpResponse() const {
        return build();
    }
};

// 响应辅助函数
inline HttpResponse success(const std::string& message = "Success") {
    return ResponseBuilder(200).success(message).build();
}

inline HttpResponse error(const std::string& message) {
    return ResponseBuilder(500).error(message).build();
}

inline HttpResponse error(int code, const std::string& message) {
    return ResponseBuilder(code).error(code, message).build();
}

// HTTP 响应
struct HttpResponse {
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;
    
    HttpResponse(int code = 200) : status_code(code) {}
    
    HttpResponse& header(const std::string& key, const std::string& value) {
        headers[key] = value;
        return *this;
    }
    
    HttpResponse& json(const std::string& json_body) {
        body = json_body;
        headers["Content-Type"] = "application/json";
        return *this;
    }
    
    template<typename T>
    HttpResponse& json(const T& instance) {
        body = uvapi::toJson(instance);
        headers["Content-Type"] = "application/json";
        return *this;
    }
};

// 前向声明
template<typename T>
T parseBody(const std::string& json);

// HTTP 请求
struct HttpRequest {
    HttpMethod method;
    std::string url_path;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> query_params;
    std::map<std::string, std::string> path_params;
    std::string body;
    int64_t user_id;
    
    auto pathAuto(const std::string& key) const -> std::variant<std::string, int64_t>;
    
    template<typename T>
    T path(const std::string& key) const {
        auto it = path_params.find(key);
        if (it == path_params.end()) return T();
        
        if constexpr (std::is_same_v<T, std::string>) {
            return it->second;
        } else if constexpr (std::is_integral_v<T>) {
            return static_cast<T>(std::stoll(it->second));
        } else if constexpr (std::is_floating_point_v<T>) {
            return static_cast<T>(std::stod(it->second));
        } else {
            return T(it->second);
        }
    }
    
    template<typename T>
    T path(const std::string& key, const T& default_value) const {
        auto it = path_params.find(key);
        if (it != path_params.end()) {
            if constexpr (std::is_same_v<T, std::string>) {
                return it->second;
            } else if constexpr (std::is_integral_v<T>) {
                return static_cast<T>(std::stoll(it->second));
            } else if constexpr (std::is_floating_point_v<T>) {
                return static_cast<T>(std::stod(it->second));
            } else {
                return T(it->second);
            }
        }
        return default_value;
    }
    
    template<typename T>
    T query(const std::string& key) const;
    
    template<typename T>
    T query(const std::string& key, const T& default_value) const;
    
    template<typename T>
    T parseBody() const {
        return uvapi::parseBody<T>(body);
    }
    
    std::string getHeader(const std::string& key) const;
    bool isAuthenticated() const;
};

} // namespace uvapi

#endif // FRAMEWORK_HTTP_H