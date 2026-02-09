#include "../include/framework.h"
#include <iostream>
#include <atomic>

using namespace uvapi;
using namespace restful;

// 请求计数器
std::atomic<uint64_t> request_count{0};

int main() {
    std::cout << "UVAPI 性能测试服务器" << std::endl;
    std::cout << "===================" << std::endl;
    
    uv_loop_t* loop = uv_default_loop();
    server::Server server(loop);
    
    // 简单的文本响应
    server.addRoute("/", HttpMethod::GET, [](const HttpRequest& req) -> HttpResponse {
        request_count++;
        HttpResponse resp(200);
        resp.headers["Content-Type"] = "text/plain";
        resp.body = "Hello, World!";
        return resp;
    });
    
    // JSON 响应
    server.addRoute("/json", HttpMethod::GET, [](const HttpRequest& req) -> HttpResponse {
        request_count++;
        HttpResponse resp(200);
        resp.headers["Content-Type"] = "application/json";
        resp.body = "{\"status\":\"ok\",\"message\":\"Hello, World!\"}";
        return resp;
    });
    
    // 健康检查
    server.addRoute("/health", HttpMethod::GET, [](const HttpRequest& req) -> HttpResponse {
        request_count++;
        HttpResponse resp(200);
        resp.headers["Content-Type"] = "text/plain";
        resp.body = "OK";
        return resp;
    });
    
    // 获取请求计数
    server.addRoute("/stats", HttpMethod::GET, [](const HttpRequest& req) -> HttpResponse {
        HttpResponse resp(200);
        resp.headers["Content-Type"] = "application/json";
        resp.body = "{\"total_requests\":" + std::to_string(request_count.load()) + "}";
        return resp;
    });
    
    std::cout << "启动服务器在 http://0.0.0.0:8080" << std::endl;
    std::cout << "\n测试路由:" << std::endl;
    std::cout << "  /        - 简单文本响应" << std::endl;
    std::cout << "  /json    - JSON 响应" << std::endl;
    std::cout << "  /health  - 健康检查" << std::endl;
    std::cout << "  /stats   - 请求统计" << std::endl;
    std::cout << "\n性能测试命令:" << std::endl;
    std::cout << "  wrk -t1 -c10 -d30s http://localhost:8080/" << std::endl;
    std::cout << "  wrk -t4 -c100 -d30s http://localhost:8080/" << std::endl;
    std::cout << "  ab -n 10000 -c 100 http://localhost:8080/" << std::endl;
    
    if (!server.listen("0.0.0.0", 8080)) {
        std::cerr << "启动服务器失败" << std::endl;
        return 1;
    }
    
    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    
    std::cout << "\n总请求数: " << request_count.load() << std::endl;
    
    return 0;
}