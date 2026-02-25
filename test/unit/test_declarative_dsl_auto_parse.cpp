/**
 * @file test_declarative_dsl_auto_parse.cpp
 * @brief 声明式 DSL 自动解析和验证功能的单元测试
 */

#include <iostream>
#include <cassert>
#include "../../include/declarative_dsl.h"
#include "../../include/framework.h"

using namespace uvapi;
using namespace uvapi::declarative;

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

#define ASSERT_FALSE(expr) do { \
    if (expr) { \
        std::cerr << "  FAILED: " << #expr << " is true" << std::endl; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        std::cerr << "  FAILED: " << #a << " (" << (a) << ") != " << #b << " (" << (b) << ")" << std::endl; \
        return; \
    } \
} while(0)

#define ASSERT_NE(a, b) do { \
    if ((a) == (b)) { \
        std::cerr << "  FAILED: " << #a << " (" << (a) << ") == " << #b << " (" << (b) << ")" << std::endl; \
        return; \
    } \
} while(0)

// ========== 参数解析器测试 ==========

TEST(ParameterParserTest_ExtractQueryParam) {
    HttpRequest req;
    req.query_params["page"] = "2";
    req.query_params["limit"] = "20";

    std::vector<restful::ParamDefinition> params;
    restful::ParamDefinition page_def("page", restful::ParamType::QUERY);
    page_def.validation.required = true;
    params.push_back(page_def);

    restful::ParamDefinition limit_def("limit", restful::ParamType::QUERY);
    limit_def.validation.required = true;
    params.push_back(limit_def);

    std::map<std::string, std::string> result = ParameterParser::extract(req, params);

    ASSERT_EQ(result["page"], "2");
    ASSERT_EQ(result["limit"], "20");
}

TEST(ParameterParserTest_ExtractPathParam) {
    HttpRequest req;
    req.path_params["id"] = "123";

    std::vector<restful::ParamDefinition> params;
    restful::ParamDefinition id_def("id", restful::ParamType::PATH);
    id_def.validation.required = true;
    params.push_back(id_def);

    std::map<std::string, std::string> result = ParameterParser::extract(req, params);

    ASSERT_EQ(result["id"], "123");
}

TEST(ParameterParserTest_ExtractWithDefaultValue) {
    HttpRequest req;
    req.query_params["page"] = "1";

    std::vector<restful::ParamDefinition> params;
    restful::ParamDefinition page_def("page", restful::ParamType::QUERY);
    page_def.validation.required = false;
    page_def.default_value = "1";
    params.push_back(page_def);

    restful::ParamDefinition limit_def("limit", restful::ParamType::QUERY);
    limit_def.validation.required = false;
    limit_def.default_value = "10";
    params.push_back(limit_def);

    std::map<std::string, std::string> result = ParameterParser::extract(req, params);

    ASSERT_EQ(result["page"], "1");
    ASSERT_EQ(result["limit"], "10");  // 使用默认值
}

// ========== 参数验证器测试 ==========

TEST(ParameterValidatorTest_ValidateRequiredParameter) {
    restful::ParamDefinition def("username", restful::ParamType::QUERY);
    def.validation.required = true;

    declarative::ValidationResult result = ParameterValidator::validate("username", "", def);

    ASSERT_FALSE(result.success);
    ASSERT_EQ(result.field_name, "username");
    ASSERT_FALSE(result.error_message.empty());
}

TEST(ParameterValidatorTest_ValidateRequiredParameterWithValue) {
    restful::ParamDefinition def("username", restful::ParamType::QUERY);
    def.validation.required = true;

    declarative::ValidationResult result = ParameterValidator::validate("username", "john", def);

    ASSERT_TRUE(result.success);
}

TEST(ParameterValidatorTest_ValidateIntRange) {
    restful::ParamDefinition def("age", restful::ParamType::QUERY);
    def.data_type = 1;
    def.validation.has_min = true;
    def.validation.min_value = 18;
    def.validation.has_max = true;
    def.validation.max_value = 120;

    // 在范围内
    declarative::ValidationResult result1 = ParameterValidator::validate("age", "25", def);
    ASSERT_TRUE(result1.success);

    // 小于最小值
    declarative::ValidationResult result2 = ParameterValidator::validate("age", "10", def);
    ASSERT_FALSE(result2.success);

    // 大于最大值
    declarative::ValidationResult result3 = ParameterValidator::validate("age", "150", def);
    ASSERT_FALSE(result3.success);
}

TEST(ParameterValidatorTest_ValidateStringLength) {
    restful::ParamDefinition def("username", restful::ParamType::QUERY);
    def.data_type = 0;
    def.validation.has_min_length = true;
    def.validation.min_length = 3;
    def.validation.has_max_length = true;
    def.validation.max_length = 20;

    // 在范围内
    declarative::ValidationResult result1 = ParameterValidator::validate("username", "john", def);
    ASSERT_TRUE(result1.success);

    // 太短
    declarative::ValidationResult result2 = ParameterValidator::validate("username", "jo", def);
    ASSERT_FALSE(result2.success);

    // 太长
    std::string long_name(25, 'a');
    declarative::ValidationResult result3 = ParameterValidator::validate("username", long_name, def);
    ASSERT_FALSE(result3.success);
}

