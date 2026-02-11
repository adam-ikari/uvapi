/**
 * @file upload_example.cpp
 * @brief 文件上传示例
 * 
 * 演示如何使用 multipart 表单解析和文件上传功能
 */

#include <iostream>
#include "framework.h"
#include "multipart.h"

using namespace uvapi;

// 文件上传处理器
Response handleUpload(const HttpRequest& req) {
    // 检查 Content-Type
    auto content_type_it = req.headers.find("Content-Type");
    if (content_type_it == req.headers.end()) {
        return badRequest("Missing Content-Type header");
    }
    
    const std::string& content_type = content_type_it->second;
    
    // 检查是否为 multipart/form-data
    if (content_type.find("multipart/form-data") == std::string::npos) {
        return badRequest("Content-Type must be multipart/form-data");
    }
    
    // 解析 multipart 表单
    std::map<std::string, std::string> fields;
    std::map<std::string, UploadedFile> files;
    
    if (!MultipartHelper::parseMultipart(content_type, req.body.c_str(), req.body.size(), fields, files)) {
        return badRequest("Failed to parse multipart form data");
    }
    
    // 打印接收到的字段
    std::cout << "=== Fields ===" << std::endl;
    for (const auto& field : fields) {
        std::cout << field.first << ": " << field.second << std::endl;
    }
    
    // 处理上传的文件
    std::cout << "=== Files ===" << std::endl;
    for (const auto& file_pair : files) {
        const UploadedFile& file = file_pair.second;
        std::cout << "Field: " << file.field_name << std::endl;
        std::cout << "Filename: " << file.filename << std::endl;
        std::cout << "Content-Type: " << file.content_type << std::endl;
        std::cout << "Size: " << file.size << " bytes" << std::endl;
        
        // 保存文件到 uploads 目录
        std::string save_path = "uploads/" + file.filename;
        if (file.saveTo(save_path)) {
            std::cout << "Saved to: " << save_path << std::endl;
        } else {
            std::cout << "Failed to save file" << std::endl;
        }
    }
    
    // 返回成功响应
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "message", "Upload successful");
    
    cJSON* files_json = cJSON_CreateArray();
    for (const auto& file_pair : files) {
        const UploadedFile& file = file_pair.second;
        cJSON* file_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(file_obj, "filename", file.filename.c_str());
        cJSON_AddStringToObject(file_obj, "content_type", file.content_type.c_str());
        cJSON_AddNumberToObject(file_obj, "size", (double)file.size);
        cJSON_AddItemToArray(files_json, file_obj);
    }
    cJSON_AddItemToObject(root, "files", files_json);
    
    char* json_str = cJSON_Print(root);
    std::string result(json_str);
    free(json_str);
    cJSON_Delete(root);
    
    Response resp(200);
    resp.headers["Content-Type"] = "application/json";
    resp.body = result;
    return resp;
}

// 简单的 HTML 上传页面
Response handleUploadPage(const HttpRequest& req) {
    std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>File Upload</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        h1 { color: #333; }
        form { background: #f5f5f5; padding: 20px; border-radius: 5px; }
        input[type="file"] { margin: 10px 0; }
        input[type="submit"] { background: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; }
        input[type="submit"]:hover { background: #0056b3; }
    </style>
</head>
<body>
    <h1>File Upload Example</h1>
    <form action="/upload" method="post" enctype="multipart/form-data">
        <label for="file">Choose a file:</label><br>
        <input type="file" id="file" name="file" required><br><br>
        <label for="description">Description:</label><br>
        <input type="text" id="description" name="description" placeholder="File description"><br><br>
        <input type="submit" value="Upload">
    </form>
</body>
</html>
    )";
    
    Response resp(200);
    resp.headers["Content-Type"] = "text/html";
    resp.body = html;
    return resp;
}

int main() {
    std::cout << "=== UVAPI File Upload Example ===" << std::endl;
    
    // 创建事件循环
    uv_loop_t* loop = uv_default_loop();
    
    // 创建服务器
    server::Server server(loop);
    
    // 添加路由
    server.addRoute("/", HttpMethod::GET, handleUploadPage);
    server.addRoute("/upload", HttpMethod::POST, handleUpload);
    
    // 启动服务器
    if (!server.listen("0.0.0.0", 8080)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    std::cout << "Server running on http://0.0.0.0:8080" << std::endl;
    std::cout << "Upload page: http://0.0.0.0:8080/" << std::endl;
    
    // 创建 uploads 目录
    system("mkdir -p uploads");
    
    // 运行事件循环
    uv_run(loop, UV_RUN_DEFAULT);
    
    return 0;
}