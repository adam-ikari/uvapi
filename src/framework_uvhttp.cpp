/**
 * @file framework_uvhttp.cpp
 * @brief RESTful 低代码框架实现（基于 uvhttp）
 * 
 * 采用 server 和 api 两层架构，事件循环注入模式
 */

#include "framework.h"
#include "uvhttp_connection.h"
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <random>

namespace uvapi {

// ========== HttpRequest 模板特化实现 ==========
// 注意：模板方法已经在头文件中内联实现，不需要在 .cpp 文件中特化

// ========== JSON 辅助函数（使用新的序列化 API）==========

std::string jsonSuccess(const std::string& message) {
    return restful::JSON::success(message);
}

std::string jsonError(const std::string& message) {
    return restful::JSON::error(message);
}

std::string jsonData(const std::string& data) {
    return restful::JSON::data(data);
}

// ========== Server 层实现 ==========

// ========== Server 层实现 ==========

namespace server {

// uvhttp 请求回调
int on_uvhttp_request(uvhttp_request_t* req, uvhttp_response_t* resp) {
    if (!req || !resp) {
        if (resp) {
            uvhttp_response_set_status(resp, 500);
            uvhttp_response_send(resp);
        }
        return -1;
    }
    
    // 获取 server 实例
    // 通过 req->client->data 获取 connection，然后通过 connection->server 获取服务器
    uvhttp_connection_t* conn = (uvhttp_connection_t*)req->client->data;
    if (!conn || !conn->server) {
        uvhttp_response_set_status(resp, 500);
        uvhttp_response_send(resp);
        return -1;
    }
    uvhttp_server_t* http_server = conn->server;
    if (!http_server || !http_server->user_data) {
        uvhttp_response_set_status(resp, 500);
        uvhttp_response_send(resp);
        return -1;
    }
    
    server::Server* svr_instance = (server::Server*)http_server->user_data;
    
    // 构造 HttpRequest
    HttpRequest uvapi_req;
    
    // 获取方法
    const char* method_str = uvhttp_request_get_method(req);
    uvhttp_method_t uvhttp_method = uvhttp_method_from_string(method_str);
    if (uvhttp_method == UVHTTP_GET) {
        uvapi_req.method = HttpMethod::GET;
    } else if (uvhttp_method == UVHTTP_POST) {
        uvapi_req.method = HttpMethod::POST;
    } else if (uvhttp_method == UVHTTP_PUT) {
        uvapi_req.method = HttpMethod::PUT;
    } else if (uvhttp_method == UVHTTP_DELETE) {
        uvapi_req.method = HttpMethod::DELETE;
    } else if (uvhttp_method == UVHTTP_HEAD) {
        uvapi_req.method = HttpMethod::HEAD;
    } else if (uvhttp_method == UVHTTP_OPTIONS) {
        uvapi_req.method = HttpMethod::OPTIONS;
    } else if (uvhttp_method == UVHTTP_PATCH) {
        uvapi_req.method = HttpMethod::PATCH;
    } else {
        uvapi_req.method = HttpMethod::ANY;
    }
    
    uvapi_req.url_path = uvhttp_request_get_path(req);
    uvapi_req.user_id = 0;
    
    // 获取请求头
    for (size_t i = 0; i < req->header_count; i++) {
        const uvhttp_header_t* header = uvhttp_request_get_header_at(req, i);
        if (header) {
            uvapi_req.headers[header->name] = header->value;
        }
    }
    
    // 获取查询参数
    const char* query_string = uvhttp_request_get_query_string(req);
    if (query_string && strlen(query_string) > 0) {
        // 简单解析查询参数
        char* query_copy = strdup(query_string);
        if (!query_copy) {
            uvhttp_response_set_status(resp, 500);
            uvhttp_response_send(resp);
            return -1;
        }
        char* token = strtok(query_copy, "&");
        while (token != nullptr) {
            char* equals = strchr(token, '=');
            if (equals) {
                *equals = '\0';
                uvapi_req.query_params[token] = equals + 1;
            }
            token = strtok(nullptr, "&");
        }
        free(query_copy);
    }
    
    // 获取请求体
    const char* body = uvhttp_request_get_body(req);
    if (body) {
        uvapi_req.body = body;
    }
    
    // 使用 uvhttp 路由匹配获取路径参数
    uvhttp_route_match_t match;
    uvhttp_error_t match_result = uvhttp_router_match(
        svr_instance->router_.get(),
        uvapi_req.url_path.c_str(),
        method_str,
        &match
    );
    
    // 查找自定义处理器
    auto handler = svr_instance->findHandler(uvapi_req.url_path, uvapi_req.method);
    
    if (handler) {
        // 提取路径参数（仅在匹配成功时）
        if (match_result == UVHTTP_OK) {
            for (size_t i = 0; i < match.param_count; i++) {
                uvapi_req.path_params[match.params[i].name] = match.params[i].value;
            }
        }
        
        // 调用处理器
        HttpResponse uvapi_resp = handler(uvapi_req);
        
        // 设置响应
        uvhttp_response_set_status(resp, uvapi_resp.status_code);
        
        // 设置响应头
        for (std::map<std::string, std::string>::const_iterator it = uvapi_resp.headers.begin(); 
             it != uvapi_resp.headers.end(); ++it) {
            uvhttp_response_set_header(resp, it->first.c_str(), it->second.c_str());
        }
        
        // 设置响应体
        if (!uvapi_resp.body.empty()) {
            uvhttp_response_set_body(resp, uvapi_resp.body.c_str(), uvapi_resp.body.size());
        }
        
        uvhttp_response_send(resp);
        return 0;
    }
    
    // 404 Not Found
    uvhttp_response_set_status(resp, 404);
    uvhttp_response_set_header(resp, "Content-Type", "application/json");
    const char* not_found = R"({"error": "Not Found", "message": "The requested resource was not found"})";
    uvhttp_response_set_body(resp, not_found, strlen(not_found));
    uvhttp_response_send(resp);
    
    return 0;
}

server::Server::Server(uv_loop_t* loop) 
    : loop_(loop), use_https_(false) {
    
    if (!loop_) {
        std::cerr << "Error: Event loop cannot be null" << std::endl;
        return;
    }
    
    // 创建 uvhttp 上下文（两步法：创建 + 初始化）
    uvhttp_context_t* ctx_ptr = nullptr;
    uvhttp_error_t result = uvhttp_context_create(loop_, &ctx_ptr);
    if (result != UVHTTP_OK || !ctx_ptr) {
        std::cerr << "Error: Failed to create uvhttp context" << std::endl;
        return;
    }
    
    result = uvhttp_context_init(ctx_ptr);
    if (result != UVHTTP_OK) {
        uvhttp_context_destroy(ctx_ptr);
        std::cerr << "Error: Failed to initialize uvhttp context" << std::endl;
        return;
    }
    uvhttp_ctx_ = makeUvhttpContext(ctx_ptr);
    
    // 创建配置
    uvhttp_config_t* config_ptr = nullptr;
    result = uvhttp_config_new(&config_ptr);
    if (result != UVHTTP_OK || !config_ptr) {
        std::cerr << "Error: Failed to create uvhttp config" << std::endl;
        return;
    }
    config_ = makeUvhttpConfig(config_ptr);
    
    // 创建路由器
    uvhttp_router_t* router_ptr = nullptr;
    result = uvhttp_router_new(&router_ptr);
    if (result != UVHTTP_OK || !router_ptr) {
        std::cerr << "Error: Failed to create uvhttp router" << std::endl;
        return;
    }
    router_ = makeUvhttpRouter(router_ptr);
    
    // 创建服务器
    uvhttp_server_t* server_ptr = nullptr;
    result = uvhttp_server_new(loop_, &server_ptr);
    if (result != UVHTTP_OK || !server_ptr) {
        std::cerr << "Error: Failed to create uvhttp server" << std::endl;
        return;
    }
    server_ = makeUvhttpServer(server_ptr);
    
    // 设置路由器到服务器
    server_->router = router_.get();
    server_->config = config_.get();
    server_->user_data = this;
}

// 移动构造函数
server::Server::Server(Server&& other) noexcept
    : loop_(other.loop_),
      server_(std::move(other.server_)),
      router_(std::move(other.router_)),
      uvhttp_ctx_(std::move(other.uvhttp_ctx_)),
      config_(std::move(other.config_)),
      tls_config_(std::move(other.tls_config_)),
      use_https_(other.use_https_),
      handlers_(std::move(other.handlers_)) {
    if (server_) server_->user_data = this;
}

// 移动赋值运算符
server::Server& server::Server::operator=(Server&& other) noexcept {
    if (this != &other) {
        loop_ = other.loop_;
        server_ = std::move(other.server_);
        router_ = std::move(other.router_);
        uvhttp_ctx_ = std::move(other.uvhttp_ctx_);
        config_ = std::move(other.config_);
        tls_config_ = std::move(other.tls_config_);
        use_https_ = other.use_https_;
        handlers_ = std::move(other.handlers_);
    }
    if (server_) server_->user_data = this;
    return *this;
}

// 析构函数 - RAII 自动清理，不需要手动释放
server::Server::~Server() {
    // 所有资源由 RAII 包装类自动管理
}

bool server::Server::listen(const std::string& host, int port) {
    if (!server_ || !loop_) {
        return false;
    }
    
    // 设置处理器
    uvhttp_server_set_handler(server_.get(), on_uvhttp_request);
    
    // 启动服务器
    uvhttp_error_t result = uvhttp_server_listen(server_.get(), host.c_str(), port);
    if (result != UVHTTP_OK) {
        fprintf(stderr, "Failed to start server on %s:%d\n", host.c_str(), port);
        return false;
    }
    
    return true;
}

bool server::Server::listenHttps(const std::string& host, int port) {
    if (!tls_config_.enabled) {
        fprintf(stderr, "Error: TLS not enabled. Call enableTls() first\n");
        return false;
    }
    
    return listen(host, port);
}

void server::Server::stop() {
    if (server_) {
        uvhttp_server_stop(server_.get());
    }
}

void server::Server::enableTls(const TlsConfig& tls_config) {
    tls_config_ = tls_config;
    
    if (!tls_config_.enabled) {
        return;
    }
    
    // 创建 TLS 上下文
    uvhttp_tls_context_t* tls_ctx = nullptr;
    uvhttp_error_t result = uvhttp_tls_context_new(&tls_ctx);
    if (result != UVHTTP_OK || !tls_ctx) {
        std::cerr << "Error: Failed to create TLS context" << std::endl;
        return;
    }
    
    // 加载证书
    result = uvhttp_tls_context_load_cert_chain(tls_ctx, tls_config_.cert_file.c_str());
    if (result != UVHTTP_OK) {
        std::cerr << "Error: Failed to load certificate file: " << tls_config_.cert_file << std::endl;
        uvhttp_tls_context_free(tls_ctx);
        return;
    }
    
    // 加载私钥
    result = uvhttp_tls_context_load_private_key(tls_ctx, tls_config_.key_file.c_str());
    if (result != UVHTTP_OK) {
        std::cerr << "Error: Failed to load private key file: " << tls_config_.key_file << std::endl;
        uvhttp_tls_context_free(tls_ctx);
        return;
    }
    
    // 可选：加载 CA 证书（用于客户端认证）
    if (!tls_config_.ca_file.empty()) {
        result = uvhttp_tls_context_load_ca_file(tls_ctx, tls_config_.ca_file.c_str());
        if (result != UVHTTP_OK) {
            std::cerr << "Warning: Failed to load CA file: " << tls_config_.ca_file << std::endl;
        } else {
            // 启用客户端认证
            uvhttp_tls_context_enable_client_auth(tls_ctx, 1);
        }
    }
    
    // 启用 TLS 到服务器
    result = uvhttp_server_enable_tls(server_.get(), tls_ctx);
    if (result != UVHTTP_OK) {
        std::cerr << "Error: Failed to enable TLS on server" << std::endl;
        uvhttp_tls_context_free(tls_ctx);
        return;
    }
    
    // TLS 上下文所有权已转移给 server，无需手动释放
    use_https_ = true;
    std::cout << "TLS/SSL enabled successfully" << std::endl;
}

void server::Server::addRoute(const std::string& path, HttpMethod method, 
                      std::function<HttpResponse(const HttpRequest&)> handler) {
    // 存储自定义处理器
    handlers_[path][toUvhttpMethod(method)] = handler;
    
    // 使用 uvhttp 的路由 API 注册路由
    uvhttp_error_t result = uvhttp_router_add_route_method(
        router_.get(),
        path.c_str(),
        toUvhttpMethod(method),
        on_uvhttp_request
    );
    
    (void)result;  // 忽略返回值避免警告
    
    if (result != UVHTTP_OK) {
        std::cerr << "Error: Failed to add route " << path << std::endl;
    }
}

std::function<HttpResponse(const HttpRequest&)> server::Server::findHandler(
    const std::string& path, HttpMethod method) const {
    // 查找自定义处理器
    auto path_it = handlers_.find(path);
    if (path_it != handlers_.end()) {
        auto method_it = path_it->second.find(toUvhttpMethod(method));
        if (method_it != path_it->second.end()) {
            return method_it->second;
        }
    }
    return nullptr;
}

} // namespace server

// ========== Body Schema 辅助函数和方法实现 ==========

namespace {

// JSON 验证辅助函数
std::string validateJsonType(const cJSON* json, FieldType expected_type, const std::string& field_name) {
    if (!json) {
        return "Field '" + field_name + "' is missing";
    }
    
    switch (expected_type) {
        case FieldType::STRING:
            if (!cJSON_IsString(json)) {
                return "Field '" + field_name + "' must be a string";
            }
            break;
        case FieldType::INT:
            if (!cJSON_IsNumber(json) || json->valuedouble != static_cast<int>(json->valuedouble)) {
                return "Field '" + field_name + "' must be an integer";
            }
            break;
        case FieldType::INT64:
            if (!cJSON_IsNumber(json)) {
                return "Field '" + field_name + "' must be an integer";
            }
            break;
        case FieldType::FLOAT:
            if (!cJSON_IsNumber(json)) {
                return "Field '" + field_name + "' must be a number";
            }
            break;
        case FieldType::DOUBLE:
            if (!cJSON_IsNumber(json)) {
                return "Field '" + field_name + "' must be a number";
            }
            break;
        case FieldType::BOOL:
            if (!cJSON_IsBool(json)) {
                return "Field '" + field_name + "' must be a boolean";
            }
            break;
        case FieldType::ARRAY:
            if (!cJSON_IsArray(json)) {
                return "Field '" + field_name + "' must be an array";
            }
            break;
        case FieldType::OBJECT:
            if (!cJSON_IsObject(json)) {
                return "Field '" + field_name + "' must be an object";
            }
            break;
        case FieldType::CUSTOM:
            // 自定义类型不进行类型检查
            break;
    }
    
    return "";
}

// 应用验证规则
// 设置字段值（支持嵌套对象和数组）
void setFieldValue(void* instance, const FieldDefinition& field_def, const cJSON* json) {
    if (!instance || !json) return;
    
    char* field_ptr = static_cast<char*>(instance) + field_def.offset;
    
    switch (field_def.type) {
        case FieldType::STRING:
            if (cJSON_IsString(json)) {
                *reinterpret_cast<std::string*>(field_ptr) = json->valuestring;
            }
            break;
        case FieldType::INT:
            if (cJSON_IsNumber(json)) {
                *reinterpret_cast<int*>(field_ptr) = static_cast<int>(json->valuedouble);
            }
            break;
        case FieldType::INT64:
            if (cJSON_IsNumber(json)) {
                *reinterpret_cast<int64_t*>(field_ptr) = static_cast<int64_t>(json->valuedouble);
            }
            break;
        case FieldType::FLOAT:
            if (cJSON_IsNumber(json)) {
                *reinterpret_cast<float*>(field_ptr) = static_cast<float>(json->valuedouble);
            }
            break;
        case FieldType::DOUBLE:
            if (cJSON_IsNumber(json)) {
                *reinterpret_cast<double*>(field_ptr) = json->valuedouble;
            }
            break;
        case FieldType::BOOL:
            if (cJSON_IsBool(json)) {
                *reinterpret_cast<bool*>(field_ptr) = cJSON_IsTrue(json);
            }
            break;
        case FieldType::OBJECT:
            // 嵌套对象
            if (cJSON_IsObject(json) && field_def.nested_schema) {
                char* nested_obj_str = cJSON_PrintUnformatted(json);
                field_def.nested_schema->fromJson(nested_obj_str, field_ptr);
                if (nested_obj_str) free(nested_obj_str);
            }
            break;
        case FieldType::ARRAY:
            // 嵌套数组
            if (cJSON_IsArray(json) && field_def.item_schema) {
                // 注意：这里需要处理 std::vector 的具体情况
                // 由于模板类型擦除，这个实现比较复杂
                // 暂时跳过数组实现
            }
            break;
        case FieldType::CUSTOM:
            // 自定义类型
            break;
    }
}

// 创建 JSON 字段
cJSON* createJsonField(void* instance, size_t offset, FieldType type) {
    if (!instance) return nullptr;
    
    char* field_ptr = static_cast<char*>(instance) + offset;
    
    switch (type) {
        case FieldType::STRING:
            return cJSON_CreateString(reinterpret_cast<std::string*>(field_ptr)->c_str());
        case FieldType::INT:
            return cJSON_CreateNumber(*reinterpret_cast<int*>(field_ptr));
        case FieldType::INT64:
            return cJSON_CreateNumber(static_cast<double>(*reinterpret_cast<int64_t*>(field_ptr)));
        case FieldType::FLOAT:
            return cJSON_CreateNumber(*reinterpret_cast<float*>(field_ptr));
        case FieldType::DOUBLE:
            return cJSON_CreateNumber(*reinterpret_cast<double*>(field_ptr));
        case FieldType::BOOL:
            return cJSON_CreateBool(*reinterpret_cast<bool*>(field_ptr));
        case FieldType::ARRAY:
        case FieldType::OBJECT:
        case FieldType::CUSTOM:
            // TODO: 实现复杂类型
            return cJSON_CreateNull();
    }
    
    return nullptr;
}

} // anonymous namespace

// ========== Body Schema 基类方法实现 ==========

std::string BodySchemaBase::validate(const cJSON* json) const {
    if (!cJSON_IsObject(json)) {
        return "Request body must be a JSON object";
    }
    
    std::vector<FieldDefinition> fields_vec = fields();
    
    // 检查必填字段
    for (const auto& field : fields_vec) {
        if (field.validation.required) {
            cJSON* field_json = cJSON_GetObjectItem(json, field.name.c_str());
            if (!field_json) {
                return "Field '" + field.name + "' is required";
            }
        }
    }
    
    // 验证每个字段
    for (const auto& field : fields_vec) {
        cJSON* field_json = cJSON_GetObjectItem(json, field.name.c_str());
        
        // 如果字段不存在且不是必填的，跳过验证
        if (!field_json && !field.validation.required) {
            continue;
        }
        
        // 如果字段不存在但是必填的，已经在前面检查过了
        if (!field_json) {
            continue;
        }
        
        // 执行类型验证
        std::string error = validateJsonType(field_json, field.type, field.name);
        if (!error.empty()) {
            return error;
        }
        
        // 执行值验证（长度、范围等）
        error = applyValidation(field_json, field.validation, field.name);
        if (!error.empty()) {
            return error;
        }
    }
    
    // 检查未知字段
    for (cJSON* item = json->child; item; item = item->next) {
        bool found = false;
        for (const auto& field : fields_vec) {
            if (field.name == item->string) {
                found = true;
                break;
            }
        }
        if (!found) {
            return "Unknown field '" + std::string(item->string) + "'";
        }
    }
    
    return "";
}

bool BodySchemaBase::fromJson(const std::string& json_str, void* instance) const {
    cJSON* json = cJSON_Parse(json_str.c_str());
    if (!json || !cJSON_IsObject(json)) {
        if (json) cJSON_Delete(json);
        return false;
    }
    
    // 先验证
    std::string error = validate(json);
    if (!error.empty()) {
        cJSON_Delete(json);
        return false;
    }
    
    // 设置字段值
    std::vector<FieldDefinition> fields_vec = fields();
    for (const auto& field : fields_vec) {
        cJSON* field_json = cJSON_GetObjectItem(json, field.name.c_str());
        if (field_json) {
            setFieldValue(instance, field, field_json);
        } else if (!field.validation.required) {
            // 设置可选字段的默认值
            switch (field.type) {
                case FieldType::STRING:
                    *reinterpret_cast<std::string*>(static_cast<char*>(instance) + field.offset) = "";
                    break;
                case FieldType::INT:
                    *reinterpret_cast<int*>(static_cast<char*>(instance) + field.offset) = 0;
                    break;
                case FieldType::INT64:
                    *reinterpret_cast<int64_t*>(static_cast<char*>(instance) + field.offset) = 0;
                    break;
                case FieldType::FLOAT:
                    *reinterpret_cast<float*>(static_cast<char*>(instance) + field.offset) = 0.0f;
                    break;
                case FieldType::DOUBLE:
                    *reinterpret_cast<double*>(static_cast<char*>(instance) + field.offset) = 0.0;
                    break;
                case FieldType::BOOL:
                    *reinterpret_cast<bool*>(static_cast<char*>(instance) + field.offset) = false;
                    break;
                default:
                    break;
            }
        }
    }
    
    cJSON_Delete(json);
    return true;
}

std::string BodySchemaBase::toJson(void* instance) const {
    cJSON* json = cJSON_CreateObject();
    if (!json) return "{}";
    
    std::vector<FieldDefinition> fields_vec = fields();
    for (const auto& field : fields_vec) {
        cJSON* field_json = createJsonField(instance, field.offset, field.type);
        if (field_json) {
            cJSON_AddItemToObject(json, field.name.c_str(), field_json);
        }
    }
    
    char* json_str = cJSON_Print(json);
    std::string result(json_str ? json_str : "{}");
    if (json_str) free(json_str);
    cJSON_Delete(json);
    
    return result;
}

// ========== RESTful API 层实现 ==========

namespace restful {

Api::Api(uv_loop_t* loop) 
    : api_title_("RESTful API")
    , api_description_("A RESTful API framework")
    , api_version_("1.0.0")
    , running_(false)
    , cors_enabled_(false)
    , server_(nullptr) {
    
    if (!loop) {
        // 事件循环不能为空
        std::cerr << "Error: Event loop cannot be null" << std::endl;
        return;
    }
    
    // 创建 Server 层（注入事件循环）
    server_ = std::unique_ptr<server::Server>(new server::Server(loop));
    if (!server_) {
        std::cerr << "Error: Failed to create server" << std::endl;
    }
}

Api::~Api() {
    stop();
    // server_ 自动释放，无需手动 delete
}

Api& Api::get(const std::string& path, RequestHandler handler) {
    server_->addRoute(path, HttpMethod::GET, handler);
    return *this;
}

Api& Api::post(const std::string& path, RequestHandler handler) {
    server_->addRoute(path, HttpMethod::POST, handler);
    return *this;
}

Api& Api::put(const std::string& path, RequestHandler handler) {
    server_->addRoute(path, HttpMethod::PUT, handler);
    return *this;
}

Api& Api::delete_(const std::string& path, RequestHandler handler) {
    server_->addRoute(path, HttpMethod::DELETE, handler);
    return *this;
}

Api& Api::patch(const std::string& path, RequestHandler handler) {
    server_->addRoute(path, HttpMethod::PATCH, handler);
    return *this;
}

Api& Api::head(const std::string& path, RequestHandler handler) {
    server_->addRoute(path, HttpMethod::HEAD, handler);
    return *this;
}

Api& Api::options(const std::string& path, RequestHandler handler) {
    server_->addRoute(path, HttpMethod::OPTIONS, handler);
    return *this;
}

bool Api::run(const std::string& host, int port) {
    if (running_ || !server_) {
        return false;
    }
    
    // 启动服务器监听
    if (!server_->listen(host, port)) {
        return false;
    }
    
    running_ = true;
    std::cout << "Server listening on http://" << host << ":" << port << std::endl;
    
    // 运行事件循环
    uv_run(server_->getLoop(), UV_RUN_DEFAULT);
    
    running_ = false;
    return true;
}

void Api::stop() {
    if (running_ && server_) {
        server_->stop();
        uv_stop(server_->getLoop());
    }
}

// CORS 配置
Api& Api::enableCors(const CorsConfig& config) {
    cors_config_ = config;
    cors_enabled_ = true;
    return *this;
}

Api& Api::enableCors(bool enabled) {
    cors_enabled_ = enabled;
    if (enabled) {
        cors_config_.enabled = true;
    }
    return *this;
}

Api& Api::disableCors() {
    cors_enabled_ = false;
    return *this;
}

// Token 管理
std::string Api::generateToken(int64_t user_id, const std::string& username, 
                              const std::string& role, int64_t expires_in_seconds) {
    // 定期清理过期 token（每 100 次调用清理一次）
    static int call_count = 0;
    if (++call_count % 100 == 0) {
        cleanupExpiredTokens();
    }
    
    // 生成随机 token
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);
    
