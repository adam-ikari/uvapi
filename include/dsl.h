/**
 * @file dsl.h
 * @brief DSL 语法糖和便捷方法
 * 
 * 提供更简洁、直观的 API 来定义路由、参数和响应
 * 
 * ## 使用示例
 * 
 * @code
 * // 1. 响应构建
 * Response resp = ok(user.toJson());
 * Response err = notFound("User not found");
 * 
 * // 2. 路由组
 * RouteGroup api(app, "/api");
 * api.get("/users", getUsersHandler);
 * api.post("/users", createUserHandler);
 * 
 * // 3. 资源路由
 * ResourceRouter users(app, "/api/users");
 * users.index(listUsersHandler)
 *      .store(createUserHandler)
 *      .show(getUserHandler)
 *      .update(updateUserHandler)
 *      .destroy(deleteUserHandler);
 * 
 * // 4. Body 解析
 * auto result = parseBody<CreateUserRequest>(req);
 * if (result.success) {
 *     // 处理 result.instance
 * } else {
 *     return badRequest(result.error);
 * }
 * @endcode
 */

#ifndef DSL_H
#define DSL_H

#include "framework.h"
#include "params_dsl.h"
#include <functional>
#include <utility>

namespace uvapi {

// ========== 便捷类型别名 ==========

/// @brief HTTP 请求类型别名
using Request = HttpRequest;

/// @brief HTTP 响应类型别名
using Response = HttpResponse;

/// @brief 请求处理器类型别名
using Handler = restful::RequestHandler;

/// @brief 参数类型别名
using ParamType = restful::ParamType;

// ========== 响应构建器（简化版）==========

/// @brief 创建成功响应的 JSON
/// @param data JSON 数据字符串
/// @return 格式化的 JSON 字符串
inline std::string jsonSuccess(const std::string& data) {
    return "{\"success\":true,\"data\":" + data + "}";
}

/// @brief 创建成功响应的 JSON（带消息）
/// @param message 成功消息
/// @param data JSON 数据字符串
/// @return 格式化的 JSON 字符串
inline std::string jsonSuccessWithMessage(const std::string& message, const std::string& data) {
    return "{\"success\":true,\"message\":\"" + message + "\",\"data\":" + data + "}";
}

/// @brief 创建错误响应的 JSON
/// @param message 错误消息
/// @return 格式化的 JSON 字符串
inline std::string jsonError(const std::string& message) {
    return "{\"success\":false,\"error\":\"" + message + "\"}";
}

/// @brief 创建数据响应的 JSON
/// @param data JSON 数据字符串
/// @return 格式化的 JSON 字符串
inline std::string jsonData(const std::string& data) {
    return "{\"data\":" + data + "}";
}

// 成功响应（JSON）
inline Response ok(const std::string& data = "{}") {
    Response resp(200);
    resp.headers["Content-Type"] = "application/json";
    resp.body = jsonSuccess(data);
    return resp;
}

// 成功响应（带自定义数据）
template<typename T>
inline Response ok(const T& instance) {
    return Response(200)
        .header("Content-Type", "application/json")
        .body(jsonSuccess(uvapi::toJson(instance)));
}

// 成功响应（带消息）
inline Response ok(const std::string& message, const std::string& data) {
    Response resp(200);
    resp.headers["Content-Type"] = "application/json";
    resp.body = jsonSuccessWithMessage(message, data);
    return resp;
}

// 错误响应
inline Response error(int code, const std::string& message) {
    Response resp(code);
    resp.headers["Content-Type"] = "application/json";
    resp.body = jsonError(message);
    return resp;
}

// 错误响应（带状态码）
inline Response badRequest(const std::string& message = "Bad Request") {
    return error(400, message);
}

// 未授权响应
inline Response unauthorized(const std::string& message = "Unauthorized") {
    return error(401, message);
}

// 禁止访问响应
inline Response forbidden(const std::string& message = "Forbidden") {
    return error(403, message);
}

// 未找到响应
inline Response notFound(const std::string& message = "Not Found") {
    return error(404, message);
}

// 方法不允许响应
inline Response methodNotAllowed(const std::string& message = "Method Not Allowed") {
    return error(405, message);
}

// 服务器错误响应
inline Response serverError(const std::string& message = "Internal Server Error") {
    return error(500, message);
}



// ========== 中间件定义 ==========

// 中间件类型（使用函数类型）
using Middleware = std::function<Response(const Request&, std::function<Response()>)>;

// ========== 路由组（批量路由定义）==========

class RouteGroup {
private:
    restful::Api& api_;
    std::string prefix_;

public:
    RouteGroup(restful::Api& api, const std::string& prefix) 
        : api_(api), prefix_(prefix) {}
    
