/**
 * @file uvhttp_middleware_adapter.h
 * @brief UVHTTP 中间件适配器 - 将 UVAPI 中间件转换为 UVHTTP 中间件
 */

#ifndef UVHTTP_MIDDLEWARE_ADAPTER_H
#define UVHTTP_MIDDLEWARE_ADAPTER_H

#include "framework.h"
#include <uvhttp_middleware.h>

namespace uvapi {

/**
 * @brief UVHTTP 中间件适配器
 * 
 * 将 UVAPI 的 Middleware 函数适配为 UVHTTP 的 uvhttp_middleware_handler_t
 */
class UvhttpMiddlewareAdapter {
public:
    /**
     * @brief 将 UVAPI 中间件转换为 UVHTTP 中间件处理函数
     * 
     * @param middleware UVAPI 中间件函数
     * @return UVHTTP 中间件处理函数
     */
    static uvhttp_middleware_handler_t adapt(const Middleware& middleware) {
        return [](uvhttp_request_t* uv_req, 
                   uvhttp_response_t* uv_resp, 
                   uvhttp_middleware_context_t* ctx) -> int {
            // 转换 UVHTTP 请求/响应为 UVAPI 请求/响应
            HttpRequest req = convertRequest(uv_req);
            
            // 创建下一个处理器的 lambda
            RequestHandler next_handler = [uv_req, uv_resp](const HttpRequest& r) -> HttpResponse {
                // 将处理后的请求转换回 UVHTTP 响应
                HttpResponse uvapi_resp = r;
                
                // 设置 UVHTTP 响应状态码
                uvhttp_response_set_status(uv_resp, uvapi_resp.status_code);
                
                // 设置响应头
                for (const auto& header : uvapi_resp.headers) {
                    uvhttp_response_set_header(uv_resp, 
                                                 header.first.c_str(), 
                                                 header.second.c_str());
                }
                
                // 设置响应体
                if (!uvapi_resp.body.empty()) {
                    uvhttp_response_set_body(uv_resp, uvapi_resp.body.c_str(), 
                                            uvapi_resp.body.length());
                }
                
                return uvapi_resp;
            };
            
            // 调用 UVAPI 中间件
            HttpResponse uvapi_resp = middleware(req, next_handler);
            
            // 转换 UVAPI 响应为 UVHTTP 响应
            uvhttp_response_set_status(uv_resp, uvapi_resp.status_code);
            
            for (const auto& header : uvapi_resp.headers) {
                uvhttp_response_set_header(uv_resp, 
                                             header.first.c_str(), 
                                             header.second.c_str());
            }
            
            if (!uvapi_resp.body.empty()) {
                uvhttp_response_set_body(uv_resp, uvapi_resp.body.c_str(), 
                                        uvapi_resp.body.length());
            }
            
            // 返回继续处理
            return UVHTTP_MIDDLEWARE_CONTINUE;
        };
    }
    
private:
    /**
     * @brief 将 UVHTTP 请求转换为 UVAPI 请求
     */
    static HttpRequest convertRequest(uvhttp_request_t* uv_req) {
        HttpRequest req;
        
        // 设置 HTTP 方法
        const char* method_str = uvhttp_request_get_method(uv_req);
        if (method_str) {
            req.method = stringToHttpMethod(method_str);
        }
        
        // 设置 URL 路径
        const char* path_str = uvhttp_request_get_path(uv_req);
        if (path_str) {
            req.url_path = path_str;
        }
        
        // 设置请求头
        uvhttp_request_foreach_header(uv_req, [](const char* name, 
                                                     const char* value, 
                                                     void* user_data) {
            HttpRequest* req = static_cast<HttpRequest*>(user_data);
            req->headers[name] = value;
            return 0;
        }, &req);
        
        // 设置查询参数
        const char* query_str = uvhttp_request_get_query(uv_req);
        if (query_str) {
            parseQueryString(query_str, req.query_params);
        }
        
        // 设置请求体
        const char* body_str = uvhttp_request_get_body(uv_req);
        size_t body_len = uvhttp_request_get_body_length(uv_req);
        if (body_str && body_len > 0) {
            req.body = std::string(body_str, body_len);
        }
        
        return req;
    }
    
    /**
     * @brief 解析查询字符串
     */
    static void parseQueryString(const std::string& query, 
                                 std::map<std::string, std::string>& params) {
        size_t pos = 0;
        while (pos < query.length()) {
            size_t key_end = query.find('=', pos);
            if (key_end == std::string::npos) break;
            
            size_t value_end = query.find('&', key_end);
            if (value_end == std::string::npos) value_end = query.length();
            
            std::string key = query.substr(pos, key_end - pos);
            std::string value = query.substr(key_end + 1, value_end - key_end - 1);
            
            // URL 解码
            key = urlDecode(key);
            value = urlDecode(value);
            
            params[key] = value;
            pos = value_end + 1;
        }
    }
    
