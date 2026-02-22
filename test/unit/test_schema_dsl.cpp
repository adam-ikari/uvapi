/**
 * @file test_schema_dsl.cpp
 * @brief Schema DSL 单元测试
 * 
 * 测试 Schema 的功能：
 * - 字段定义（Required、OptionalWithDefault）
 * - 字段验证规则（range、length、pattern、oneOf）
 * - Schema 可复用性
 */

#include <gtest/gtest.h>
#include "../include/declarative_dsl.h"

using namespace uvapi::declarative;

// ========== Schema 字段定义测试 ==========

TEST(SchemaDsl, RequiredIntField) {
    Schema<int> schema;
    schema.field("age", Required<int>());
    
    auto fields = schema.getFields();
    ASSERT_EQ(fields.size(), 1);
    EXPECT_EQ(fields[0].name, "age");
    EXPECT_EQ(fields[0].validation.required, true);
    EXPECT_EQ(fields[0].data_type, 1);  // int
}

TEST(SchemaDsl, RequiredStringField) {
    Schema<int> schema;
    schema.field("username", Required<std::string>());
    
    auto fields = schema.getFields();
    ASSERT_EQ(fields.size(), 1);
    EXPECT_EQ(fields[0].name, "username");
    EXPECT_EQ(fields[0].validation.required, true);
    EXPECT_EQ(fields[0].data_type, 0);  // string
}

TEST(SchemaDsl, OptionalWithDefaultIntField) {
    Schema<int> schema;
    schema.field("age", OptionalWithDefault<int>(18));
    
    auto fields = schema.getFields();
    ASSERT_EQ(fields.size(), 1);
    EXPECT_EQ(fields[0].name, "age");
    EXPECT_EQ(fields[0].validation.required, false);
    EXPECT_EQ(fields[0].data_type, 1);  // int
    EXPECT_EQ(fields[0].default_value, "18");
}

TEST(SchemaDsl, OptionalWithDefaultStringField) {
    Schema<int> schema;
    schema.field("status", OptionalWithDefault<std::string>("active"));
    
    auto fields = schema.getFields();
    ASSERT_EQ(fields.size(), 1);
    EXPECT_EQ(fields[0].name, "status");
    EXPECT_EQ(fields[0].validation.required, false);
    EXPECT_EQ(fields[0].data_type, 0);  // string
    EXPECT_EQ(fields[0].default_value, "active");
}

TEST(SchemaDsl, OptionalWithDefaultBoolField) {
    Schema<int> schema;
    schema.field("active", OptionalWithDefault<bool>(true));
    
    auto fields = schema.getFields();
    ASSERT_EQ(fields.size(), 1);
    EXPECT_EQ(fields[0].name, "active");
    EXPECT_EQ(fields[0].validation.required, false);
    EXPECT_EQ(fields[0].data_type, 5);  // bool
    EXPECT_EQ(fields[0].default_value, "true");
}

// ========== Schema 验证规则测试 ==========

TEST(SchemaDsl, RangeValidation) {
    Schema<int> schema;
    schema.field("age", OptionalWithDefault<int>(18)).range(18, 120);
    
    auto fields = schema.getFields();
    ASSERT_EQ(fields.size(), 1);
    EXPECT_EQ(fields[0].validation.min_value, 18);
    EXPECT_EQ(fields[0].validation.max_value, 120);
    EXPECT_EQ(fields[0].validation.has_min, true);
    EXPECT_EQ(fields[0].validation.has_max, true);
}

TEST(SchemaDsl, LengthValidation) {
    Schema<int> schema;
    schema.field("username", Required<std::string>()).length(3, 20);
    
    auto fields = schema.getFields();
    ASSERT_EQ(fields.size(), 1);
    EXPECT_EQ(fields[0].validation.min_length, 3);
    EXPECT_EQ(fields[0].validation.max_length, 20);
    EXPECT_EQ(fields[0].validation.has_min_length, true);
    EXPECT_EQ(fields[0].validation.has_max_length, true);
}