    std::string token;
    for (int i = 0; i < 32; i++) {
        token += charset[dis(gen)];
    }
    
    // 存储 token 信息
    TokenInfo info;
    info.user_id = user_id;
    info.username = username;
    info.role = role;
    // 使用 int64_t 避免 time_t 32 位溢出问题
    info.expires_at = static_cast<int64_t>(time(nullptr)) + expires_in_seconds;
    tokens_[token] = info;
    
    return token;
}

bool Api::validateToken(const std::string& token, int64_t& user_id, 
                        std::string& username, std::string& role) {
    std::map<std::string, TokenInfo>::iterator it = tokens_.find(token);
    if (it == tokens_.end()) {
        return false;
    }
    
    // 检查过期时间（使用 int64_t 避免溢出问题）
    if (static_cast<int64_t>(time(nullptr)) > it->second.expires_at) {
        tokens_.erase(it);
        return false;
    }
    
    user_id = it->second.user_id;
    username = it->second.username;
    role = it->second.role;
    return true;
}

std::string Api::refreshToken(const std::string& token, int64_t expires_in_seconds) {
    std::map<std::string, TokenInfo>::iterator it = tokens_.find(token);
    if (it == tokens_.end()) {
        return "";
    }
    
    // 检查过期时间（使用 int64_t 避免溢出问题）
    if (static_cast<int64_t>(time(nullptr)) > it->second.expires_at) {
        tokens_.erase(it);
        return "";
    }
    
    // 生成新 token
    std::string new_token = generateToken(it->second.user_id, it->second.username, 
                                         it->second.role, expires_in_seconds);
    
    // 删除旧 token
    tokens_.erase(it);
    
    return new_token;
}

bool Api::revokeToken(const std::string& token) {
    return tokens_.erase(token) > 0;
}

void Api::cleanupExpiredTokens() {
    time_t now = time(nullptr);
    for (std::map<std::string, TokenInfo>::iterator it = tokens_.begin(); it != tokens_.end(); ) {
        if (now > it->second.expires_at) {
            tokens_.erase(it++);
        } else {
            ++it;
        }
    }
}

// 请求处理
HttpResponse Api::handle_request(const HttpRequest& req) {
    (void)req; // 消除未使用参数警告
    // 查找路由处理器
    // 注意：实际的请求处理在 on_uvhttp_request 回调中进行
    // 这个方法保留供其他用途
    return HttpResponse(404).json(JSON::error("Not Found"));
}

// 辅助方法
std::string Api::generateRandomString(size_t length) {
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);
    
