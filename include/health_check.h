/**
 * @file health_check.h
 * @brief 健康检查功能
 * 
 * 提供健康检查端点，用于生产环境监控
 */

#ifndef HEALTH_CHECK_H
#define HEALTH_CHECK_H

#include <string>
#include <map>
#include <functional>
#include <chrono>
#include <ctime>

namespace uvapi {
namespace health {

/**
 * @brief 健康状态
 */
enum class HealthStatus {
    HEALTHY,    // 健康
    UNHEALTHY,  // 不健康
    DEGRADED    // 降级
};

/**
 * @brief 健康检查结果
 */
struct HealthCheckResult {
    HealthStatus status;
    std::string message;
    std::map<std::string, std::string> details;
    std::chrono::system_clock::time_point timestamp;
    
    HealthCheckResult() 
        : status(HealthStatus::HEALTHY)
        , timestamp(std::chrono::system_clock::now()) {}
    
    std::string toJson() const {
        std::string status_str;
        switch (status) {
            case HealthStatus::HEALTHY: status_str = "healthy"; break;
            case HealthStatus::UNHEALTHY: status_str = "unhealthy"; break;
            case HealthStatus::DEGRADED: status_str = "degraded"; break;
        }
        
        std::time_t ts = std::chrono::system_clock::to_time_t(timestamp);
        char time_buf[64];
        std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&ts));
        
        std::string json = "{";
        json += "\"status\":\"" + status_str + "\",";
        json += "\"message\":\"" + message + "\",";
        json += "\"timestamp\":\"" + std::string(time_buf) + "\",";
        json += "\"details\":{";
        
        bool first = true;
        for (const auto& [key, value] : details) {
            if (!first) json += ",";
            json += "\"" + key + "\":\"" + value + "\"";
            first = false;
        }
        
        json += "}}";
        return json;
    }
};

/**
 * @brief 健康检查器函数类型
 */
using HealthCheckFunction = std::function<HealthCheckResult()>;

/**
 * @brief 健康检查器
 */
class HealthChecker {
private:
    std::string name_;
    HealthCheckFunction check_function_;
    
public:
    HealthChecker(const std::string& name, HealthCheckFunction func)
        : name_(name), check_function_(func) {}
    
    HealthCheckResult check() const {
        if (check_function_) {
            return check_function_();
        }
        HealthCheckResult result;
        result.status = HealthStatus::UNHEALTHY;
        result.message = "Check function not defined";
        return result;
    }
    
    const std::string& name() const { return name_; }
};

/**
 * @brief 健康检查管理器
 */
class HealthCheckManager {
private:
    std::vector<HealthChecker> checkers_;
    
public:
    HealthCheckManager() {}
    
    // 添加健康检查器
    void addChecker(const HealthChecker& checker) {
        checkers_.push_back(checker);
    }
    
    // 执行所有健康检查
    std::string checkAll() const {
        std::map<std::string, HealthCheckResult> results;
        HealthStatus overall_status = HealthStatus::HEALTHY;
        
        for (const auto& checker : checkers_) {
            HealthCheckResult result = checker.check();
            results[checker.name()] = result;
            
            if (result.status == HealthStatus::UNHEALTHY) {
                overall_status = HealthStatus::UNHEALTHY;
            } else if (result.status == HealthStatus::DEGRADED && overall_status == HealthStatus::HEALTHY) {
                overall_status = HealthStatus::DEGRADED;
            }
        }
        
        std::string json = "{";
        json += "\"status\":\"" + statusToString(overall_status) + "\",";
        json += "\"checks\":{";
        
        bool first = true;
        for (const auto& [name, result] : results) {
            if (!first) json += ",";
            json += "\"" + name + "\":" + result.toJson();
            first = false;
        }
        
        json += "}}";
        return json;
    }
    
private:
    static std::string statusToString(HealthStatus status) {
        switch (status) {
            case HealthStatus::HEALTHY: return "healthy";
            case HealthStatus::UNHEALTHY: return "unhealthy";
            case HealthStatus::DEGRADED: return "degraded";
            default: return "unknown";
        }
    }
};

// ========== 预定义的健康检查器 ==========

/**
 * @brief 内存使用检查器
 */
inline HealthChecker memoryChecker(const std::string& name = "memory") {
    return HealthChecker(name, []() -> HealthCheckResult {
        HealthCheckResult result;
        
        // 读取内存使用情况（Linux）
        std::ifstream statm("/proc/self/statm");
        if (statm.is_open()) {
            unsigned long size, resident, share, text, lib, data, dt;
            statm >> size >> resident >> share >> text >> lib >> data >> dt;
            
            unsigned long pagesize = sysconf(_SC_PAGESIZE);
            unsigned long memory_mb = (resident * pagesize) / (1024 * 1024);
            
            result.details["memory_mb"] = std::to_string(memory_mb);
            result.details["memory_kb"] = std::to_string((resident * pagesize) / 1024);
            
            if (memory_mb > 1024) {
                result.status = HealthStatus::DEGRADED;
                result.message = "Memory usage high";
            } else {
                result.status = HealthStatus::HEALTHY;
                result.message = "Memory usage normal";
            }
        } else {
            result.status = HealthStatus::DEGRADED;
            result.message = "Cannot read memory usage";
        }
        
        return result;
    });
}

/**
 * @brief 磁盘空间检查器
 */
inline HealthChecker diskChecker(const std::string& path = "/", const std::string& name = "disk") {
    return HealthChecker(name, [path]() -> HealthCheckResult {
        HealthCheckResult result;
        
        struct statvfs stat;
        if (statvfs(path.c_str(), &stat) == 0) {
            unsigned long total = stat.f_blocks * stat.f_frsize / (1024 * 1024);
            unsigned long free = stat.f_bfree * stat.f_frsize / (1024 * 1024);
            unsigned long used = total - free;
            double usage_percent = (double)used / total * 100;
            
            result.details["total_mb"] = std::to_string(total);
            result.details["free_mb"] = std::to_string(free);
            result.details["used_mb"] = std::to_string(used);
            result.details["usage_percent"] = std::to_string(usage_percent);
            
            if (usage_percent > 90) {
                result.status = HealthStatus::UNHEALTHY;
                result.message = "Disk space critically low";
            } else if (usage_percent > 80) {
                result.status = HealthStatus::DEGRADED;
                result.message = "Disk space low";
            } else {
                result.status = HealthStatus::HEALTHY;
                result.message = "Disk space normal";
            }
        } else {
            result.status = HealthStatus::UNHEALTHY;
            result.message = "Cannot check disk space";
        }
        
        return result;
    });
}

/**
 * @brief 简单的存活检查器（总是返回健康）
 */
inline HealthChecker livenessChecker(const std::string& name = "liveness") {
    return HealthChecker(name, []() -> HealthCheckResult {
        HealthCheckResult result;
        result.status = HealthStatus::HEALTHY;
        result.message = "Service is alive";
        return result;
    });
}

} // namespace health
} // namespace uvapi

#endif // HEALTH_CHECK_H