    // 添加 GET 路由
    RouteGroup& get(const std::string& path, Handler handler) {
        api_.get(prefix_ + path, handler);
        return *this;
    }
    
    // 添加 POST 路由
    RouteGroup& post(const std::string& path, Handler handler) {
        api_.post(prefix_ + path, handler);
        return *this;
    }
    
    // 添加 PUT 路由
    RouteGroup& put(const std::string& path, Handler handler) {
        api_.put(prefix_ + path, handler);
        return *this;
    }
    
    // 添加 DELETE 路由
    RouteGroup& del(const std::string& path, Handler handler) {
        api_.delete_(prefix_ + path, handler);
        return *this;
    }
    
    // 添加 PATCH 路由
    RouteGroup& patch(const std::string& path, Handler handler) {
        api_.patch(prefix_ + path, handler);
        return *this;
    }
    
    // 创建子路由组
    RouteGroup group(const std::string& prefix) {
        return RouteGroup(api_, prefix_ + prefix);
    }
};

// ========== 资源路由（RESTful CRUD）==========

class ResourceRouter {
private:
    restful::Api& api_;
    std::string path_;

public:
    ResourceRouter(restful::Api& api, const std::string& path) 
        : api_(api), path_(path) {}
    
    // 设置列表处理器
    ResourceRouter& index(Handler handler) {
        api_.get(path_, handler);
        return *this;
    }
    
    // 设置创建处理器
    ResourceRouter& store(Handler handler) {
        api_.post(path_, handler);
        return *this;
    }
    
    // 设置详情处理器
    ResourceRouter& show(Handler handler) {
        api_.get(path_ + "/:id", handler);
        return *this;
    }
    
    // 设置更新处理器
    ResourceRouter& update(Handler handler) {
        api_.put(path_ + "/:id", handler);
        return *this;
    }
    
    // 设置删除处理器
    ResourceRouter& destroy(Handler handler) {
        api_.delete_(path_ + "/:id", handler);
        return *this;
    }
    
    // 批量设置所有处理器
    ResourceRouter& all(Handler index, Handler store, Handler show, 
                        Handler update, Handler destroy) {
        return this->index(index)
                  .store(store)
                  .show(show)
                  .update(update)
                  .destroy(destroy);
    }
};

// ========== 中间件便捷函数 ==========

// CORS 中间件
inline Middleware cors(const std::string& origin = "*") {
    return [origin](const Request& req, std::function<Response()> next) -> Response {
        Response resp = next();
        resp.headers["Access-Control-Allow-Origin"] = origin;
        resp.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, PATCH, OPTIONS";
        resp.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
        return resp;
    };
}

// 认证中间件（基于 Bearer Token）
inline Middleware auth(restful::Api& api) {
    return [&api](const Request& req, std::function<Response()> next) -> Response {
        std::string auth_header;
        auto it = req.headers.find("Authorization");
        if (it != req.headers.end()) {
            auth_header = it->second;
        }

        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            return unauthorized("Missing or invalid authorization token");
        }

        // 提取 token
        std::string token = auth_header.substr(7);

        // 验证 token
        int64_t user_id = 0;
        std::string username;
        std::string role;
        if (!api.validateToken(token, user_id, username, role)) {
            return unauthorized("Invalid or expired token");
        }

        // 将用户信息存储在请求中（通过 path_params 临时传递）
        // 注意：这是一个简化的实现，实际应用中应该使用专门的上下文对象
        // const_cast<HttpRequest&>(req).user_id = user_id;

        return next();
    };
}

// 无参数的 auth() 保留用于向后兼容（不进行实际验证）
inline Middleware auth() {
    return [](const Request& req, std::function<Response()> next) -> Response {
        std::string auth_header;
        auto it = req.headers.find("Authorization");
        if (it != req.headers.end()) {
            auth_header = it->second;
        }

        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            return unauthorized("Missing or invalid authorization token");
        }

        // 注意：此版本不进行实际 token 验证，仅检查格式
        // 建议使用 auth(Api& api) 版本进行完整验证
        return next();
    };
}

// 日志中间件
inline Middleware logging() {
    return [](const Request& req, std::function<Response()> next) -> Response {
        std::cout << "[" << req.url_path << "] " 
                  << static_cast<int>(req.method) << std::endl;
        return next();
    };
}

// 错误处理中间件
inline Middleware catchError() {
    return [](const Request& req, std::function<Response()> next) -> Response {
        try {
            return next();
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return serverError(e.what());
        } catch (...) {
            std::cerr << "Unknown error" << std::endl;
            return serverError("Unknown error");
        }
    };
}

// ========== 参数验证便捷函数 ==========

