#ifndef UVAPI_STRING_BUILDER_H
#define UVAPI_STRING_BUILDER_H

#include <string>
#include <sstream>

namespace uvapi {

/**
 * @brief 高效的字符串构建器
 * 
 * 相比直接使用 std::string + 操作符，StringBuilder 可以减少内存分配次数
 */
class StringBuilder {
private:
    std::ostringstream stream_;
    
public:
    StringBuilder() {}
    
    // 追加各种类型
    StringBuilder& append(const std::string& str) {
        stream_ << str;
        return *this;
    }
    
    StringBuilder& append(const char* str) {
        if (str) stream_ << str;
        return *this;
    }
    
    StringBuilder& append(char c) {
        stream_ << c;
        return *this;
    }
    
    StringBuilder& append(int value) {
        stream_ << value;
        return *this;
    }
    
    StringBuilder& append(int64_t value) {
        stream_ << value;
        return *this;
    }
    
    StringBuilder& append(double value) {
        stream_ << value;
        return *this;
    }
    
    StringBuilder& append(bool value) {
        stream_ << (value ? "true" : "false");
        return *this;
    }
    
    // 操作符重载
    StringBuilder& operator<<(const std::string& str) {
        return append(str);
    }
    
    StringBuilder& operator<<(const char* str) {
        return append(str);
    }
    
    StringBuilder& operator<<(char c) {
        return append(c);
    }
    
    StringBuilder& operator<<(int value) {
        return append(value);
    }
    
    StringBuilder& operator<<(int64_t value) {
        return append(value);
    }
    
    StringBuilder& operator<<(double value) {
        return append(value);
    }
    
    StringBuilder& operator<<(bool value) {
        return append(value);
    }
    
    // 构建最终字符串
    std::string toString() const {
        return stream_.str();
    }
    
    // 清空
    void clear() {
        stream_.str("");
        stream_.clear();
    }
};

} 

#endif