    /**
     * @brief URL 解码
     */
    static std::string urlDecode(const std::string& str) {
        std::string result;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '%' && i + 2 < str.length()) {
                char hex[3] = {str[i+1], str[i+2], 0};
                result += static_cast<char>(std::strtol(hex, nullptr, 16));
                i += 2;
            } else if (str[i] == '+') {
                result += ' ';
            } else {
                result += str[i];
            }
        }
        return result;
    }
    
    /**
     * @brief 将字符串转换为 HTTP 方法枚举
     */
    static HttpMethod stringToHttpMethod(const std::string& method) {
        if (method == "GET") return HttpMethod::GET;
        if (method == "POST") return HttpMethod::POST;
        if (method == "PUT") return HttpMethod::PUT;
        if (method == "DELETE") return HttpMethod::DELETE;
        if (method == "PATCH") return HttpMethod::PATCH;
        if (method == "HEAD") return HttpMethod::HEAD;
        if (method == "OPTIONS") return HttpMethod::OPTIONS;
        return HttpMethod::GET;
    }
};

/**
 * @brief 预定义的 UVHTTP 中间件
 */
namespace UvhttpMiddleware {

/**
 * @brief 日志中间件
 */
inline int logging_middleware(uvhttp_request_t* req, 
                               uvhttp_response_t* resp, 
                               uvhttp_middleware_context_t* ctx) {
    const char* method = uvhttp_request_get_method(req);
    const char* path = uvhttp_request_get_path(req);
    printf("[LOG] %s %s\n", method ? method : "UNKNOWN", path ? path : "/");
    
    return UVHTTP_MIDDLEWARE_CONTINUE;
}

/**
 * @brief CORS 中间件
 */
inline int cors_middleware(uvhttp_request_t* req, 
                           uvhttp_response_t* resp, 
                           uvhttp_middleware_context_t* ctx) {
    // 设置 CORS 头
    uvhttp_response_set_header(resp, "Access-Control-Allow-Origin", "*");
    uvhttp_response_set_header(resp, "Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");
    uvhttp_response_set_header(resp, "Access-Control-Allow-Headers", "Content-Type, Authorization");
    
    return UVHTTP_MIDDLEWARE_CONTINUE;
}

/**
 * @brief 认证中间件（简单示例）
 */
inline int auth_middleware(uvhttp_request_t* req, 
                           uvhttp_response_t* resp, 
                           uvhttp_middleware_context_t* ctx) {
    // 检查 Authorization 头
    const char* auth_header = uvhttp_request_get_header(req, "Authorization");
    if (!auth_header || strlen(auth_header) == 0) {
        uvhttp_response_set_status(resp, 401);
        uvhttp_response_set_header(resp, "Content-Type", "application/json");
        const char* body = "{\"code\":401,\"message\":\"Unauthorized: Missing token\"}";
        uvhttp_response_set_body(resp, body, strlen(body));
        return UVHTTP_MIDDLEWARE_STOP;
    }
    
    // TODO: 实际的 token 验证逻辑
    
    return UVHTTP_MIDDLEWARE_CONTINUE;
}

/**
 * @brief 响应时间中间件
 */
inline int response_time_middleware(uvhttp_request_t* req, 
                                    uvhttp_response_t* resp, 
                                    uvhttp_middleware_context_t* ctx) {
    static __thread uint64_t start_time = 0;
    
    if (!ctx->data) {
        // 请求开始：记录时间
        ctx->data = malloc(sizeof(uint64_t));
        if (ctx->data) {
            uint64_t now = uv_hrtime();
            memcpy(ctx->data, &now, sizeof(uint64_t));
            ctx->cleanup = [](void* data) {
                free(data);
            };
        }
    } else {
        // 请求结束：计算响应时间
        uint64_t end_time = uv_hrtime();
        uint64_t start_time = 0;
        memcpy(&start_time, ctx->data, sizeof(uint64_t));
        
        double duration_ms = (end_time - start_time) / 1000000.0;
        printf("[RESPONSE_TIME] %.2f ms\n", duration_ms);
    }
    
    return UVHTTP_MIDDLEWARE_CONTINUE;
}

/**
 * @brief 定义中间件链
 */
UVHTTP_DEFINE_MIDDLEWARE_CHAIN(uvapi_standard_chain,
    logging_middleware,
    cors_middleware,
    response_time_middleware
);

UVHTTP_DEFINE_MIDDLEWARE_CHAIN(uvapi_auth_chain,
    logging_middleware,
    auth_middleware,
    cors_middleware
);

} // namespace UvhttpMiddleware

} // namespace uvapi

#endif // UVHTTP_MIDDLEWARE_ADAPTER_H