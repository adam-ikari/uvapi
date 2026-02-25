/**
 * @file uvhttp_middleware_example.cpp
 * @brief UVHTTP 中间件使用示例
 * 
 * 演示如何使用 UVHTTP 提供的中间件系统
 */

#include <iostream>
#include <uvhttp.h>
#include <uvhttp_router.h>
#include "../include/uvhttp_middleware_simple.h"

using namespace uvhttp_middleware;

// 处理函数
int my_handler(uvhttp_request_t* uv_req, uvhttp_response_t* uv_resp) {
    // 使用预定义的中间件链
    UVHTTP_EXECUTE_MIDDLEWARE_CHAIN(uv_req, uv_resp, standard_chain);

    // 处理请求
    const char* path = uvhttp_request_get_path(uv_req);
    (void)path;  // 暂时未使用，消除警告

    std::string body = "{\"code\":200,\"message\":\"Hello from UVHTTP Middleware!\"}";
    
    uvhttp_response_set_status(uv_resp, 200);
    uvhttp_response_set_header(uv_resp, "Content-Type", "application/json");
    uvhttp_response_set_body(uv_resp, body.c_str(), body.length());
    
    return 0;
}

// 带认证的处理函数
int auth_handler(uvhttp_request_t* uv_req, uvhttp_response_t* uv_resp) {
    // 使用认证中间件链
    UVHTTP_EXECUTE_MIDDLEWARE_CHAIN(uv_req, uv_resp, auth_chain);
    
    // 如果认证失败，响应已经被中间件设置，直接返回
    if (uv_resp->status_code == 401) {
        return 0;
    }
    
    // 处理请求
    std::string body = "{\"code\":200,\"message\":\"Authenticated access granted\"}";
    
    uvhttp_response_set_status(uv_resp, 200);
    uvhttp_response_set_header(uv_resp, "Content-Type", "application/json");
    uvhttp_response_set_body(uv_resp, body.c_str(), body.length());
    
    return 0;
}

// 使用自定义中间件的处理函数
int custom_handler(uvhttp_request_t* uv_req, uvhttp_response_t* uv_resp) {
    // 使用 UVHTTP_EXECUTE_MIDDLEWARE 宏执行中间件链
    UVHTTP_EXECUTE_MIDDLEWARE(uv_req, uv_resp,
        logging_middleware,
        cors_middleware,
        response_time_middleware
    );
    
    // 处理请求
    std::string body = "{\"code\":200,\"message\":\"Custom middleware chain\"}";
    
    uvhttp_response_set_status(uv_resp, 200);
    uvhttp_response_set_header(uv_resp, "Content-Type", "application/json");
    uvhttp_response_set_body(uv_resp, body.c_str(), body.length());
    
    return 0;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    std::cout << "=== UVHTTP 中间件使用示例 ===" << std::endl;
    
    uv_loop_t* loop = uv_default_loop();
    
    // 创建服务器
    uvhttp_server_t* server = nullptr;
    uvhttp_error_t result = uvhttp_server_new(loop, &server);
    if (result != UVHTTP_OK) {
        std::cerr << "Failed to create server: " << result << std::endl;
        return 1;
    }
    
    // 创建路由器
    uvhttp_router_t* router = nullptr;
    result = uvhttp_router_new(&router);
    if (result != UVHTTP_OK) {
        std::cerr << "Failed to create router: " << result << std::endl;
        uvhttp_server_free(server);
        return 1;
    }
    
    // 添加路由
    uvhttp_router_add_route(router, "/standard", my_handler);
    uvhttp_router_add_route(router, "/auth", auth_handler);
    uvhttp_router_add_route(router, "/custom", custom_handler);
    
    server->router = router;
    
    // 启动服务器
    const char* host = "0.0.0.0";
    int port = 8082;
    result = uvhttp_server_listen(server, host, port);
    
    if (result != UVHTTP_OK) {
        std::cerr << "Failed to start server: " << result << std::endl;
        uvhttp_router_free(router);
        uvhttp_server_free(server);
        return 1;
    }
    
    std::cout << "Server listening on http://" << host << ":" << port << std::endl;
    std::cout << "\n测试路由:" << std::endl;
    std::cout << "  - http://localhost:8082/standard (标准中间件链)" << std::endl;
    std::cout << "  - http://localhost:8082/auth (认证中间件链)" << std::endl;
    std::cout << "  - http://localhost:8082/custom (自定义中间件链)" << std::endl;
    std::cout << "\n按 Ctrl+C 停止服务器" << std::endl;
    
    uv_run(loop, UV_RUN_DEFAULT);
    
    // 清理资源
    uvhttp_router_free(router);
    uvhttp_server_free(server);
    
    return 0;
}