    std::string result;
    for (size_t i = 0; i < length; i++) {
        result += charset[dis(gen)];
    }
    return result;
}

std::string Api::extractBearerToken(const std::string& auth_header) {
    if (auth_header.length() >= 7 && auth_header.substr(0, 7) == "Bearer ") {
        return auth_header.substr(7);
    }
    return "";
}

// ========== DSL 路由方法实现 ==========

RouteBuilder Api::route(const std::string& path, HttpMethod method) {
    return RouteBuilder(this, path, method);
}

RouteBuilder Api::get(const std::string& path) {
    return RouteBuilder(this, path, HttpMethod::GET);
}

RouteBuilder Api::post(const std::string& path) {
    return RouteBuilder(this, path, HttpMethod::POST);
}

RouteBuilder Api::put(const std::string& path) {
    return RouteBuilder(this, path, HttpMethod::PUT);
}

RouteBuilder Api::delete_(const std::string& path) {
    return RouteBuilder(this, path, HttpMethod::DELETE);
}

RouteBuilder Api::patch(const std::string& path) {
    return RouteBuilder(this, path, HttpMethod::PATCH);
}

RouteBuilder Api::head(const std::string& path) {
    return RouteBuilder(this, path, HttpMethod::HEAD);
}

RouteBuilder Api::options(const std::string& path) {
    return RouteBuilder(this, path, HttpMethod::OPTIONS);
}

