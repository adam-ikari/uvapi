/**
 * @file rate_limiter.h
 * @brief 连接限流功能
 * 
 * 提令牌桶算法实现，用于限制连接速率和并发数
 */

#ifndef RATE_LIMITER_H
#define RATE_LIMITER_H

#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace uvapi {
namespace rate {

/**
 * @brief 令牌桶限流器
 */
class TokenBucket {
private:
    std::atomic<uint64_t> tokens_;
    const uint64_t capacity_;
    const uint64_t refill_rate_;
    std::chrono::milliseconds refill_interval_;
    std::chrono::steady_clock::time_point last_refill_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;

public:
    /**
     * @brief 构造令牌桶
     * @param capacity 桶容量（令牌数）
     * @param refill_rate 填充速率（令牌/秒）
     */
    TokenBucket(uint64_t capacity, uint64_t refill_rate)
        : tokens_(capacity)
        , capacity_(capacity)
        , refill_rate_(refill_rate)
        , refill_interval_(1000)  // 1秒填充一次
        , last_refill_(std::chrono::steady_clock::now()) {}
    
    /**
     * @brief 尝试获取令牌
     * @param count 需要的令牌数
     * @param timeout 超时时间（毫秒）
     * @return 是否成功获取令牌
     */
    bool tryAcquire(uint64_t count = 1, uint64_t timeout_ms = 0) {
        refill();
        
        uint64_t current = tokens_.load();
        while (current >= count) {
            if (tokens_.compare_exchange_weak(current, current - count)) {
                return true;
            }
        }
        
        if (timeout_ms == 0) {
            return false;
        }
        
        // 等待令牌补充
        std::unique_lock<std::mutex> lock(mutex_);
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
        
        while (tokens_.load() < count) {
            if (cv_.wait_until(lock, deadline) == std::cv_status::timeout) {
                return false;
            }
        }
        
        tokens_ -= count;
        return true;
    }
    
    /**
     * @brief 获取当前令牌数
     */
    uint64_t availableTokens() const {
        return tokens_.load();
    }
    
    /**
     * @brief 获取容量
     */
    uint64_t capacity() const {
        return capacity_;
    }
    
    /**
     * @brief 添加令牌（外部调用）
     */
    void addTokens(uint64_t count) {
        tokens_ += count;
        if (tokens_ > capacity_) {
            tokens_ = capacity_;
        }
        cv_.notify_all();
    }

private:
    /**
     * @brief 填充令牌
     */
    void refill() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_refill_);
        
        if (elapsed >= refill_interval_) {
            uint64_t intervals = elapsed.count() / refill_interval_.count();
            uint64_t tokens_to_add = intervals * refill_rate_;
            
            uint64_t current = tokens_.load();
            uint64_t new_tokens = current + tokens_to_add;
            if (new_tokens > capacity_) {
                new_tokens = capacity_;
            }
            
            tokens_.store(new_tokens);
            last_refill_ = now;
            cv_.notify_all();
        }
    }
};

/**
 * @brief 滑动窗口限流器
 */
class SlidingWindow {
private:
    std::vector<std::chrono::steady_clock::time_point> requests_;
    const uint64_t window_size_;  // 窗口大小（毫秒）
    const uint64_t max_requests_;  // 最大请求数
    mutable std::mutex mutex_;

public:
    SlidingWindow(uint64_t window_size_ms, uint64_t max_requests)
        : window_size_(window_size_ms)
        , max_requests_(max_requests) {
        requests_.reserve(max_requests_);
    }
    
    /**
     * @brief 尝试通过限流
     * @return 是否允许请求
     */
    bool tryAcquire() {
        auto now = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 移除窗口外的请求
        auto cutoff = now - std::chrono::milliseconds(window_size_);
        requests_.erase(
            std::remove_if(requests_.begin(), requests_.end(),
                [cutoff](const auto& time) { return time < cutoff; }),
            requests_.end()
        );
        
        // 检查是否超过限制
        if (requests_.size() >= max_requests_) {
            return false;
        }
        
        // 添加当前请求
        requests_.push_back(now);
        return true;
    }
    
    /**
     * @brief 获取当前请求数
     */
    uint64_t currentRequests() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return requests_.size();
    }
};

/**
 * @brief 并发连接数限制器
 */
class ConnectionLimiter {
private:
    std::atomic<uint64_t> current_connections_;
    const uint64_t max_connections_;
    std::mutex mutex_;
    std::condition_variable cv_;

public:
    explicit ConnectionLimiter(uint64_t max_connections)
        : current_connections_(0)
        , max_connections_(max_connections) {}
    
    /**
     * @brief 尝试获取连接
     * @param timeout_ms 超时时间（毫秒），0表示不等待
     * @return 是否成功获取连接
     */
    bool tryAcquire(uint64_t timeout_ms = 0) {
        uint64_t current = current_connections_.load();
        
        if (current < max_connections_) {
            if (current_connections_.compare_exchange_weak(current, current + 1)) {
                return true;
            }
        }
        
        if (timeout_ms == 0) {
            return false;
        }
        
        // 等待连接释放
        std::unique_lock<std::mutex> lock(mutex_);
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
        
        while (current_connections_.load() >= max_connections_) {
            if (cv_.wait_until(lock, deadline) == std::cv_status::timeout) {
                return false;
            }
        }
        
        current_connections_++;
        return true;
    }
    
