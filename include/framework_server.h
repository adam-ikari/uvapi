/**
 * @file framework_server.h
 * @brief RESTful 低代码框架 Server 层定义
 * 
 * 直接使用 UVHTTP 的路由系统，避免重复实现路由查找逻辑
 */

#ifndef FRAMEWORK_SERVER_H
#define FRAMEWORK_SERVER_H

#include "framework_http.h"

// C 库头文件（已包含 C++ 兼容性）
#include <uv.h>
#include <uvhttp.h>

namespace uvapi {

// ========== Server 层：底层 HTTP 服务器 ==========
namespace server {

// 前向声明
class Server;

// 前向声明
int on_uvhttp_request(uvhttp_request_t* req, uvhttp_response_t* resp);

class Server {
public:
    Server(uv_loop_t* loop);  // 事件循环注入
    ~Server();
    
    bool listen(const std::string& host, int port);
    void stop();
    uv_loop_t* getLoop() const { return loop_; }
    
    // 路由注册（供上层 API 使用）
    void addRoute(const std::string& path, HttpMethod method, 
                  std::function<HttpResponse(const HttpRequest&)> handler);
    
    // 便捷路由注册方法
    void get(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler) {
        addRoute(path, HttpMethod::GET, handler);
    }
    
    void post(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler) {
        addRoute(path, HttpMethod::POST, handler);
    }
    
    void put(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler) {
        addRoute(path, HttpMethod::PUT, handler);
    }
    
    void del(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler) {
        addRoute(path, HttpMethod::DELETE, handler);
    }
    
    void patch(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler) {
        addRoute(path, HttpMethod::PATCH, handler);
    }
    
    void head(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler) {
        addRoute(path, HttpMethod::HEAD, handler);
    }
    
    void options(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler) {
        addRoute(path, HttpMethod::OPTIONS, handler);
    }
    
    // 添加中间件（使用 UVHTTP 的中间件系统）
    void addMiddleware(uvhttp_middleware_t middleware);
    
    // 声明友元函数
    friend int on_uvhttp_request(uvhttp_request_t* req, uvhttp_response_t* resp);
    
private:
    uv_loop_t* loop_;  // 注入的事件循环，不拥有所有权
    uvhttp_server_t* server_;
    uvhttp_router_t* router_;
    uvhttp_context_t* uvhttp_ctx_;
    uvhttp_config_t* config_;
};

} // namespace server

} // namespace uvapi

#endif // FRAMEWORK_SERVER_H