// 验证参数（返回错误响应，如果验证失败）
inline Response validateParams(const Request& req, 
                                const std::vector<restful::ParamDefinition>& params) {
    std::string error = ParamValidator::validateAll(params, req.query_params);
    if (!error.empty()) {
        return badRequest(error);
    }
    
    error = ParamValidator::validateAll(params, req.path_params);
    if (!error.empty()) {
        return badRequest(error);
    }
    
    return Response(0);  // 0 表示验证通过（无效的状态码，仅用于标识）
}

// 检查验证结果
inline bool isValidationOk(const Response& resp) {
    return resp.status_code == 0;
}

// ========== Body 解析便捷函数 ==========

// 解析请求 Body 到对象（带验证）
template<typename T>
struct ParseResult {
    bool success;
    T instance;
    std::string error;
    
    ParseResult() : success(false), error("") {}
    ParseResult(const T& inst) : success(true), instance(inst), error("") {}
    ParseResult(const std::string& err) : success(false), error(err) {}
};

// 解析 Body
template<typename T>
inline ParseResult<T> parseBody(const Request& req) {
    try {
        T instance;
        BodySchemaBase* schema = instance.schema();
        if (!schema) {
            return ParseResult<T>("Schema not defined");
        }
        
        // 验证 JSON 格式
        cJSON* root = cJSON_Parse(req.body.c_str());
        if (!root) {
            return ParseResult<T>("Invalid JSON format");
        }
        
        // 验证 Schema
        std::string error = schema->validate(root);
        if (!error.empty()) {
            cJSON_Delete(root);
            return ParseResult<T>(error);
        }
        
        cJSON_Delete(root);
        
        // 反序列化
        if (!schema->fromJson(req.body, &instance)) {
            return ParseResult<T>("Failed to parse body");
        }
        
        // 验证对象
        error = schema->validateObject(&instance);
        if (!error.empty()) {
            return ParseResult<T>(error);
        }
        
        return ParseResult<T>(instance);
    } catch (const std::exception& e) {
        return ParseResult<T>(e.what());
    } catch (...) {
        return ParseResult<T>("Unknown error");
    }
}

// 验证 Body（返回 ParseResult）
template<typename T>
inline ParseResult<T> validateBody(const Request& req) {
    return parseBody<T>(req);
}

// ========== 路由便捷宏 ==========

// 定义资源路由
#define RESOURCE(path) ResourceRouter(app, path)
#define ROUTE_GROUP(prefix) RouteGroup(app, prefix)

// 定义简单的响应
#define SUCCESS(data) ok(data)
#define ERROR(message) error(500, message)
#define BAD_REQUEST(message) badRequest(message)
#define NOT_FOUND(message) notFound(message)
#define UNAUTHORIZED(message) unauthorized(message)
#define FORBIDDEN(message) forbidden(message)

// ========== DSL 构建器 ==========

// DSL 应用构建器（提供链式 API）
class App {
private:
    restful::Api& api_;
    
public:
    App(restful::Api& api) : api_(api) {}
    
    // API 信息
    App& title(const std::string& t) {
        api_.title(t);
        return *this;
    }
    
    App& description(const std::string& d) {
        api_.description(d);
        return *this;
    }
    
    App& version(const std::string& v) {
        api_.version(v);
        return *this;
    }
    
    // CORS
    App& cors(bool enabled = true) {
        if (enabled) {
            api_.enableCors();
        } else {
            api_.disableCors();
        }
        return *this;
    }
    
    // 路由
    template<typename Handler>
    App& get(const std::string& path, Handler handler) {
        api_.get(path, handler);
        return *this;
    }
    
    template<typename Handler>
    App& post(const std::string& path, Handler handler) {
        api_.post(path, handler);
        return *this;
    }
    
    template<typename Handler>
    App& put(const std::string& path, Handler handler) {
        api_.put(path, handler);
        return *this;
    }
    
    template<typename Handler>
    App& del(const std::string& path, Handler handler) {
        api_.delete_(path, handler);
        return *this;
    }
    
    template<typename Handler>
    App& patch(const std::string& path, Handler handler) {
        api_.patch(path, handler);
        return *this;
    }
    
    // 路由组
    RouteGroup group(const std::string& prefix) {
        return RouteGroup(api_, prefix);
    }
    
    // 资源路由
    ResourceRouter resource(const std::string& path) {
        return ResourceRouter(api_, path);
    }
    
    // 启动
    bool run(const std::string& host = "0.0.0.0", int port = 8080) {
        return api_.run(host, port);
    }
    
    // 停止
    void stop() {
        api_.stop();
    }
};

} // namespace uvapi

// ========== DSL API 完整文档 ==========

