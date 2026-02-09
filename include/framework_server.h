/**
 * @file framework_server.h
 * @brief RESTful 低代码框架 Server 层定义
 */

#ifndef FRAMEWORK_SERVER_H
#define FRAMEWORK_SERVER_H

#include "framework_http.h"

extern "C" {
#include <uv.h>
#include <uvhttp.h>
}

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
    
    // 声明友元函数
    friend int on_uvhttp_request(uvhttp_request_t* req, uvhttp_response_t* resp);
    
    // 路由查找（供内部使用）
    std::function<HttpResponse(const HttpRequest&)> findHandler(const std::string& path, HttpMethod method) const;
    
private:
    uv_loop_t* loop_;  // 注入的事件循环，不拥有所有权
    uvhttp_server_t* server_;
    uvhttp_router_t* router_;
    uvhttp_context_t* uvhttp_ctx_;
    uvhttp_config_t* config_;
    // 使用 unordered_map 存储自定义处理器（直接使用 uvhttp 路由）
    std::unordered_map<std::string, std::unordered_map<uvhttp_method_t, std::function<HttpResponse(const HttpRequest&)> > > handlers_;
};

} // namespace server

} // namespace uvapi

#endif // FRAMEWORK_SERVER_H