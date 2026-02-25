#include "framework.h"
#include "response_cache.h"
#include <iostream>

using namespace uvapi;
using namespace uvapi::restful;

/**
 * @brief 缓存使用示例
 * 
 * 本示例展示如何安全地使用 ResponseCache
 */

// 全局缓存实例（仅用于示例）
static uvapi::StringResponseCache g_config_cache(50, std::chrono::milliseconds(3600000)); // 1小时 TTL
static uvapi::StringResponseCache g_status_cache(100, std::chrono::milliseconds(300000));  // 5分钟 TTL

/**
 * @brief 获取系统配置（可以安全缓存）
 * 
 * ✅ 可以缓存的原因：
 * - 配置很少变化
 * - 所有用户看到的配置相同
 * - 配置更新频率低
 * - 即使返回旧配置，影响也很小
 */
std::string getSystemConfig() {
    std::string cached_config;
    
    // 尝试从缓存获取
    if (g_config_cache.get("system_config", cached_config)) {
        return cached_config;
    }
    
    // 缓存未命中，生成新配置
    std::string config = JSON::Object()
        .set("version", "1.0.0")
        .set("maintenance", false)
        .set("max_users", 1000)
        .toString();
    
    // 存入缓存
    g_config_cache.put("system_config", std::move(config));
    
    return config;
}

/**
 * @brief 获取用户状态（不能缓存！）
 * 
 * ❌ 不能缓存的原因：
 * - 每个用户的状态不同
 * - 状态可能频繁变化
 * - 需要实时准确性
 * - 包含用户特定信息
 */
std::string getUserStatus(int64_t user_id) {
    // 直接查询数据库或实时数据源
    // 不要使用缓存！
    return JSON::Object()
        .set("user_id", user_id)
        .set("status", "online")
        .set("last_seen", 1234567890)
        .toString();
}

/**
 * @brief 获取公共字典数据（可以缓存）
 * 
 * ✅ 可以缓存的原因：
 * - 字典数据很少变化
 * - 所有用户看到的字典相同
 * - 数据量小，适合缓存
 */
std::string getStatusCodes() {
    std::string cached_codes;
    
    if (g_status_cache.get("status_codes", cached_codes)) {
        return cached_codes;
    }
    
    // 生成状态码字典
    auto array = JSON::Array()
        .append(JSON::Object().set("code", 0).set("name", "active"))
        .append(JSON::Object().set("code", 1).set("name", "inactive"))
        .append(JSON::Object().set("code", 2).set("name", "pending"));
    
    std::string result = array.toString();
    g_status_cache.put("status_codes", std::move(result));
    
    return result;
}

/**
 * @brief 配置更新时清理缓存
 * 
 * 重要：当配置数据更新时，必须清理相关缓存
 */
void updateSystemConfig() {
    // 1. 更新数据库中的配置
    // ...
    
    // 2. 清理缓存
    g_config_cache.remove("system_config");
    
    // 或者清空所有配置缓存
    // g_config_cache.clear();
}

/**
 * @brief 定期清理过期缓存
 * 
 * 建议在后台线程中定期调用
 */
void periodicCacheCleanup() {
    static auto last_cleanup = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - last_cleanup);
    
    if (elapsed.count() >= 5) {  // 每5分钟清理一次
        g_config_cache.cleanup();
        g_status_cache.cleanup();
        last_cleanup = now;
    }
}

int main() {
    std::cout << "=== ResponseCache 使用示例 ===" << std::endl << std::endl;
    
    // 示例 1: 获取系统配置（使用缓存）
    std::cout << "1. 获取系统配置（使用缓存）:" << std::endl;
    std::cout << getSystemConfig() << std::endl << std::endl;
    
    // 示例 2: 获取用户状态（不使用缓存）
    std::cout << "2. 获取用户状态（不使用缓存）:" << std::endl;
    std::cout << getUserStatus(123) << std::endl << std::endl;
    
    // 示例 3: 获取状态码字典（使用缓存）
    std::cout << "3. 获取状态码字典（使用缓存）:" << std::endl;
    std::cout << getStatusCodes() << std::endl << std::endl;
    
    // 示例 4: 显示缓存统计
    std::cout << "4. 缓存统计:" << std::endl;
    std::cout << "   配置缓存大小: " << g_config_cache.size() << std::endl;
    std::cout << "   状态缓存大小: " << g_status_cache.size() << std::endl;
    std::cout << "   配置缓存命中率: " << (g_config_cache.getHitRate() * 100) << "%" << std::endl;
    
    // 示例 5: 配置更新时清理缓存
    std::cout << "\n5. 模拟配置更新（清理缓存）:" << std::endl;
    updateSystemConfig();
    std::cout << "   配置已更新，缓存已清理" << std::endl;
    std::cout << "   配置缓存大小: " << g_config_cache.size() << std::endl;
    
    return 0;
}

/**
 * 缓存使用最佳实践总结：
 * 
 * ✅ 可以缓存的内容：
 * - 系统配置（很少变化）
 * - 字典数据（枚举值、状态码等）
 * - 公共资源列表（可以接受短暂延迟）
 * - 统计数据（非实时要求）
 * 
 * ❌ 不能缓存的内容：
 * - 用户个人信息
 * - 实时数据（在线状态、实时价格等）
 * - 需要认证的内容
 * - 包含时间戳的内容
 * - 每次请求都不同的内容
 * 
 * ⚠️ 使用注意事项：
 * 1. 缓存更新时必须清理相关缓存
 * 2. 定期清理过期缓存以释放内存
 * 3. 监控缓存命中率，调整缓存策略
 * 4. 多实例部署时，缓存不会自动同步
 * 5. 设置合理的 TTL，避免返回过期数据
 */