/**
 * @file test_response_builder.cpp
 * @brief ResponseBuilder DSL 单元测试
 * 
 * 测试 ResponseBuilder 的核心功能
 */

#include <iostream>
#include <cassert>
#include "../../include/framework.h"

using namespace uvapi;
using namespace uvapi::restful;

int test_count = 0;
int passed_count = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    test_count++; \
    std::cout << "Running " << #name << "... "; \
    test_##name(); \
    passed_count++; \
    std::cout << "PASSED" << std::endl; \
} while(0)

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        std::cerr << "  FAILED: " << #expr << " is false" << std::endl; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        std::cerr << "  FAILED: " << #a << " != " << #b << std::endl; \
        std::cerr << "    Expected: " << (b) << std::endl; \
        std::cerr << "    Actual: " << (a) << std::endl; \
        return; \
    } \
} while(0)

// ========== 数据模型 ==========

struct TestUser {
    int64_t id;
    std::string name;
    
    std::string toJson() const {
        return JSON::Object()
            .set("id", id)
            .set("name", name)
            .toCompactString();
    }
};

// ========== 测试函数 ==========

TEST(ResponseBuilder_BasicOkResponse) {
    ResponseBuilder builder = ResponseBuilder::ok();
    ASSERT_EQ(builder.getStatus(), 200);
}

TEST(ResponseBuilder_CreatedResponse) {
    ResponseBuilder builder = ResponseBuilder::created();
    ASSERT_EQ(builder.getStatus(), 201);
}

TEST(ResponseBuilder_NotFoundResponse) {
    ResponseBuilder builder = ResponseBuilder::notFound();
    ASSERT_EQ(builder.getStatus(), 404);
}

TEST(ResponseBuilder_StatusMethod) {
    ResponseBuilder builder;
    builder.status(201);
    ASSERT_EQ(builder.getStatus(), 201);
}

TEST(ResponseBuilder_MessageMethod) {
    ResponseBuilder builder;
    builder.message("Custom message");
    ASSERT_EQ(builder.getMessage(), "Custom message");
}

TEST(ResponseBuilder_ChainedMethods) {
    ResponseBuilder builder = ResponseBuilder::created()
        .message("User created")
        .requestId("12345");
    
    ASSERT_EQ(builder.getStatus(), 201);
    ASSERT_EQ(builder.getMessage(), "User created");
}

TEST(ResponseBuilder_DataSerialization) {
    TestUser user = {1, "Alice"};
    
    ResponseBuilder builder = ResponseBuilder::ok()
        .data(user);
    
    HttpResponse response = builder;
    ASSERT_EQ(response.status_code, 200);
    ASSERT_TRUE(response.body.find("\"name\":\"Alice\"") != std::string::npos);
}

TEST(ResponseBuilder_ImplicitConversion) {
    TestUser user = {1, "Alice"};
    
    HttpResponse response = ResponseBuilder::ok().data(user);
    
    ASSERT_EQ(response.status_code, 200);
    ASSERT_TRUE(response.body.find("\"name\":\"Alice\"") != std::string::npos);
}

TEST(ResponseBuilder_MakeSuccessResponse) {
    ResponseBuilder builder = makeSuccessResponse();
    ASSERT_EQ(builder.getStatus(), 200);
}

TEST(ResponseBuilder_MakeCreatedResponse) {
    ResponseBuilder builder = makeCreatedResponse();
    ASSERT_EQ(builder.getStatus(), 201);
}

TEST(ResponseBuilder_MakeErrorResponse) {
    ResponseBuilder builder = makeErrorResponse();
    ASSERT_EQ(builder.getStatus(), 400);
}

TEST(ResponseBuilder_MakeNotFoundResponse) {
    ResponseBuilder builder = makeNotFoundResponse();
    ASSERT_EQ(builder.getStatus(), 404);
}

TEST(ResponseBuilder_CompleteWorkflow) {
    TestUser user = {1, "Alice"};
    
    HttpResponse response = ResponseBuilder::created()
        .message("User created")
        .requestId("12345")
        .data(user);
    
    ASSERT_EQ(response.status_code, 201);
    ASSERT_TRUE(response.body.find("\"name\":\"Alice\"") != std::string::npos);
}

// ========== 主函数 ==========

int main() {
    std::cout << "=== ResponseBuilder DSL 单元测试 ===" << std::endl << std::endl;
    
    RUN_TEST(ResponseBuilder_BasicOkResponse);
    RUN_TEST(ResponseBuilder_CreatedResponse);
    RUN_TEST(ResponseBuilder_NotFoundResponse);
    RUN_TEST(ResponseBuilder_StatusMethod);
    RUN_TEST(ResponseBuilder_MessageMethod);
    RUN_TEST(ResponseBuilder_ChainedMethods);
    RUN_TEST(ResponseBuilder_DataSerialization);
    RUN_TEST(ResponseBuilder_ImplicitConversion);
    RUN_TEST(ResponseBuilder_MakeSuccessResponse);
    RUN_TEST(ResponseBuilder_MakeCreatedResponse);
    RUN_TEST(ResponseBuilder_MakeErrorResponse);
    RUN_TEST(ResponseBuilder_MakeNotFoundResponse);
    RUN_TEST(ResponseBuilder_CompleteWorkflow);
    
    std::cout << std::endl;
    std::cout << "=== 测试结果 ===" << std::endl;
    std::cout << "总测试数: " << test_count << std::endl;
    std::cout << "通过: " << passed_count << std::endl;
    std::cout << "失败: " << (test_count - passed_count) << std::endl;
    
    if (passed_count == test_count) {
        std::cout << std::endl << "✓ 所有测试通过！" << std::endl;
        return 0;
    } else {
        std::cout << std::endl << "✗ 有测试失败！" << std::endl;
        return 1;
    }
}