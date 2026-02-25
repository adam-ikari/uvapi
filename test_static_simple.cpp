#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <uvhttp.h>
#include <uvhttp_static.h>

// 简单的静态文件处理器
int static_file_handler(uvhttp_request_t* req, uvhttp_response_t* resp) {
    printf("[DEBUG] static_file_handler called\n");
    printf("[DEBUG] Path: %s\n", uvhttp_request_get_path(req));
    
    // 创建静态文件上下文（简化版）
    uvhttp_static_config_t config;
    memset(&config, 0, sizeof(config));
    strncpy(config.root_directory, "./public", UVHTTP_MAX_FILE_PATH_SIZE - 1);
    strncpy(config.index_file, "index.html", UVHTTP_MAX_PATH_SIZE - 1);
    config.enable_sendfile = 1;
    
    uvhttp_static_context_t* static_ctx = NULL;
    uvhttp_error_t result = uvhttp_static_create(&config, &static_ctx);
    if (result != UVHTTP_OK) {
        printf("[ERROR] Failed to create static context: %d\n", result);
        uvhttp_response_set_status(resp, 500);
        uvhttp_response_send(resp);
        return -1;
    }
    
    // 处理静态文件请求
    result = uvhttp_static_handle_request(static_ctx, req, resp);
    printf("[DEBUG] uvhttp_static_handle_request returned: %d\n", result);
    
    // 清理
    uvhttp_static_free(static_ctx);
    
    return 0;
}

int main() {
    printf("=== Simple Static File Test ===\n");
    
    // 创建事件循环
    uv_loop_t* loop = uv_default_loop();
    
    // 创建服务器
    uvhttp_server_t* server = uvhttp_server_new(loop);
    if (!server) {
        printf("Failed to create server\n");
        return 1;
    }
    
    // 创建路由器
    uvhttp_router_t* router = uvhttp_router_new();
    if (!router) {
        printf("Failed to create router\n");
        uvhttp_server_free(server);
        return 1;
    }
    
    // 设置服务器的路由器
    server->router = router;
    
    // 设置服务器的用户数据
    server->user_data = NULL;
    
    // 添加静态文件路由（使用 /static/* 模式）
    uvhttp_error_t result = uvhttp_router_add_route(router, "/static/*", static_file_handler);
    if (result != UVHTTP_OK) {
        printf("Failed to add static route: %d\n", result);
        uvhttp_router_free(router);
        uvhttp_server_free(server);
        return 1;
    }
    printf("Static route added: /static/*\n");
    
    // 启动服务器
    result = uvhttp_server_listen(server, "0.0.0.0", 8082);
    if (result != UVHTTP_OK) {
        printf("Failed to start server: %d\n", result);
        uvhttp_router_free(router);
        uvhttp_server_free(server);
        return 1;
    }
    printf("Server listening on http://0.0.0.0:8082\n");
    printf("Static files: http://0.0.0.0:8082/static/*\n");
    
    // 运行事件循环
    uv_run(loop, UV_RUN_DEFAULT);
    
    // 清理
    uvhttp_router_free(router);
    uvhttp_server_free(server);
    
    return 0;
}