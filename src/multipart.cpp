/**
 * @file multipart.cpp
 * @brief 多部分表单解析和文件上传支持实现
 */

#include "multipart.h"
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cctype>

namespace uvapi {

// ========== MultipartParser 实现 ==========

MultipartParser::MultipartParser(const std::string& boundary)
    : boundary_(boundary)
    , config_(UploadConfig::defaultConfig())
    , total_uploaded_size_(0) {
}

MultipartParser::MultipartParser(const std::string& boundary, const UploadConfig& config)
    : boundary_(boundary)
    , config_(config)
    , total_uploaded_size_(0) {
}

MultipartParser::~MultipartParser() {
}

bool MultipartParser::parse(const char* data, size_t size) {
    if (!data || size == 0) return false;
    
    // 检查总大小限制
    if (size > config_.max_total_size) {
        return false;
    }
    
    std::string boundary_line = "--" + boundary_;
    std::string end_boundary = "--" + boundary_ + "--";
    
    std::string content(data, size);
    
    size_t pos = 0;
    
    // 跳过第一个 boundary
    pos = content.find(boundary_line);
    if (pos == std::string::npos) return false;
    pos += boundary_line.length();
    
    // 跳过 CRLF
    if (pos < content.size() && content[pos] == '\r') pos++;
    if (pos < content.size() && content[pos] == '\n') pos++;
    
    while (pos < content.size()) {
        // 查找下一个 boundary
        size_t next_boundary = content.find(boundary_line, pos);
        if (next_boundary == std::string::npos) {
            // 检查是否是结束 boundary
            next_boundary = content.find(end_boundary, pos);
            if (next_boundary == std::string::npos) return false;
        }
        
        // 提取 part 数据
        std::string part = content.substr(pos, next_boundary - pos);
        
        // 移除末尾的 CRLF
        if (!part.empty() && part.back() == '\n') part.pop_back();
        if (!part.empty() && part.back() == '\r') part.pop_back();
        
        // 解析 part
        parsePart(part);
        
        // 移动到下一个 part
        pos = next_boundary + boundary_line.length();
        
        // 检查是否是结束 boundary
        if (pos + 2 <= content.size() && content[pos] == '-' && content[pos + 1] == '-') {
            break;
        }
        
        // 跳过 CRLF
        if (pos < content.size() && content[pos] == '\r') pos++;
        if (pos < content.size() && content[pos] == '\n') pos++;
    }
    
    return true;
}

bool MultipartParser::parsePart(const std::string& part) {
    if (part.empty()) return false;
    
    // 分离头部和主体
    size_t header_end = part.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        header_end = part.find("\n\n");
        if (header_end == std::string::npos) return false;
    }
    
    std::string headers = part.substr(0, header_end);
    std::string body = part.substr(header_end + 4); // +4 跳过 \r\n\r\n
    
    // 检查是否是文件上传
    std::string content_disposition = extractHeaderValue(headers, "Content-Disposition");
    
    if (content_disposition.empty()) return false;
    
    // 提取字段名
    size_t name_pos = content_disposition.find("name=\"");
    if (name_pos == std::string::npos) return false;
    name_pos += 6; // name=" 的长度
    
    size_t name_end = content_disposition.find("\"", name_pos);
    if (name_end == std::string::npos) return false;
    
    std::string field_name = content_disposition.substr(name_pos, name_end - name_pos);
    
    // 检查是否有 filename
    std::string filename = extractFilename(content_disposition);
    std::string content_type = extractHeaderValue(headers, "Content-Type");
    
    if (!filename.empty()) {
        // 文件上传
        UploadedFile file;
        file.field_name = field_name;
        file.filename = filename;
        file.content_type = content_type;
        file.data.assign(body.begin(), body.end());
        file.size = body.size();
        
        // 验证文件
        UploadValidationResult validation = validateFile(filename, content_type, file.size);
        if (!validation.valid) {
            // 跳过无效文件
            return false;
        }
        
        // 更新总上传大小
        total_uploaded_size_ += file.size;
        if (total_uploaded_size_ > config_.max_total_size) {
            return false;
        }
        
        files_[field_name] = file;
        
        if (file_callback_) {
            file_callback_(field_name, filename, content_type, file.data);
        }
    } else {
        // 普通字段
        fields_[field_name] = body;
        
        if (field_callback_) {
            field_callback_(field_name, body);
        }
    }
    
    return true;
}

std::string MultipartParser::extractHeaderValue(const std::string& headers, const std::string& name) {
    size_t pos = headers.find(name + ":");
    if (pos == std::string::npos) return "";
    
    pos += name.length() + 1; // +1 跳过 ":"
    
    // 跳过空格
    while (pos < headers.size() && headers[pos] == ' ') pos++;
    
    size_t end = headers.find("\r\n", pos);
    if (end == std::string::npos) {
        end = headers.find("\n", pos);
    }
    
    if (end == std::string::npos) {
        return headers.substr(pos);
    }
    
    return headers.substr(pos, end - pos);
}

