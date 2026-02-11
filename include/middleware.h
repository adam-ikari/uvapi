/**
 * @file middleware.h
 * @brief 中间件支持机制
 * 
 * 为 UVAPI 添加中间件支持，允许在请求处理链中插入自定义逻辑
 */

#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <unordered_map>

namespace uvapi {

// 前置声明
class HttpRequest;
class HttpResponse;

// 处理器函数类型
using Handler = std::function<HttpResponse(const HttpRequest&)>;

// 中间件函数类型
// 参数：请求对象、下一个处理器
// 返回：响应对象
using Middleware = std::function<HttpResponse(const HttpRequest&, Handler)>;

// 中间件链
class MiddlewareChain {
private:
    std::vector<Middleware> middlewares_;
    Handler final_handler_;
    
public:
    MiddlewareChain() {}
    
    // 添加中间件
    void add(Middleware middleware) {
        middlewares_.push_back(middleware);
    }
    
    // 设置最终处理器
    void setFinalHandler(Handler handler) {
        final_handler_ = handler;
    }
    
    // 执行中间件链
    HttpResponse execute(const HttpRequest& request) {
        return executeMiddleware(request, 0);
    }
    
private:
    // 递归执行中间件
    HttpResponse executeMiddleware(const HttpRequest& request, size_t index) {
        // 如果还有中间件，执行当前中间件
        if (index < middlewares_.size()) {
            // 创建下一个处理器（递归调用）
            Handler next = [this, index](const HttpRequest& req) -> HttpResponse {
                return executeMiddleware(req, index + 1);
            };
            
            // 执行当前中间件
            return middlewares_[index](request, next);
        }
        
        // 所有中间件执行完毕，执行最终处理器
        if (final_handler_) {
            return final_handler_(request);
        }
        
        // 没有最终处理器，返回 404
        HttpResponse resp(404);
        resp.headers["Content-Type"] = "application/json";
        resp.body = "{\"error\":\"Not Found\"}";
        return resp;
    }
};

// 常用中间件工厂函数

// 日志中间件
inline Middleware logger() {
    return [](const HttpRequest& req, Handler next) -> HttpResponse {
        // 记录请求信息
        std::cout << "[REQUEST] " << req.url_path << std::endl;
        
        // 执行下一个处理器
        HttpResponse response = next(req);
        
        // 记录响应信息
        std::cout << "  -> Response sent" << std::endl;
        
        return response;
    };
}

// CORS 中间件
inline Middleware cors(const std::string& allowed_origins = "*",
                       const std::string& allowed_methods = "GET, POST, PUT, DELETE, PATCH, OPTIONS",
                       const std::string& allowed_headers = "Content-Type, Authorization") {
    return [=](const HttpRequest& req, Handler next) -> HttpResponse {
        // 处理 OPTIONS 预检请求
        if (static_cast<int>(req.method) == static_cast<int>(HttpMethod::OPTIONS)) {
            return HttpResponse(204)
                .header("Access-Control-Allow-Origin", allowed_origins)
                .header("Access-Control-Allow-Methods", allowed_methods)
                .header("Access-Control-Allow-Headers", allowed_headers)
                .header("Access-Control-Max-Age", "86400");
        }
        
        HttpResponse resp = next(req);
        resp.header("Access-Control-Allow-Origin", allowed_origins)
            .header("Access-Control-Allow-Methods", allowed_methods)
            .header("Access-Control-Allow-Headers", allowed_headers);
        return resp;
    };
}

// 认证中间件（示例：简单的 Bearer Token 验证）
inline Middleware auth(const std::string& token_header = "Authorization",
                       const std::function<bool(const std::string&)>& validator = nullptr) {
    return [=](const HttpRequest& req, Handler next) -> HttpResponse {
        // 检查 Authorization 头
        auto it = req.headers.find(token_header);
        if (it == req.headers.end()) {
            return HttpResponse(401).setBody("Unauthorized: Missing token").header("Content-Type", "text/plain");
        }
        
        std::string token = it->second;
        
        // 验证 token
        if (validator && !validator(token)) {
            return HttpResponse(401).setBody("Unauthorized: Invalid token").header("Content-Type", "text/plain");
        }
        
        // 简单的 Bearer Token 验证
        if (token.find("Bearer ") != 0) {
            return HttpResponse(401).setBody("Unauthorized: Invalid token format").header("Content-Type", "text/plain");
        }
        
        std::string actual_token = token.substr(7); // 移除 "Bearer "
        if (actual_token.empty()) {
            return HttpResponse(401).setBody("Unauthorized: Empty token").header("Content-Type", "text/plain");
        }
        
        // 执行下一个处理器
        return next(req);
    };
}

// 错误处理中间件
inline Middleware errorHandler() {
    return [](const HttpRequest& req, Handler next) -> HttpResponse {
        try {
            return next(req);
        } catch (const std::exception& e) {
            return HttpResponse(500, std::string("Internal Server Error: ") + e.what());
        } catch (...) {
            return HttpResponse(500, "Internal Server Error: Unknown error");
        }
    };
}

// 请求限流中间件（简单的内存限流）
class RateLimiter {
private:
    std::unordered_map<std::string, std::vector<double>> request_times_;
    int max_requests_;
    double window_seconds_;
    
public:
    RateLimiter(int max_requests, double window_seconds)
        : max_requests_(max_requests), window_seconds_(window_seconds) {}
    
    Middleware create() {
        return [this](const HttpRequest& req, Handler next) -> HttpResponse {
            // 获取客户端 IP（简化：使用 X-Real-IP 或 X-Forwarded-For）
            std::string client_ip = "127.0.0.1";
            auto ip_it = req.headers.find("X-Real-IP");
            if (ip_it != req.headers.end()) {
                client_ip = ip_it->second;
            } else {
                auto forwarded_it = req.headers.find("X-Forwarded-For");
                if (forwarded_it != req.headers.end()) {
                    client_ip = forwarded_it->second;
                }
            }
            
            // 获取当前时间
            double current_time = std::chrono::duration<double>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            
            // 清理过期请求
            auto& times = request_times_[client_ip];
            times.erase(
                std::remove_if(times.begin(), times.end(),
                    [current_time, this](double t) {
                        return current_time - t > window_seconds_;
                    }),
                times.end()
            );
            
            // 检查是否超过限流
            if (times.size() >= static_cast<size_t>(max_requests_)) {
                return HttpResponse(429, "Too Many Requests");
            }
            
            // 记录当前请求
            times.push_back(current_time);
            
            // 执行下一个处理器
            return next(req);
        };
    }
};

// 响应时间中间件
inline Middleware responseTime() {
    return [](const HttpRequest& req, Handler next) -> HttpResponse {
        auto start = std::chrono::high_resolution_clock::now();
        
        HttpResponse response = next(req);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        response.header("X-Response-Time", std::to_string(duration.count()) + "ms");
        
        return response;
    };
}

} // namespace uvapi

#endif // MIDDLEWARE_H