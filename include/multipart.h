/**
 * @file multipart.h
 * @brief 多部分表单解析和文件上传支持
 * 
 * 支持 multipart/form-data 解析和文件上传，包含安全限制
 */

#ifndef MULTIPART_H
#define MULTIPART_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <set>

namespace uvapi {

// ========== 上传的文件信息 ==========

struct UploadedFile {
    std::string field_name;      // 表单字段名
    std::string filename;        // 原始文件名
    std::string content_type;    // 内容类型（MIME type）
    std::vector<char> data;      // 文件数据
    size_t size;                 // 文件大小
    
    UploadedFile() : size(0) {}
    
    // 保存文件到磁盘
    bool saveTo(const std::string& filepath) const {
        FILE* fp = fopen(filepath.c_str(), "wb");
        if (!fp) return false;
        
        size_t written = fwrite(data.data(), 1, data.size(), fp);
        fclose(fp);
        
        return written == data.size();
    }
};

// ========== 文件上传配置 ==========

struct UploadConfig {
    size_t max_file_size;                    // 最大文件大小（字节）
    size_t max_total_size;                   // 最大总上传大小（字节）
    std::set<std::string> allowed_types;    // 允许的文件类型（MIME type）
    bool check_file_extension;               // 是否检查文件扩展名
    
    // 默认配置
    static UploadConfig defaultConfig() {
        UploadConfig config;
        config.max_file_size = 10 * 1024 * 1024;  // 10MB
        config.max_total_size = 50 * 1024 * 1024; // 50MB
        config.allowed_types = {
            "image/jpeg", "image/png", "image/gif",
            "text/plain", "application/json",
            "application/pdf"
        };
        config.check_file_extension = true;
        return config;
    }
    
    UploadConfig() 
        : max_file_size(10 * 1024 * 1024)
        , max_total_size(50 * 1024 * 1024)
        , check_file_extension(true) {}
};

// ========== 文件上传验证结果 ==========

struct UploadValidationResult {
    bool valid;
    std::string error_message;
    
    static UploadValidationResult ok() {
        return UploadValidationResult{true, ""};
    }
    
    static UploadValidationResult fail(const std::string& error) {
        return UploadValidationResult{false, error};
    }
};

// ========== 多部分表单解析器 ==========

class MultipartParser {
public:
    // 解析回调函数类型
    using FieldCallback = std::function<void(const std::string& name, const std::string& value)>;
    using FileCallback = std::function<void(const std::string& name, const std::string& filename, 
                                            const std::string& content_type, const std::vector<char>& data)>;
    
    MultipartParser(const std::string& boundary);
    MultipartParser(const std::string& boundary, const UploadConfig& config);
    ~MultipartParser();
    
    // 解析多部分表单数据
    bool parse(const char* data, size_t size);
    
    // 设置字段回调
    void onField(FieldCallback callback) { field_callback_ = callback; }
    
    // 设置文件回调
    void onFile(FileCallback callback) { file_callback_ = callback; }
    
    // 获取解析的字段
    const std::map<std::string, std::string>& getFields() const { return fields_; }
    
    // 获取解析的文件
    const std::map<std::string, UploadedFile>& getFiles() const { return files_; }
    
    // 获取上传配置
    const UploadConfig& getConfig() const { return config_; }
    
    // 设置上传配置
    void setConfig(const UploadConfig& config) { config_ = config; }
    
private:
    std::string boundary_;
    std::map<std::string, std::string> fields_;
    std::map<std::string, UploadedFile> files_;
    UploadConfig config_;
    size_t total_uploaded_size_;  // 已上传的总大小
    
    FieldCallback field_callback_;
    FileCallback file_callback_;
    
    // 解析辅助方法
    bool parsePart(const std::string& part);
    std::string extractHeaderValue(const std::string& headers, const std::string& name);
    std::string extractFilename(const std::string& content_disposition);
    
    // 验证辅助方法
    UploadValidationResult validateFile(const std::string& filename, 
                                        const std::string& content_type,
                                        size_t size);
    bool isAllowedExtension(const std::string& filename);
};

// ========== 多部分表单解析辅助函数 ==========

class MultipartHelper {
public:
    // 从请求中提取 boundary
    static std::string extractBoundary(const std::string& content_type);
    
    // 解析多部分表单请求（使用默认配置）
    static bool parseMultipart(const std::string& content_type, 
                               const char* body, size_t body_size,
                               MultipartParser::FieldCallback field_cb,
                               MultipartParser::FileCallback file_cb);
    
    // 解析多部分表单请求（使用自定义配置）
    static bool parseMultipart(const std::string& content_type,
                               const char* body, size_t body_size,
                               MultipartParser::FieldCallback field_cb,
                               MultipartParser::FileCallback file_cb,
                               const UploadConfig& config);
    
    // 简化版解析：直接返回字段和文件（使用默认配置）
    static bool parseMultipart(const std::string& content_type,
                               const char* body, size_t body_size,
                               std::map<std::string, std::string>& fields,
                               std::map<std::string, UploadedFile>& files);
    
    // 简化版解析：直接返回字段和文件（使用自定义配置）
    static bool parseMultipart(const std::string& content_type,
                               const char* body, size_t body_size,
                               std::map<std::string, std::string>& fields,
                               std::map<std::string, UploadedFile>& files,
                               const UploadConfig& config);
};

} // namespace uvapi

#endif // MULTIPART_H