std::string MultipartParser::extractFilename(const std::string& content_disposition) {
    size_t pos = content_disposition.find("filename=\"");
    if (pos == std::string::npos) return "";
    
    pos += 10; // filename=" 的长度
    
    size_t end = content_disposition.find("\"", pos);
    if (end == std::string::npos) return "";
    
    return content_disposition.substr(pos, end - pos);
}

UploadValidationResult MultipartParser::validateFile(const std::string& filename, 
                                                         const std::string& content_type,
                                                         size_t size) {
    // 检查文件大小
    if (size > config_.max_file_size) {
        return UploadValidationResult::fail("File size exceeds maximum limit of " + 
            std::to_string(config_.max_file_size / 1024 / 1024) + "MB");
    }
    
    // 检查文件类型
    if (!config_.allowed_types.empty()) {
        if (config_.allowed_types.find(content_type) == config_.allowed_types.end()) {
            return UploadValidationResult::fail("File type '" + content_type + "' is not allowed");
        }
    }
    
    // 检查文件扩展名
    if (config_.check_file_extension) {
        if (!isAllowedExtension(filename)) {
            return UploadValidationResult::fail("File extension is not allowed");
        }
    }
    
    return UploadValidationResult::ok();
}

bool MultipartParser::isAllowedExtension(const std::string& filename) {
    // 从文件名中提取扩展名
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos == std::string::npos) return false;
    
    std::string extension = filename.substr(dot_pos + 1);
    // 转换为小写
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    // 根据允许的 MIME 类型推断允许的扩展名
    std::set<std::string> allowed_extensions;
    for (const auto& mime_type : config_.allowed_types) {
        if (mime_type == "image/jpeg") {
            allowed_extensions.insert("jpg");
            allowed_extensions.insert("jpeg");
        } else if (mime_type == "image/png") {
            allowed_extensions.insert("png");
        } else if (mime_type == "image/gif") {
            allowed_extensions.insert("gif");
        } else if (mime_type == "text/plain") {
            allowed_extensions.insert("txt");
            allowed_extensions.insert("text");
        } else if (mime_type == "application/json") {
            allowed_extensions.insert("json");
        } else if (mime_type == "application/pdf") {
            allowed_extensions.insert("pdf");
        }
    }
    
    return allowed_extensions.find(extension) != allowed_extensions.end();
}

// ========== MultipartHelper 实现 ==========

std::string MultipartHelper::extractBoundary(const std::string& content_type) {
    size_t pos = content_type.find("boundary=");
    if (pos == std::string::npos) return "";
    
    pos += 9; // boundary= 的长度
    
    // boundary 可能被引号包围
    if (pos < content_type.size() && content_type[pos] == '"') {
        pos++; // 跳过引号
        size_t end = content_type.find("\"", pos);
        if (end == std::string::npos) return "";
        return content_type.substr(pos, end - pos);
    }
    
    // 提取到分号或结尾
    size_t end = content_type.find(";", pos);
    if (end == std::string::npos) {
        return content_type.substr(pos);
    }
    
    return content_type.substr(pos, end - pos);
}

bool MultipartHelper::parseMultipart(const std::string& content_type,
                                     const char* body, size_t body_size,
                                     MultipartParser::FieldCallback field_cb,
                                     MultipartParser::FileCallback file_cb) {
    return parseMultipart(content_type, body, body_size, field_cb, file_cb, 
                        UploadConfig::defaultConfig());
}

bool MultipartHelper::parseMultipart(const std::string& content_type,
                                     const char* body, size_t body_size,
                                     MultipartParser::FieldCallback field_cb,
                                     MultipartParser::FileCallback file_cb,
                                     const UploadConfig& config) {
    std::string boundary = extractBoundary(content_type);
    if (boundary.empty()) return false;
    
    MultipartParser parser(boundary, config);
    
    if (field_cb) parser.onField(field_cb);
    if (file_cb) parser.onFile(file_cb);
    
    return parser.parse(body, body_size);
}

bool MultipartHelper::parseMultipart(const std::string& content_type,
                                     const char* body, size_t body_size,
                                     std::map<std::string, std::string>& fields,
                                     std::map<std::string, UploadedFile>& files) {
    return parseMultipart(content_type, body, body_size, fields, files,
                        UploadConfig::defaultConfig());
}

bool MultipartHelper::parseMultipart(const std::string& content_type,
                                     const char* body, size_t body_size,
                                     std::map<std::string, std::string>& fields,
                                     std::map<std::string, UploadedFile>& files,
                                     const UploadConfig& config) {
    std::string boundary = extractBoundary(content_type);
    if (boundary.empty()) return false;
    
    MultipartParser parser(boundary, config);
    
    if (!parser.parse(body, body_size)) return false;
    
    fields = parser.getFields();
    files = parser.getFiles();
    
    return true;
}

} // namespace uvapi