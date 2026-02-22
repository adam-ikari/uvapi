/**
 * @file metrics.h
 * @brief 监控指标功能
 * 
 * 提供监控指标收集和导出功能，用于生产环境监控
 */

#ifndef METRICS_H
#define METRICS_H

#include <string>
#include <map>
#include <atomic>
#include <mutex>
#include <chrono>
#include <vector>
#include <sstream>

namespace uvapi {
namespace metrics {

/**
 * @brief 指标类型
 */
enum class MetricType {
    COUNTER,    // 计数器（只增不减）
    GAUGE,      // 仪表盘（可增可减）
    HISTOGRAM   // 直方图（分布统计）
};

/**
 * @brief 指标标签
 */
using MetricLabels = std::map<std::string, std::string>;

/**
 * @brief 基础指标
 */
class Metric {
protected:
    std::string name_;
    std::string help_;
    MetricType type_;
    MetricLabels labels_;
    mutable std::mutex mutex_;
    
public:
    Metric(const std::string& name, const std::string& help, MetricType type)
        : name_(name), help_(help), type_(type) {}
    
    virtual ~Metric() = default;
    
    const std::string& name() const { return name_; }
    const std::string& help() const { return help_; }
    MetricType type() const { return type_; }
    
    void addLabel(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        labels_[key] = value;
    }
    
    virtual std::string toPrometheus() const = 0;
};

/**
 * @brief 计数器指标
 */
class Counter : public Metric {
private:
    std::atomic<uint64_t> value_;
    
public:
    Counter(const std::string& name, const std::string& help = "")
        : Metric(name, help, MetricType::COUNTER), value_(0) {}
    
    void increment(uint64_t delta = 1) {
        value_ += delta;
    }
    
    uint64_t value() const {
        return value_.load();
    }
    
    void reset() {
        value_ = 0;
    }
    
    std::string toPrometheus() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ostringstream oss;
        
        oss << "# HELP " << name_ << " " << help_ << "\n";
        oss << "# TYPE " << name_ << " counter\n";
        
        if (!labels_.empty()) {
            oss << name_ << "{";
            bool first = true;
            for (const auto& [key, value] : labels_) {
                if (!first) oss << ",";
                oss << key << "=\"" << value << "\"";
                first = false;
            }
            oss << "} " << value_ << "\n";
        } else {
            oss << name_ << " " << value_ << "\n";
        }
        
        return oss.str();
    }
};

/**
 * @brief 仪表盘指标
 */
class Gauge : public Metric {
private:
    std::atomic<double> value_;
    
public:
    Gauge(const std::string& name, const std::string& help = "")
        : Metric(name, help, MetricType::GAUGE), value_(0) {}
    
    void set(double value) {
        value_ = value;
    }
    
    void increment(double delta = 1.0) {
        value_ += delta;
    }
    
    void decrement(double delta = 1.0) {
        value_ -= delta;
    }
    
    double value() const {
        return value_.load();
    }
    
    std::string toPrometheus() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ostringstream oss;
        
        oss << "# HELP " << name_ << " " << help_ << "\n";
        oss << "# TYPE " << name_ << " gauge\n";
        
        if (!labels_.empty()) {
            oss << name_ << "{";
            bool first = true;
            for (const auto& [key, value] : labels_) {
                if (!first) oss << ",";
                oss << key << "=\"" << value << "\"";
                first = false;
            }
            oss << "} " << value_ << "\n";
        } else {
            oss << name_ << " " << value_ << "\n";
        }
        
        return oss.str();
    }
};

/**
 * @brief 直方图指标（简化版）
 */
class Histogram : public Metric {
private:
    std::vector<uint64_t> buckets_;
    std::vector<double> bucket_boundaries_;
    std::atomic<uint64_t> sum_;
    std::atomic<uint64_t> count_;
    
public:
    Histogram(const std::string& name, const std::string& help = "",
              const std::vector<double>& boundaries = {0.1, 0.5, 1.0, 5.0, 10.0})
        : Metric(name, help, MetricType::HISTOGRAM)
        , bucket_boundaries_(boundaries)
        , sum_(0)
        , count_(0) {
        buckets_.resize(boundaries.size() + 1, 0);
    }
    
    void observe(double value) {
        sum_ += static_cast<uint64_t>(value);
        count_++;
        
        // 找到合适的 bucket
        size_t bucket_index = buckets_.size() - 1;  // 默认到最后一个
        for (size_t i = 0; i < bucket_boundaries_.size(); ++i) {
            if (value <= bucket_boundaries_[i]) {
                bucket_index = i;
                break;
            }
        }
        
        buckets_[bucket_index]++;
    }
    