TEST(SchemaDsl, PatternValidation) {
    Schema<int> schema;
    schema.field("email", Required<std::string>())
        .pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    
    auto fields = schema.getFields();
    ASSERT_EQ(fields.size(), 1);
    EXPECT_EQ(fields[0].validation.pattern, "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    EXPECT_EQ(fields[0].validation.has_pattern, true);
}

TEST(SchemaDsl, OneOfValidation) {
    Schema<int> schema;
    schema.field("status", OptionalWithDefault<std::string>("active"))
        .oneOf({"active", "inactive", "pending"});
    
    auto fields = schema.getFields();
    ASSERT_EQ(fields.size(), 1);
    EXPECT_EQ(fields[0].validation.enum_values.size(), 3);
    EXPECT_EQ(fields[0].validation.enum_values[0], "active");
    EXPECT_EQ(fields[0].validation.enum_values[1], "inactive");
    EXPECT_EQ(fields[0].validation.enum_values[2], "pending");
    EXPECT_EQ(fields[0].validation.has_enum, true);
}

// ========== Schema 多字段测试 ==========

TEST(SchemaDsl, MultipleFields) {
    Schema<int> schema;
    schema.field("username", Required<std::string>()).length(3, 20);
    schema.field("email", Required<std::string>())
        .pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    schema.field("age", OptionalWithDefault<int>(18)).range(18, 120);
    schema.field("active", OptionalWithDefault<bool>(true));
    
    auto fields = schema.getFields();
    ASSERT_EQ(fields.size(), 4);
    
    EXPECT_EQ(fields[0].name, "username");
    EXPECT_EQ(fields[0].validation.required, true);
    EXPECT_EQ(fields[0].validation.has_min_length, true);
    
    EXPECT_EQ(fields[1].name, "email");
    EXPECT_EQ(fields[1].validation.required, true);
    EXPECT_EQ(fields[1].validation.has_pattern, true);
    
    EXPECT_EQ(fields[2].name, "age");
    EXPECT_EQ(fields[2].validation.required, false);
    EXPECT_EQ(fields[2].default_value, "18");
    EXPECT_EQ(fields[2].validation.has_min, true);
    
    EXPECT_EQ(fields[3].name, "active");
    EXPECT_EQ(fields[3].validation.required, false);
    EXPECT_EQ(fields[3].default_value, "true");
}

// ========== Schema 与 ApiDefinition 集成测试 ==========

TEST(SchemaDsl, ApiDefinitionWithSchema) {
    Schema<int> userSchema;
    userSchema.field("username", Required<std::string>()).length(3, 20);
    userSchema.field("email", Required<std::string>())
        .pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    userSchema.field("age", OptionalWithDefault<int>(18)).range(18, 120);
    userSchema.field("active", OptionalWithDefault<bool>(true));
    
    ApiBuilder api;
    auto& def = api.post("/api/users").body(userSchema);
    
    ASSERT_EQ(def.body_fields.size(), 4);
    EXPECT_EQ(def.body_fields[0].name, "username");
    EXPECT_EQ(def.body_fields[1].name, "email");
    EXPECT_EQ(def.body_fields[2].name, "age");
    EXPECT_EQ(def.body_fields[3].name, "active");
}

TEST(SchemaDsl, SchemaReusability) {
    Schema<int> userSchema;
    userSchema.field("username", Required<std::string>()).length(3, 20);
    userSchema.field("email", Required<std::string>())
        .pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    
    ApiBuilder api;
    auto& createDef = api.post("/api/users").body(userSchema);
    auto& updateDef = api.put("/api/users/:id")
        .pathParam("id", Required<int>())
        .body(userSchema);
    
    ASSERT_EQ(createDef.body_fields.size(), 2);
    ASSERT_EQ(updateDef.body_fields.size(), 2);
    
    EXPECT_EQ(createDef.body_fields[0].name, "username");
    EXPECT_EQ(updateDef.body_fields[0].name, "username");
}

// ========== 主函数 ==========

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}