void RouteBuilder::register_() {
    // 将路由注册到 API
    if (api_) {
        std::string path = route_.path;
        HttpMethod method = route_.method;
        RequestHandler handler = route_.handler;
        std::vector<ParamDefinition> path_params = param_group_.getParams();
        std::vector<ParamDefinition> query_params = param_group_.getParams();
        
        // 创建包装的处理器，执行参数验证
        RequestHandler wrapped_handler = [handler, path_params, query_params](const HttpRequest& req) -> HttpResponse {
            // 验证路径参数
            for (const auto& param : path_params) {
                if (param.validation.required) {
                    auto it = req.path_params.find(param.name);
                    if (it == req.path_params.end() || it->second.empty()) {
                        return HttpResponse(400).json(
                            "{\"code\":\"400\",\"message\":\"Path parameter '" + param.name + "' is required\"}"
                        );
                    }
                }
                
                // 执行验证
                auto it = req.path_params.find(param.name);
                if (it != req.path_params.end()) {
                    const std::string& value = it->second;
                    
                    // 整数范围验证
                    if (param.validation.has_min || param.validation.has_max) {
                        char* endptr = nullptr;
                        errno = 0;
                        long int_value = strtol(value.c_str(), &endptr, 10);
                        if (endptr == value.c_str() || *endptr != '\0' || errno == ERANGE) {
                            return HttpResponse(400).json(
                                "{\"code\":\"400\",\"message\":\"Path parameter '" + param.name + "' must be an integer\"}"
                            );
                        }
                        if (param.validation.has_min && int_value < param.validation.min_value) {
                            return HttpResponse(400).json(
                                "{\"code\":\"400\",\"message\":\"Path parameter '" + param.name + "' must be at least " + std::to_string(param.validation.min_value) + "\"}"
                            );
                        }
                        if (param.validation.has_max && int_value > param.validation.max_value) {
                            return HttpResponse(400).json(
                                "{\"code\":\"400\",\"message\":\"Path parameter '" + param.name + "' must be at most " + std::to_string(param.validation.max_value) + "\"}"
                            );
                        }
                    }
                }
            }
            
            // 验证查询参数
            for (const auto& param : query_params) {
                if (param.validation.required) {
                    auto it = req.query_params.find(param.name);
                    if (it == req.query_params.end() || it->second.empty()) {
                        return HttpResponse(400).json(
                            "{\"code\":\"400\",\"message\":\"Query parameter '" + param.name + "' is required\"}"
                        );
                    }
                }
                
                // 执行验证
                auto it = req.query_params.find(param.name);
                if (it != req.query_params.end()) {
                    const std::string& value = it->second;
                    
                    // 整数范围验证
                    if (param.validation.has_min || param.validation.has_max) {
                        char* endptr = nullptr;
                        errno = 0;
                        long int_value = strtol(value.c_str(), &endptr, 10);
                        if (endptr == value.c_str() || *endptr != '\0' || errno == ERANGE) {
                            return HttpResponse(400).json(
                                "{\"code\":\"400\",\"message\":\"Query parameter '" + param.name + "' must be an integer\"}"
                            );
                        }
                        if (param.validation.has_min && int_value < param.validation.min_value) {
                            return HttpResponse(400).json(
                                "{\"code\":\"400\",\"message\":\"Query parameter '" + param.name + "' must be at least " + std::to_string(param.validation.min_value) + "\"}"
                            );
                        }
                        if (param.validation.has_max && int_value > param.validation.max_value) {
                            return HttpResponse(400).json(
                                "{\"code\":\"400\",\"message\":\"Query parameter '" + param.name + "' must be at most " + std::to_string(param.validation.max_value) + "\"}"
                            );
                        }
                    }
                    
                    // 浮点数范围验证
                    if (param.validation.has_min || param.validation.has_max) {
                        char* endptr = nullptr;
                        errno = 0;
                        double double_value = strtod(value.c_str(), &endptr);
                        if (endptr == value.c_str() || *endptr != '\0' || errno == ERANGE) {
                            return HttpResponse(400).json(
                                "{\"code\":\"400\",\"message\":\"Query parameter '" + param.name + "' must be a number\"}"
                            );
                        }
                        if (param.validation.has_min && double_value < param.validation.min_double) {
                            return HttpResponse(400).json(
                                "{\"code\":\"400\",\"message\":\"Query parameter '" + param.name + "' must be at least " + std::to_string(param.validation.min_double) + "\"}"
                            );
                        }
                        if (param.validation.has_max && double_value > param.validation.max_double) {
                            return HttpResponse(400).json(
                                "{\"code\":\"400\",\"message\":\"Query parameter '" + param.name + "' must be at most " + std::to_string(param.validation.max_double) + "\"}"
                            );
                        }
                    }
                    
                    // 枚举值验证
                    if (param.validation.has_enum) {
                        bool found = false;
                        for (const auto& enum_val : param.validation.enum_values) {
                            if (enum_val == value) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            return HttpResponse(400).json(
                                "{\"code\":\"400\",\"message\":\"Query parameter '" + param.name + "' has invalid value\"}"
                            );
                        }
                    }
                }
            }
            
            // 调用原始处理器
            return handler(req);
        };
        
        // 注册路由
        api_->getServer()->addRoute(path, method, wrapped_handler);
    }
}

} // namespace uvapi

