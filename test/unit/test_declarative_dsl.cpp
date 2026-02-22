/**
 * @file test_declarative_dsl.cpp
 * @brief 声明式 DSL 单元测试
 * 
 * 测试声明式 DSL 的核心功能：
 * - 参数定义（Required、OptionalWithDefault）
 * - 验证规则（range、length、pattern、oneOf）
 * - 便捷方法（pagination、search、sort 等）
 */

#include <gtest/gtest.h>
#include "../include/declarative_dsl.h"

using namespace uvapi::declarative;

// ========== 参数定义测试 ==========

TEST(DeclarativeDsl, RequiredInt) {
    ApiBuilder api;
    auto& def = api.get("/api/test")
        .param("page", Required<int>());
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].name, "page");
    EXPECT_EQ(def.params[0].validation.required, true);
    EXPECT_EQ(def.params[0].data_type, 1);  // int
}

TEST(DeclarativeDsl, RequiredString) {
    ApiBuilder api;
    auto& def = api.get("/api/test")
        .param("username", Required<std::string>());
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].name, "username");
    EXPECT_EQ(def.params[0].validation.required, true);
    EXPECT_EQ(def.params[0].data_type, 0);  // string
}

TEST(DeclarativeDsl, RequiredBool) {
    ApiBuilder api;
    auto& def = api.get("/api/test")
        .param("active", Required<bool>());
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].name, "active");
    EXPECT_EQ(def.params[0].validation.required, true);
    EXPECT_EQ(def.params[0].data_type, 5);  // bool
}

TEST(DeclarativeDsl, OptionalWithDefaultInt) {
    ApiBuilder api;
    auto& def = api.get("/api/test")
        .param("limit", OptionalWithDefault<int>(10));
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].name, "limit");
    EXPECT_EQ(def.params[0].validation.required, false);
    EXPECT_EQ(def.params[0].data_type, 1);  // int
    EXPECT_EQ(def.params[0].default_value, "10");
}

TEST(DeclarativeDsl, OptionalWithDefaultString) {
    ApiBuilder api;
    auto& def = api.get("/api/test")
        .param("status", OptionalWithDefault<std::string>("active"));
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].name, "status");
    EXPECT_EQ(def.params[0].validation.required, false);
    EXPECT_EQ(def.params[0].data_type, 0);  // string
    EXPECT_EQ(def.params[0].default_value, "active");
}

TEST(DeclarativeDsl, OptionalWithDefaultBool) {
    ApiBuilder api;
    auto& def = api.get("/api/test")
        .param("enabled", OptionalWithDefault<bool>(true));
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].name, "enabled");
    EXPECT_EQ(def.params[0].validation.required, false);
    EXPECT_EQ(def.params[0].data_type, 5);  // bool
    EXPECT_EQ(def.params[0].default_value, "true");
}

TEST(DeclarativeDsl, OptionalWithDefaultBoolFalse) {
    ApiBuilder api;
    auto& def = api.get("/api/test")
        .param("disabled", OptionalWithDefault<bool>(false));
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].name, "disabled");
    EXPECT_EQ(def.params[0].validation.required, false);
    EXPECT_EQ(def.params[0].data_type, 5);  // bool
    EXPECT_EQ(def.params[0].default_value, "false");
}

// ========== 验证规则测试 ==========

TEST(DeclarativeDsl, RangeValidation) {
    ApiBuilder api;
    auto& def = api.get("/api/test")
        .param("age", Required<int>()).range(18, 120);
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].validation.min_value, 18);
    EXPECT_EQ(def.params[0].validation.max_value, 120);
    EXPECT_EQ(def.params[0].validation.has_min, true);
    EXPECT_EQ(def.params[0].validation.has_max, true);
}

TEST(DeclarativeDsl, LengthValidation) {
    ApiBuilder api;
    auto& def = api.get("/api/test")
        .param("username", Required<std::string>()).length(3, 20);
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].validation.min_length, 3);
    EXPECT_EQ(def.params[0].validation.max_length, 20);
    EXPECT_EQ(def.params[0].validation.has_min_length, true);
    EXPECT_EQ(def.params[0].validation.has_max_length, true);
}

TEST(DeclarativeDsl, PatternValidation) {
    ApiBuilder api;
    auto& def = api.get("/api/test")
        .param("email", Required<std::string>()).pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].validation.pattern, "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    EXPECT_EQ(def.params[0].validation.has_pattern, true);
}

TEST(DeclarativeDsl, OneOfValidation) {
    ApiBuilder api;
    auto& def = api.get("/api/test")
        .param("status", OptionalWithDefault<std::string>("active"))
        .oneOf({"active", "inactive", "pending"});
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].validation.enum_values.size(), 3);
    EXPECT_EQ(def.params[0].validation.enum_values[0], "active");
    EXPECT_EQ(def.params[0].validation.enum_values[1], "inactive");
    EXPECT_EQ(def.params[0].validation.enum_values[2], "pending");
    EXPECT_EQ(def.params[0].validation.has_enum, true);
}

// ========== 便捷方法测试 ==========

