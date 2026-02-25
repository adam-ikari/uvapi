#ifndef UVAPI_RESPONSE_CACHE_H
#define UVAPI_RESPONSE_CACHE_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <functional>

namespace uvapi {

/**
 * @brief 简单的响应缓存（谨慎使用）
 * 
 * ⚠️ 重要警告：
 * 1. 仅缓存**静态内容**或**很少变化**的数据
 * 2. 不要缓存用户特定的响应（包含用户ID、权限等）
 * 3. 不要缓存动态生成的响应（如实时数据）
 * 4. 确保在数据变化时主动清理缓存
 * 5. 多实例部署时，缓存不会自动同步
 * 
 * 适用场景：
 * - 配置信息（系统配置、功能开关等）
 * - 字典数据（枚举值、状态码等）
 * - 统计数据（可以接受短暂延迟的）
 * - 公共资源（静态数据列表等）
 * 
 * 不适用场景：
 * - 用户个人信息
 * - 实时数据（股票价格、在线状态等）
 * - 需要实时一致性的数据
 * - 包含时间戳或随机数的内容
 */
template<typename KeyType>
class ResponseCache {
public:
    struct CacheEntry {
        std::string response;
        std::chrono::steady_clock::time_point timestamp;
        
        CacheEntry() {}
        
        explicit CacheEntry(const std::string& resp)
            : response(resp)
            , timestamp(std::chrono::steady_clock::now()) {}
        
        CacheEntry(std::string&& resp)
            : response(std::move(resp))
            , timestamp(std::chrono::steady_clock::now()) {}
    };
    
private:
    mutable std::mutex mutex_;
    std::unordered_map<KeyType, CacheEntry> cache_;
    size_t max_size_;
    std::chrono::milliseconds ttl_;
    
public:
    /**
     * @brief 构造响应缓存
     * @param max_size 最大缓存条目数（默认 100）
     * @param ttl 缓存过期时间（默认 60 秒）
     * 
     * 建议：
     * - 静态配置：ttl 可以设置较长（如 3600 秒）
     * - 半静态数据：ttl 设置中等（如 300 秒）
     * - 动态数据：设置很短或不使用缓存
     */
    ResponseCache(size_t max_size = 100, std::chrono::milliseconds ttl = std::chrono::milliseconds(60000))
        : max_size_(max_size)
        , ttl_(ttl) {}
    
    // 获取缓存的响应
    /**
     * @brief 获取缓存的响应
     * @param key 缓存键
     * @param out_response 输出缓存的响应
     * @return 是否找到有效缓存
     * 
     * 注意：此方法会自动检查并删除过期的缓存条目
     */
    bool get(const KeyType& key, std::string& out_response) const {
        std::lock_guard<std::mutex> lock(mutex_);
        total_requests_++;
        
        auto it = cache_.find(key);
        if (it == cache_.end()) {
            return false;
        }
        
        // 检查是否过期
        auto now = std::chrono::steady_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.timestamp);
        if (age > ttl_) {
            // 过期条目，删除它（需要使用 mutable）
            auto mutable_this = const_cast<ResponseCache*>(this);
            mutable_this->cache_.erase(it);
            return false;
        }
        
        hits_++;
        out_response = it->second.response;
        return true;
    }
    
    // 缓存响应
    /**
     * @brief 缓存响应
     * @param key 缓存键
     * @param response 响应内容
     * 
     * 注意：如果缓存已满，会删除最旧的条目
     */
    void put(const KeyType& key, const std::string& response) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 如果缓存已满，删除最旧的条目（简单策略）
        if (cache_.size() >= max_size_) {
            evictOldest();
        }
        
        cache_[key] = CacheEntry(response);
    }
    
    // 移动版本的 put（更高效）
    void put(const KeyType& key, std::string&& response) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 如果缓存已满，删除最旧的条目（简单策略）
        if (cache_.size() >= max_size_) {
            evictOldest();
        }
        
        cache_[key] = CacheEntry(std::move(response));
    }
    
    // 清空缓存
    /**
     * @brief 清空所有缓存
     * 
     * 使用场景：
     * - 配置更新后
     * - 数据批量刷新后
     * - 内存压力过大时
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.clear();
    }
    
    /**
     * @brief 删除指定键的缓存
     * @param key 要删除的缓存键
     * @return 是否成功删除
     * 
     * 使用场景：
     * - 单条数据更新时
     * - 精确控制缓存失效
     */
    bool remove(const KeyType& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_.erase(key) > 0;
    }
    
    // 获取缓存大小
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_.size();
    }
    
    // 设置最大大小
    /**
     * @brief 设置最大缓存大小
     * @param max_size 新的最大大小
     * 
     * 注意：如果当前大小超过新的最大大小，会删除最旧的条目
     */
    void setMaxSize(size_t max_size) {
        std::lock_guard<std::mutex> lock(mutex_);
        max_size_ = max_size;
        
        // 如果当前大小超过新的最大大小，删除最旧的条目
        while (cache_.size() > max_size_) {
            evictOldest();
        }
    }
    
    /**
     * @brief 设置缓存过期时间
     * @param ttl 新的过期时间
     * 
     * 注意：不会立即清理过期条目，只在下次访问时清理
     */
    void setTtl(std::chrono::milliseconds ttl) {
        std::lock_guard<std::mutex> lock(mutex_);
        ttl_ = ttl;
    }
    
    // 清理过期条目
    /**
     * @brief 清理所有过期的缓存条目
     * 
     * 建议定期调用（如每分钟一次）以释放内存
     */
    void cleanup() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::steady_clock::now();
        auto it = cache_.begin();
        while (it != cache_.end()) {
            auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.timestamp);
            if (age > ttl_) {
                it = cache_.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    /**
     * @brief 获取缓存命中率
     * @return 命中率（0.0 - 1.0）
     * 
     * 注意：需要手动记录命中和未命中次数
     */
    double getHitRate() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (total_requests_ == 0) return 0.0;
        return static_cast<double>(hits_) / total_requests_;
    }
    
    /**
     * @brief 重置统计信息
     */
    void resetStats() {
        std::lock_guard<std::mutex> lock(mutex_);
        hits_ = 0;
        total_requests_ = 0;
    }

private:
    /**
     * @brief 淘汰最旧的缓存条目
     */
    void evictOldest() {
        if (cache_.empty()) return;
        
        auto oldest = cache_.begin();
        for (auto it = cache_.begin(); it != cache_.end(); ++it) {
            if (it->second.timestamp < oldest->second.timestamp) {
                oldest = it;
            }
        }
        cache_.erase(oldest);
    }
    
    // 统计信息
    mutable uint64_t hits_ = 0;
    mutable uint64_t total_requests_ = 0;
};

// 特化：使用字符串键的响应缓存
using StringResponseCache = ResponseCache<std::string>;

} 

#endif