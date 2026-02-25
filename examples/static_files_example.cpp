#include "framework.h"
#include <iostream>
#include <chrono>

using namespace uvapi;
using namespace uvapi::restful;

/**
 * @brief 静态文件服务示例
 * 
 * 本示例展示如何使用 uvhttp 的静态文件服务功能
 */

int main(int argc, char* argv[]) {
    // 从命令行参数获取端口号，默认 8080
    int port = 8080;
    if (argc > 1) {
        port = std::atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            std::cerr << "Invalid port number: " << argv[1] << std::endl;
            return 1;
        }
    }

    std::cout << "=== 静态文件服务示例 ===" << std::endl << std::endl;

    // 创建事件循环
    uv_loop_t* loop = uv_default_loop();
    
    // 创建 API 实例
    Api api(loop);
    
    // 配置 API 信息
    api.title("Static Files Example")
       .description("示例：使用 uvhttp 静态文件服务")
       .version("1.0.0");
    
    // ========== 启用静态文件服务 ==========
    std::cout << "1. 启用静态文件服务..." << std::endl;
    
    // 方法 1: 使用默认配置
    // bool success = api.getServer()->enableStaticFiles("./public", "/static");
    
    // 方法 2: 自定义配置（先获取 Server 实例）
    auto server = api.getServer();
    
    // 启用静态文件服务
    bool success = server->enableStaticFiles(
        "./public",    // 根目录
        "/static",     // URL 前缀
        true           // 启用缓存
    );
    
    if (!success) {
        std::cerr << "Failed to enable static files" << std::endl;
        return 1;
    }
    
    std::cout << "   ✓ 静态文件服务已启用" << std::endl;
    std::cout << "   - 根目录: ./public" << std::endl;
    std::cout << "   - URL 前缀: /static" << std::endl;
    std::cout << "   - 缓存: 已启用" << std::endl << std::endl;
    
    // ========== 预热缓存 ==========
    std::cout << "2. 预热缓存..." << std::endl;
    
    // 预热单个文件
    server->prewarmCache("index.html");
    
    // 预热整个目录（最多 100 个文件）
    server->prewarmDirectory("css", 100);
    server->prewarmDirectory("js", 100);
    server->prewarmDirectory("images", 100);
    
    std::cout << "   ✓ 缓存预热完成" << std::endl << std::endl;
    
    // ========== 添加一些 API 路由 ==========
    std::cout << "3. 添加 API 路由..." << std::endl;
    
    // 健康检查
    server->addRoute("/health", HttpMethod::GET, [](const HttpRequest&) -> HttpResponse {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        
        std::string body = JSON::Object()
            .set("status", "ok")
            .set("message", "Server is running")
            .set("timestamp", static_cast<int64_t>(timestamp))
            .toString();
        
        return HttpResponse(200).json(body);
    });
    
    // API 信息
    server->addRoute("/api/info", HttpMethod::GET, [](const HttpRequest&) -> HttpResponse {
        std::string body = JSON::Object()
            .set("name", "Static Files Example")
            .set("version", "1.0.0")
            .set("static_files", JSON::Object()
                .set("enabled", true)
                .set("prefix", "/static")
                .set("root", "./public")
            )
            .toString();
        
        return HttpResponse(200).json(body);
    });
    
    std::cout << "   ✓ API 路由已添加" << std::endl << std::endl;
    
    // ========== 启动服务器 ==========
    std::cout << "4. 启动服务器..." << std::endl;
    std::cout << "   监听地址: http://0.0.0.0:8080" << std::endl;
    std::cout << "   静态文件: http://0.0.0.0:8080/static/*" << std::endl;
    std::cout << "   健康检查: http://0.0.0.0:8080/health" << std::endl;
    std::cout << "   API 信息: http://0.0.0.0:8080/api/info" << std::endl;
    std::cout << std::endl;
    std::cout << "按 Ctrl+C 停止服务器" << std::endl << std::endl;
    
    if (!api.run("0.0.0.0", port)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    return 0;
}

/**
 * 静态文件服务使用说明：
 * 
 * 1. 创建 public 目录并添加文件：
 *    mkdir -p public/css public/js public/images
 *    echo "Hello, World!" > public/index.html
 *    cp your-style.css public/css/
 *    cp your-script.js public/js/
 * 
 * 2. 访问静态文件：
 *    - http://localhost:8080/static/index.html
 *    - http://localhost:8080/static/css/style.css
 *    - http://localhost:8080/static/js/script.js
 * 
 * 3. 缓存预热：
 *    - 在应用启动时预热常用文件
 *    - 预热整个目录以加载多个文件
 *    - 可以设置最大文件数限制
 * 
 * 4. 缓存管理：
 *    - 定期清理缓存：server->clearStaticCache()
 *    - 动态调整缓存大小：server->setStaticCacheConfig(...)
 *    - 监控缓存命中率（需要自定义实现）
 * 
 * 5. 性能优化：
 *    - 启用 sendfile 零拷贝传输（默认启用）
 *    - 使用 ETag 和 Last-Modified 头（默认启用）
 *    - 设置合理的缓存大小和过期时间
 *    - 预热常用文件以减少首次访问延迟
 * 
 * 6. 安全注意事项：
 *    - 确保根目录权限正确
 *    - 禁用目录列表（默认禁用）
 *    - 设置最大文件大小限制
 *    - 使用安全的文件路径解析
 * 
 * 7. 生产环境建议：
 *    - 使用 CDN 加速静态资源
 *    - 设置更长的缓存时间（如 1 天）
 *    - 启用 gzip 压缩（通过中间件）
 *    - 监控缓存使用情况
 */