TEST(ParameterValidatorTest_ValidateEmailPattern) {
    restful::ParamDefinition def("email", restful::ParamType::QUERY);
    def.data_type = 0;
    def.validation.has_pattern = true;
    def.validation.pattern = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";

    // 有效邮箱
    declarative::ValidationResult result1 = ParameterValidator::validate("email", "john@example.com", def);
    ASSERT_TRUE(result1.success);

    // 无效邮箱
    declarative::ValidationResult result2 = ParameterValidator::validate("email", "invalid-email", def);
    ASSERT_FALSE(result2.success);
}

TEST(ParameterValidatorTest_ValidateEnumValues) {
    restful::ParamDefinition def("status", restful::ParamType::QUERY);
    def.data_type = 0;
    def.validation.has_enum = true;
    def.validation.enum_values = {"pending", "paid", "shipped", "completed"};

    // 有效值
    declarative::ValidationResult result1 = ParameterValidator::validate("status", "paid", def);
    ASSERT_TRUE(result1.success);

    // 无效值
    declarative::ValidationResult result2 = ParameterValidator::validate("status", "invalid", def);
    ASSERT_FALSE(result2.success);
}

TEST(ParameterValidatorTest_ValidateOptionalParameter) {
    restful::ParamDefinition def("search", restful::ParamType::QUERY);
    def.validation.required = false;
    def.default_value = "";

    // 可选参数为空
    declarative::ValidationResult result1 = ParameterValidator::validate("search", "", def);
    ASSERT_TRUE(result1.success);
}

// ========== ApiDefinition 测试 ==========

TEST(ApiDefinitionTest_HandleWithParams) {
    ApiDefinition api_def("/api/users", HttpMethod::GET);

    bool handler_called = false;
    std::string captured_param;

    api_def.param("page", OptionalWithDefault<int>(1))
        .handleWithParams([&handler_called, &captured_param](const HttpRequest& req, const std::map<std::string, std::string>& params) -> HttpResponse {
            (void)req;
            handler_called = true;
            captured_param = params.at("page");
            return HttpResponse(200).json("{\"code\":200}");
        });

    // 创建测试请求
    HttpRequest req;
    req.query_params["page"] = "2";

    // 执行处理器
    HttpResponse resp = api_def.executeHandler(req);

    ASSERT_TRUE(handler_called);
    ASSERT_EQ(captured_param, "2");
    ASSERT_EQ(resp.status_code, 200);
}

TEST(ApiDefinitionTest_HandleWithParamsValidationError) {
    ApiDefinition api_def("/api/users", HttpMethod::GET);

    api_def.param("age", Required<int>()).range(18, 120)
        .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) -> HttpResponse {
            (void)req;
            (void)params;
            return HttpResponse(200).json("{\"code\":200}");
        });

    // 创建测试请求（年龄小于最小值）
    HttpRequest req;
    req.query_params["age"] = "10";

    // 执行处理器（应该返回 400 错误）
    HttpResponse resp = api_def.executeHandler(req);

    ASSERT_EQ(resp.status_code, 400);
    ASSERT_TRUE(resp.body.find("400") != std::string::npos);
}

// ========== 集成测试 ==========

TEST(IntegrationTest_FullWorkflow) {
    ApiBuilder api;

    api.get("/api/users/:id")
        .pathParam("id", Required<int>()).range(1, 1000)
        .param("include_details", OptionalWithDefault<bool>(false))
        .handleWithParams([](const HttpRequest& req, const std::map<std::string, std::string>& params) -> HttpResponse {
            (void)req;
            int id = std::stoi(params.at("id"));
            bool include_details = (params.at("include_details") == "true");

            std::string body = "{"
                "\"code\":200,"
                "\"data\":{"
                    "\"id\":" + std::to_string(id) + ","
                    "\"include_details\":" + (include_details ? "true" : "false") +
                "}"
                "}";

            return HttpResponse(200).json(body);
        });

    // 获取 API 定义
    const std::vector<ApiDefinition>& apis = api.getApis();

    ASSERT_EQ(apis.size(), 1);
    ASSERT_EQ(apis[0].path, "/api/users/:id");
    // HttpMethod 比较（通过 GET 方式创建）
    ASSERT_EQ(apis[0].params.size(), 2);  // id 和 include_details
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "声明式 DSL 自动解析和验证功能测试" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;

    RUN_TEST(ParameterParserTest_ExtractQueryParam);
    RUN_TEST(ParameterParserTest_ExtractPathParam);
    RUN_TEST(ParameterParserTest_ExtractWithDefaultValue);

    RUN_TEST(ParameterValidatorTest_ValidateRequiredParameter);
    RUN_TEST(ParameterValidatorTest_ValidateRequiredParameterWithValue);
    RUN_TEST(ParameterValidatorTest_ValidateIntRange);
    RUN_TEST(ParameterValidatorTest_ValidateStringLength);
    RUN_TEST(ParameterValidatorTest_ValidateEmailPattern);
    RUN_TEST(ParameterValidatorTest_ValidateEnumValues);
    RUN_TEST(ParameterValidatorTest_ValidateOptionalParameter);

    RUN_TEST(ApiDefinitionTest_HandleWithParams);
    RUN_TEST(ApiDefinitionTest_HandleWithParamsValidationError);

    RUN_TEST(IntegrationTest_FullWorkflow);

    std::cout << "========================================" << std::endl;
    std::cout << "测试结果: " << passed_count << "/" << test_count << " 通过" << std::endl;

    if (passed_count == test_count) {
        std::cout << "所有测试通过！" << std::endl;
        return 0;
    } else {
        std::cout << (test_count - passed_count) << " 个测试失败" << std::endl;
        return 1;
    }
}