/**
 * @file uvhttp_middleware_simple.h
 * @brief 简化的 UVHTTP 中间件使用示例
 * 
 * 直接使用 UVHTTP 的中间件宏和系统
 */

#ifndef UVHTTP_MIDDLEWARE_SIMPLE_H
#define UVHTTP_MIDDLEWARE_SIMPLE_H

#include <uvhttp_middleware.h>

namespace uvhttp_middleware {

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
UVHTTP_DEFINE_MIDDLEWARE_CHAIN(standard_chain,
    logging_middleware,
    cors_middleware,
    response_time_middleware
);

UVHTTP_DEFINE_MIDDLEWARE_CHAIN(auth_chain,
    logging_middleware,
    auth_middleware,
    cors_middleware
);

} // namespace uvhttp_middleware

#endif // UVHTTP_MIDDLEWARE_SIMPLE_H