TEST(DeclarativeDsl, PaginationDefault) {
    ApiBuilder api;
    auto& def = api.get("/api/test").pagination();
    
    ASSERT_EQ(def.params.size(), 2);
    EXPECT_EQ(def.params[0].name, "page");
    EXPECT_EQ(def.params[0].default_value, "1");
    EXPECT_EQ(def.params[1].name, "limit");
    EXPECT_EQ(def.params[1].default_value, "10");
}

TEST(DeclarativeDsl, PaginationCustom) {
    ApiBuilder api;
    auto& def = api.get("/api/test").pagination(PageParam().page(2).limit(50));
    
    ASSERT_EQ(def.params.size(), 2);
    EXPECT_EQ(def.params[0].name, "page");
    EXPECT_EQ(def.params[0].default_value, "2");
    EXPECT_EQ(def.params[1].name, "limit");
    EXPECT_EQ(def.params[1].default_value, "50");
}

TEST(DeclarativeDsl, SearchDefault) {
    ApiBuilder api;
    auto& def = api.get("/api/test").search();
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].name, "search");
    EXPECT_EQ(def.params[0].default_value, "");
}

TEST(DeclarativeDsl, SearchCustom) {
    ApiBuilder api;
    auto& def = api.get("/api/test").search(SearchParam("test"));
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].name, "search");
    EXPECT_EQ(def.params[0].default_value, "test");
}

TEST(DeclarativeDsl, SortDefault) {
    ApiBuilder api;
    auto& def = api.get("/api/test").sort();
    
    ASSERT_EQ(def.params.size(), 2);
    EXPECT_EQ(def.params[0].name, "sort");
    EXPECT_EQ(def.params[0].default_value, "id");
    EXPECT_EQ(def.params[1].name, "order");
    EXPECT_EQ(def.params[1].default_value, "asc");
}

TEST(DeclarativeDsl, SortCustom) {
    ApiBuilder api;
    auto& def = api.get("/api/test")
        .sort(SortParam().field("created_at").order("desc"));
    
    ASSERT_EQ(def.params.size(), 2);
    EXPECT_EQ(def.params[0].name, "sort");
    EXPECT_EQ(def.params[0].default_value, "created_at");
    EXPECT_EQ(def.params[1].name, "order");
    EXPECT_EQ(def.params[1].default_value, "desc");
}

TEST(DeclarativeDsl, StatusFilter) {
    ApiBuilder api;
    auto& def = api.get("/api/test")
        .statusFilter({"active", "inactive", "pending"}, "active");
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].name, "status");
    EXPECT_EQ(def.params[0].default_value, "active");
    EXPECT_EQ(def.params[0].validation.enum_values.size(), 3);
    EXPECT_EQ(def.params[0].validation.has_enum, true);
}

// ========== 多个参数组合测试 ==========

TEST(DeclarativeDsl, MultipleParams) {
    ApiBuilder api;
    auto& def = api.get("/api/users")
        .pagination(PageParam().page(1).limit(20))
        .search(SearchParam())
        .sort(SortParam().field("created_at").order("desc"))
        .statusFilter({"active", "inactive", "pending"}, "active");
    
    ASSERT_EQ(def.params.size(), 5);
    EXPECT_EQ(def.params[0].name, "page");
    EXPECT_EQ(def.params[1].name, "limit");
    EXPECT_EQ(def.params[2].name, "search");
    EXPECT_EQ(def.params[3].name, "sort");
    EXPECT_EQ(def.params[4].name, "status");
}

TEST(DeclarativeDsl, PathParam) {
    ApiBuilder api;
    auto& def = api.get("/api/users/:id")
        .pathParam("id", Required<int>());
    
    ASSERT_EQ(def.params.size(), 1);
    EXPECT_EQ(def.params[0].name, "id");
    EXPECT_EQ(def.params[0].type, restful::ParamType::PATH);
    EXPECT_EQ(def.params[0].validation.required, true);
    EXPECT_EQ(def.params[0].data_type, 1);  // int
}

// ========== 不同 HTTP 方法测试 ==========

TEST(DeclarativeDsl, PostMethod) {
    ApiBuilder api;
    auto& def = api.post("/api/users");
    
    EXPECT_EQ(def.method, HttpMethod::POST);
    EXPECT_EQ(def.path, "/api/users");
}

TEST(DeclarativeDsl, PutMethod) {
    ApiBuilder api;
    auto& def = api.put("/api/users/:id");
    
    EXPECT_EQ(def.method, HttpMethod::PUT);
    EXPECT_EQ(def.path, "/api/users/:id");
}

TEST(DeclarativeDsl, DeleteMethod) {
    ApiBuilder api;
    auto& def = api.del("/api/users/:id");
    
    EXPECT_EQ(def.method, HttpMethod::DELETE);
    EXPECT_EQ(def.path, "/api/users/:id");
}

TEST(DeclarativeDsl, PatchMethod) {
    ApiBuilder api;
    auto& def = api.patch("/api/users/:id");
    
    EXPECT_EQ(def.method, HttpMethod::PATCH);
    EXPECT_EQ(def.path, "/api/users/:id");
}

// ========== 主函数 ==========

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