/**
 * @defgroup dsl DSL 语法糖和便捷方法
 * @brief 提供简洁、直观的 API 来定义路由、参数和响应
 * 
 * ## 响应构建器
 * 
 * @code
 * // 基本响应
 * Response resp(200);
 * resp.headers["Content-Type"] = "application/json";
 * 
 * // 便捷方法
 * ok()              // 200 OK
 * ok(data)          // 200 OK with data
 * ok(msg, data)     // 200 OK with message and data
 * error(code, msg)   // Custom error response
 * badRequest(msg)    // 400 Bad Request
 * unauthorized(msg)    // 401 Unauthorized
 * forbidden(msg)      // 403 Forbidden
 * notFound(msg)       //  #include "dsl.h"
 * 
 * int main() {
 *     uv_loop_t* loop = uv_default_loop();
 *     uvapi::App app(loop);
 *     
 *     // 设置 API 信息
 *     app.title("User API")
 *        .description("User management API")
 *        .version("1.0.0");
 *     
 *     // 启用 CORS
 *     app.cors();
 *     
 *     // 基本路由
 *     app.get("/api/ping", [](const uvapi::Request& req) -> uvapi::Response {
 *         return uvapi::ok("pong");
 *     });
 *     
 *     // 使用 Body 解析
 *     app.post("/api/users", [](const uvapi::Request& req) -> uvapi::Response {
 *         auto result = uvapi::parseBody<CreateUserRequest>(req);
 *         if (!result.success) {
 *             return uvapi::badRequest(result.error);
 *         }
 *         // 处理 result.instance
 *         return uvapi::ok("User created");
 *     });
 *     
 *     // 使用路由组
 *     auto api = app.group("/api");
 *     api.get("/users", listUsersHandler);
 *     api.post("/users", createUserHandler);
 *     
 *     // 使用资源路由（RESTful CRUD）
 *     app.resource("/api/users")
 *        .index(listUsersHandler)
 *        .store(createUserHandler)
 *        .show(getUserHandler)
 *        .update(updateUserHandler)
 *        .destroy(deleteUserHandler);
 *     
 *     // 启动服务器
 *     app.run("0.0.0.0", 8080);
 *     
 *     return 0;
 * }
 * @endcode
 * 
 * ## 参数验证
 * 
 * @code
 * // 使用 ParamAccessor
 * uvapi::ParamAccessor params(req);
 * int page = params.getQueryInt("page", 1);
 * std::string keyword = params.getQueryString("keyword", "");
 * 
 * // 使用 ParamValidator
 * std::vector<restful::ParamDefinition> param_defs;
 * param_defs.push_back(restful::ParamDefinition("page", restful::ParamType::QUERY));
 * param_defs.back().validation.required = false;
 * param_defs.back().validation.min_value = 1;
 * 
 * std::string error = restful::ParamValidator::validateAll(param_defs, req.query_params);
 * if (!error.empty()) {
 *     return badRequest(error);
 * }
 * @endcode
 * 
 * ## Schema 定义
 * 
 * @code
 * struct User {
 *     int64_t id;
 *     std::string username;
 *     std::string email;
 *     int age;
 *     bool active;
 *     
 *     uvapi::BodySchemaBase* schema() const;
 * };
 * 
 * class UserSchema : public uvapi::DslBodySchema<User> {
 * public:
 *     void define() override {
 *         // 使用成员指针，自动计算偏移量
 *         this->field(&User::id, "id").asInt64().required();
 *         this->field(&User::username, "username").asString().required()
 *             .minLength(3).maxLength(20);
 *         this->field(&User::email, "email").asString().required()
 *             .pattern("^[^@]+@[^@]+$");
 *         this->field(&User::age, "age").asInt().required()
 *             .range(18, 120);
 *         this->field(&User::active, "active").asBool().required();
 *     }
 * };
 * 
 * uvapi::BodySchemaBase* User::schema() const {
 *     static UserSchema instance;
 *     return &instance;
 * }
 * @endcode
 * 
 * ## 中间件
 * 
 * @code
 * // 使用预定义中间件
 * using namespace uvapi;
 * 
 * // CORS 中间件
 * Middleware cors_middleware = cors("*");
 * 
 * // 认证中间件
 * Middleware auth_middleware = auth();
 * 
 * // 日志中间件
 * Middleware logging_middleware = logging();
 * 
 * // 错误处理中间件
 * Middleware error_middleware = catchError();
 * 
 * // 自定义中间件
 * Middleware custom_middleware = [](const Request& req, std::function<Response()> next) -> Response {
 *     std::cout << "Custom middleware" << std::endl;
 *     return next();
 * };
 * @endcode
 */

#endif // DSL_H