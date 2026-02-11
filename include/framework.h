void stop();
    uv_loop_t* getLoop() const { return loop_; }
    
    // 路由注册（供上层 API 使用）
    void addRoute(const std::string& path, HttpMethod method, 
                  std::function<HttpResponse(const HttpRequest&)> handler);
    
    // 添加中间件（使用 UVHTTP 的中间件系统）
    void addMiddleware(uvhttp_middleware_t middleware);
    
    // 声明友元函数
    friend int on_uvhttp_request(uvhttp_request_t* req, uvhttp_response_t* resp);
    
    // 路由查找（供内部使用）
    std::function<HttpResponse(const HttpRequest&)> findHandler(const std::string& path, HttpMethod method) const;
    
    // 获取处理器存储（供请求回调使用）
    auto& getHandlers() { return handlers_; }
    
private:
    uv_loop_t* loop_;  // 注入的事件循环，不拥有所有权
    UvhttpServerPtr server_;
    UvhttpRouterPtr router_;
    UvhttpContextPtr uvhttp_ctx_;
    UvhttpConfigPtr config_;
    TlsConfig tls_config_;  // TLS 配置
    bool use_https_;
    // 使用 unordered_map 存储自定义处理器（直接使用 uvhttp 路由）
    std::unordered_map<std::string, std::unordered_map<uvhttp_method_t, std::function<HttpResponse(const HttpRequest&)> > > handlers_;
};