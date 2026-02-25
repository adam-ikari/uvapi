#include "../include/framework.h"
#include <iostream>
#include <atomic>

using namespace uvapi;
using namespace restful;

// 请求计数器
std::atomic<uint64_t> request_count{0};

int main() {
    std::cout << "UVAPI 性能测试服务器 (轻量级优化版)" << std::endl;
    std::cout << "===================" << std::endl;
    
    uv_loop_t* loop = uv_default_loop();
    server::Server server(loop);
    
    // 简单的文本响应（零拷贝优化 + 移动语义）
    server.addRoute("/", HttpMethod::GET, [](const HttpRequest& req) -> HttpResponse {
        (void)req;
        request_count++;
        HttpResponse resp(200);
        resp.header("Content-Type", "text/plain");
        resp.setBody("Hello, World!");
        return resp;
    });

    // JSON 响应（零拷贝优化 + 移动语义）
    server.addRoute("/json", HttpMethod::GET, [](const HttpRequest& req) -> HttpResponse {
        (void)req;
        request_count++;
        HttpResponse resp(200);
        resp.header("Content-Type", "application/json");
        resp.setBody("{\"status\":\"ok\",\"message\":\"Hello, World!\"}");
        return resp;
    });

    // 健康检查（零拷贝优化 + 移动语义）
    server.addRoute("/health", HttpMethod::GET, [](const HttpRequest& req) -> HttpResponse {
        (void)req;
        request_count++;
        HttpResponse resp(200);
        resp.header("Content-Type", "text/plain");
        resp.setBody("OK");
        return resp;
    });

    // 请求统计（零拷贝优化 + 移动语义）
    server.addRoute("/stats", HttpMethod::GET, [](const HttpRequest& req) -> HttpResponse {
        (void)req;
        HttpResponse resp(200);
        resp.header("Content-Type", "application/json");
        std::string body = "{\"total_requests\":" + std::to_string(request_count.load()) + "}";
        resp.setBody(std::move(body));
        return resp;
    });
    
    std::cout << "启动服务器在 http://0.0.0.0:8080" << std::endl;
    std::cout << "\n测试路由:" << std::endl;
    std::cout << "  /        - 简单文本响应" << std::endl;
    std::cout << "  /json    - JSON 响应" << std::endl;
    std::cout << "  /health  - 健康检查" << std::endl;
    std::cout << "  /stats   - 请求统计" << std::endl;
    std::cout << "\n性能优化:" << std::endl;
    std::cout << "  - mimalloc 内存分配器" << std::endl;
    std::cout << "  - 零拷贝响应处理" << std::endl;
    std::cout << "\n性能测试命令:" << std::endl;
    std::cout << "  wrk -t1 -c10 -d30s http://localhost:8080/" << std::endl;
    std::cout << "  wrk -t4 -c50 -d30s http://localhost:8080/" << std::endl;
    
    if (!server.listen("0.0.0.0", 8080)) {
        std::cerr << "启动服务器失败" << std::endl;
        return 1;
    }
    
    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    
    std::cout << "\n总请求数: " << request_count.load() << std::endl;
    
    return 0;
}