    std::string toPrometheus() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ostringstream oss;
        
        oss << "# HELP " << name_ << " " << help_ << "\n";
        oss << "# TYPE " << name_ << " histogram\n";
        
        // 输出 bucket 指标
        for (size_t i = 0; i < bucket_boundaries_.size(); ++i) {
            uint64_t cumulative = 0;
            for (size_t j = 0; j <= i; ++j) {
                cumulative += buckets_[j];
            }
            
            oss << name_ << "_bucket{le=\"" << bucket_boundaries_[i] << "\"} " << cumulative << "\n";
        }
        
        // 输出 +Inf bucket（总数）
        oss << name_ << "_bucket{le=\"+Inf\"} " << count_ << "\n";
        
        // 输出 sum 和 count
        oss << name_ << "_sum " << sum_ << "\n";
        oss << name_ << "_count " << count_ << "\n";
        
        return oss.str();
    }
};

/**
 * @brief 指标注册表
 */
class MetricRegistry {
private:
    std::map<std::string, std::shared_ptr<Metric>> metrics_;
    mutable std::mutex mutex_;
    
public:
    MetricRegistry() {}
    
    // 注册计数器
    std::shared_ptr<Counter> registerCounter(const std::string& name,
                                               const std::string& help = "") {
        std::lock_guard<std::mutex> lock(mutex_);
        auto counter = std::make_shared<Counter>(name, help);
        metrics_[name] = counter;
        return counter;
    }
    
    // 注册仪表盘
    std::shared_ptr<Gauge> registerGauge(const std::string& name,
                                           const std::string& help = "") {
        std::lock_guard<std::mutex> lock(mutex_);
        auto gauge = std::make_shared<Gauge>(name, help);
        metrics_[name] = gauge;
        return gauge;
    }
    
    // 注册直方图
    std::shared_ptr<Histogram> registerHistogram(const std::string& name,
                                                   const std::string& help = "",
                                                   const std::vector<double>& boundaries = {0.1, 0.5, 1.0, 5.0, 10.0}) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto histogram = std::make_shared<Histogram>(name, help, boundaries);
        metrics_[name] = histogram;
        return histogram;
    }
    
    // 获取指标
    std::shared_ptr<Metric> getMetric(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = metrics_.find(name);
        if (it != metrics_.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    // 导出为 Prometheus 格式
    std::string toPrometheus() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ostringstream oss;
        
        for (const auto& [name, metric] : metrics_) {
            oss << metric->toPrometheus() << "\n";
        }
        
        return oss.str();
    }
};

// ========== 全局指标注册表 ==========

inline MetricRegistry& getGlobalMetricRegistry() {
    static MetricRegistry registry;
    return registry;
}

// ========== 预定义的指标 ==========

/**
 * @brief HTTP 请求计数器
 */
inline std::shared_ptr<Counter> httpRequestsTotal(const std::string& method = "GET",
                                                     const std::string& path = "/") {
    auto& registry = getGlobalMetricRegistry();
    std::string name = "http_requests_total";
    
    auto counter = std::dynamic_pointer_cast<Counter>(registry.getMetric(name));
    if (!counter) {
        counter = registry.registerCounter(name, "Total number of HTTP requests");
    }
    
    counter->addLabel("method", method);
    counter->addLabel("path", path);
    
    return counter;
}

/**
 * @brief HTTP 请求延迟直方图
 */
inline std::shared_ptr<Histogram> httpRequestDuration(const std::string& method = "GET",
                                                       const std::string& path = "/") {
    auto& registry = getGlobalMetricRegistry();
    std::string name = "http_request_duration_seconds";
    
    auto histogram = std::dynamic_pointer_cast<Histogram>(registry.getMetric(name));
    if (!histogram) {
        histogram = registry.registerHistogram(name, "HTTP request duration in seconds");
    }
    
    histogram->addLabel("method", method);
    histogram->addLabel("path", path);
    
    return histogram;
}

/**
 * @brief 当前连接数仪表盘
 */
inline std::shared_ptr<Gauge> currentConnections() {
    auto& registry = getGlobalMetricRegistry();
    std::string name = "current_connections";
    
    auto gauge = std::dynamic_pointer_cast<Gauge>(registry.getMetric(name));
    if (!gauge) {
        gauge = registry.registerGauge(name, "Current number of connections");
    }
    
    return gauge;
}

} // namespace metrics
} // namespace uvapi

#endif // METRICS_H