// ParamGroup::applyTo 的实现
void uvapi::restful::ParamGroup::applyTo(uvapi::restful::RouteBuilder& route_builder) {
    for (const auto& param : params_) {
        if (param.type == uvapi::restful::ParamType::PATH) {
            if (param.validation.required ||
                param.validation.has_min || param.validation.has_max ||
                param.validation.has_pattern || param.validation.has_enum) {
                // 有验证规则，需要配置
                route_builder.param(param.name, [&](uvapi::restful::ParamBuilder& p) {
                    if (param.validation.required) p.required();
                    if (!param.validation.required) p.optional();
                    if (param.validation.has_min) p.min(param.validation.min_value);
                    if (param.validation.has_max) p.max(param.validation.max_value);
                    if (param.validation.has_min && param.validation.has_max) {
                        p.range(param.validation.min_value, param.validation.max_value);
                    }
                    if (param.validation.has_pattern) p.pattern(param.validation.pattern);
                    if (param.validation.has_enum) p.enum_(param.validation.enum_values);
                });
            } else {
                // 无验证规则，使用空配置
                route_builder.param(param.name, [](uvapi::restful::ParamBuilder& p) {
                    p.optional();
                });
            }
        } else {
            if (param.validation.required ||
                param.validation.has_min || param.validation.has_max ||
                param.validation.has_pattern || param.validation.has_enum) {
                // 有验证规则，需要配置
                route_builder.query(param.name, [&](uvapi::restful::ParamBuilder& p) {
                    if (!param.default_value.empty()) {
                        if (param.validation.has_min || param.validation.has_max) {
                            // 尝试解析为整数，使用 strtol 避免异常
                            char* endptr = nullptr;
                            errno = 0;
                            long val = strtol(param.default_value.c_str(), &endptr, 10);
                            if (endptr == param.default_value.c_str() || *endptr != '\0' || errno != 0) {
                                // 解析失败，使用字符串
                                p.defaultValue(param.default_value);
                            } else {
                                p.defaultValue(static_cast<int>(val));
                            }
                        } else {
                            p.defaultValue(param.default_value);
                        }
                    }
                    if (param.validation.required) p.required();
                    if (!param.validation.required) p.optional();
                    if (param.validation.has_min) p.min(param.validation.min_value);
                    if (param.validation.has_max) p.max(param.validation.max_value);
                    if (param.validation.has_min && param.validation.has_max) {
                        p.range(param.validation.min_value, param.validation.max_value);
                    }
                    if (param.validation.has_pattern) p.pattern(param.validation.pattern);
                    if (param.validation.has_enum) p.enum_(param.validation.enum_values);
                });
            } else {
                // 无验证规则，使用空配置
                route_builder.query(param.name, [](uvapi::restful::ParamBuilder& p) {
                    p.optional();
                });
            }
        }
    }
}

} // namespace uvapi