    /**
     * @brief 释放连接
     */
    void release() {
        current_connections_--;
        cv_.notify_one();
    }
    
    /**
     * @brief 获取当前连接数
     */
    uint64_t currentConnections() const {
        return current_connections_.load();
    }
    
    /**
     * @brief 获取最大连接数
     */
    uint64_t maxConnections() const {
        return max_connections_;
    }
    
    /**
     * @brief 获取连接利用率
     */
    double utilization() const {
        return static_cast<double>(current_connections_.load()) / max_connections_;
    }
};

/**
 * @brief 限流管理器
 */
class RateLimiter {
private:
    std::unique_ptr<TokenBucket> token_bucket_;
    std::unique_ptr<SlidingWindow> sliding_window_;
    std::unique_ptr<ConnectionLimiter> connection_limiter_;
    bool enabled_;
    
public:
    RateLimiter() : enabled_(false) {}
    
    /**
     * @brief 启用令牌桶限流
     */
    void enableTokenBucket(uint64_t capacity, uint64_t refill_rate) {
        token_bucket_ = std::make_unique<TokenBucket>(capacity, refill_rate);
        enabled_ = true;
    }
    
    /**
     * @brief 启用滑动窗口限流
     */
    void enableSlidingWindow(uint64_t window_size_ms, uint64_t max_requests) {
        sliding_window_ = std::make_unique<SlidingWindow>(window_size_ms, max_requests);
        enabled_ = true;
    }
    
    /**
     * @brief 启用连接数限制
     */
    void enableConnectionLimiter(uint64_t max_connections) {
        connection_limiter_ = std::make_unique<ConnectionLimiter>(max_connections);
        enabled_ = true;
    }
    
    /**
     * @brief 检查是否允许请求
     */
    bool isAllowed() {
        if (!enabled_) {
            return true;
        }
        
        // 检查令牌桶
        if (token_bucket_ && !token_bucket_->tryAcquire(1, 0)) {
            return false;
        }
        
        // 检查滑动窗口
        if (sliding_window_ && !sliding_window_->tryAcquire()) {
            return false;
        }
        
        // 检查连接数限制
        if (connection_limiter_ && !connection_limiter_->tryAcquire(0)) {
            return false;
        }
        
        return true;
    }
    
    /**
     * @brief 等待并获取连接
     */
    bool acquireConnection(uint64_t timeout_ms = 5000) {
        if (!enabled_ || !connection_limiter_) {
            return true;
        }
        return connection_limiter_->tryAcquire(timeout_ms);
    }
    
    /**
     * @brief 释放连接
     */
    void releaseConnection() {
        if (connection_limiter_) {
            connection_limiter_->release();
        }
    }
    
    /**
     * @brief 获取限流状态
     */
    std::string getStatus() const {
        if (!enabled_) {
            return "{\"enabled\":false}";
        }
        
        std::string json = "{\"enabled\":true";
        
        if (token_bucket_) {
            json += ",\"token_bucket\":{";
            json += "\"available\":" + std::to_string(token_bucket_->availableTokens());
            json += ",\"capacity\":" + std::to_string(token_bucket_->capacity());
            json += "}";
        }
        
        if (sliding_window_) {
            json += ",\"sliding_window\":{";
            json += "\"current\":" + std::to_string(sliding_window_->currentRequests());
            json += "}";
        }
        
        if (connection_limiter_) {
            json += ",\"connections\":{";
            json += "\"current\":" + std::to_string(connection_limiter_->currentConnections());
            json += ",\"max\":" + std::to_string(connection_limiter_->maxConnections());
            json += ",\"utilization\":" + std::to_string(connection_limiter_->utilization());
            json += "}";
        }
        
        json += "}";
        return json;
    }
};

// ========== RAII 连接管理器 ==========

class ConnectionGuard {
private:
    RateLimiter& limiter_;
    bool acquired_;
    
public:
    ConnectionGuard(RateLimiter& limiter) : limiter_(limiter), acquired_(false) {
        acquired_ = limiter_.acquireConnection();
    }
    
    ~ConnectionGuard() {
        if (acquired_) {
            limiter_.releaseConnection();
        }
    }
    
    bool acquired() const { return acquired_; }
    
    // 禁止拷贝
    ConnectionGuard(const ConnectionGuard&) = delete;
    ConnectionGuard& operator=(const ConnectionGuard&) = delete;
    
    // 允许移动
    ConnectionGuard(ConnectionGuard&& other) noexcept
        : limiter_(other.limiter_), acquired_(other.acquired_) {
        other.acquired_ = false;
    }
    
    ConnectionGuard& operator=(ConnectionGuard&& other) noexcept {
        if (this != &other) {
            if (acquired_) {
                limiter_.releaseConnection();
            }
            limiter_ = other.limiter_;
            acquired_ = other.acquired_;
            other.acquired_ = false;
        }
        return *this;
    }
};

} // namespace rate
} // namespace uvapi

#endif // RATE_LIMITER_H