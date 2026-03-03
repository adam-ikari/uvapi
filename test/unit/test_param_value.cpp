/**
 * @file test_param_value.cpp
 * @brief 单元测试：ParamValue 和 ParamAccessor 类
 */

#include <iostream>
#include <cassert>
#include <limits>
#include "../../include/framework.h"

using namespace uvapi;

int test_count = 0;
int passed_count = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    test_count++; \
    std::cout << "Running test: " << #name << "..."; \
    test_##name(); \
    std::cout << " PASSED" << std::endl; \
    passed_count++; \
} while(0)

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        std::cerr << "\n  FAILED: " << #expr << " is false" << std::endl; \
        exit(1); \
    } \
} while(0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        std::cerr << "\n  FAILED: " << #a << " != " << #b << std::endl; \
        std::cerr << "    Expected: " << (b) << ", Got: " << (a) << std::endl; \
        exit(1); \
    } \
} while(0)

#define ASSERT_NE(a, b) ASSERT_TRUE((a) != (b))

#define ASSERT_DOUBLE_EQ(a, b) do { \
    double diff = (a) - (b); \
    if (diff < 0) diff = -diff; \
    if (diff > 0.0001) { \
        std::cerr << "\n  FAILED: " << #a << " != " << #b << std::endl; \
        exit(1); \
    } \
} while(0)

// ========== ParamValue 基本功能测试 ==========

TEST(HasValue_DefaultConstructor) {
    ParamValue pv;
    ASSERT_FALSE(pv.hasValue());
    ASSERT_TRUE(pv.empty());
    ASSERT_FALSE(pv.hasError());
    ASSERT_EQ(pv.value(), "");
}

TEST(HasValue_ValueConstructor) {
    ParamValue pv("test");
    ASSERT_TRUE(pv.hasValue());
    ASSERT_FALSE(pv.empty());
    ASSERT_FALSE(pv.hasError());
    ASSERT_EQ(pv.value(), "test");
}

TEST(Empty_EmptyString) {
    ParamValue pv("");
    ASSERT_TRUE(pv.hasValue());
    ASSERT_TRUE(pv.empty());
    ASSERT_FALSE(pv.hasError());
    ASSERT_EQ(pv.value(), "");
}

// ========== ParamValue 错误报告测试 ==========

TEST(ErrorState_InitialState) {
    ParamValue pv("test");
    ASSERT_FALSE(pv.hasError());
    ASSERT_EQ(pv.errorMessage(), "");
}

TEST(ErrorState_AfterFailedConversion) {
    ParamValue pv("abc");
    int value = pv;
    ASSERT_TRUE(pv.hasError());
    ASSERT_FALSE(pv.errorMessage().empty());
}

// ========== 字符串转换测试 ==========

TEST(StringConversion_Success) {
    ParamValue pv("hello");
    std::string value = pv;
    ASSERT_EQ(value, "hello");
    ASSERT_FALSE(pv.hasError());
}

TEST(StringConversion_Empty) {
    ParamValue pv("");
    std::string value = pv;
    ASSERT_EQ(value, "");
    ASSERT_FALSE(pv.hasError());
}

// ========== 布尔转换测试 ==========

TEST(BoolConversion_True) {
    ParamValue pv("true");
    bool value = pv;
    ASSERT_TRUE(value);
    ASSERT_FALSE(pv.hasError());
}

TEST(BoolConversion_False) {
    ParamValue pv("false");
    bool value = pv;
    ASSERT_FALSE(value);
    ASSERT_FALSE(pv.hasError());
}

TEST(BoolConversion_CaseInsensitive) {
    ParamValue pv1("TRUE");
    ParamValue pv2("False");
    ParamValue pv3("tRuE");
    ParamValue pv4("fAlSe");
    
    bool v1 = pv1;
    bool v2 = pv2;
    bool v3 = pv3;
    bool v4 = pv4;
    
    ASSERT_TRUE(v1);
    ASSERT_FALSE(v2);
    ASSERT_TRUE(v3);
    ASSERT_FALSE(v4);
    
    ASSERT_FALSE(pv1.hasError());
    ASSERT_FALSE(pv2.hasError());
    ASSERT_FALSE(pv3.hasError());
    ASSERT_FALSE(pv4.hasError());
}

TEST(BoolConversion_RejectOne) {
    ParamValue pv("1");
    bool value = pv;
    ASSERT_FALSE(value);
    ASSERT_TRUE(pv.hasError());
}

TEST(BoolConversion_RejectYes) {
    ParamValue pv("yes");
    bool value = pv;
    ASSERT_FALSE(value);
    ASSERT_TRUE(pv.hasError());
}

TEST(BoolConversion_RejectOn) {
    ParamValue pv("on");
    bool value = pv;
    ASSERT_FALSE(value);
    ASSERT_TRUE(pv.hasError());
}

TEST(BoolConversion_RejectInvalid) {
    ParamValue pv("invalid");
    bool value = pv;
    ASSERT_FALSE(value);
    ASSERT_TRUE(pv.hasError());
}

// ========== 整数转换测试 ==========

TEST(IntConversion_Zero) {
    ParamValue pv("0");
    int value = pv;
    ASSERT_EQ(value, 0);
    ASSERT_FALSE(pv.hasError());
}

TEST(IntConversion_Positive) {
    ParamValue pv("123");
    int value = pv;
    ASSERT_EQ(value, 123);
    ASSERT_FALSE(pv.hasError());
}

TEST(IntConversion_Negative) {
    ParamValue pv("-456");
    int value = pv;
    ASSERT_EQ(value, -456);
    ASSERT_FALSE(pv.hasError());
}

TEST(IntConversion_InvalidFormat) {
    ParamValue pv("abc");
    int value = pv;
    ASSERT_EQ(value, 0);
    ASSERT_TRUE(pv.hasError());
}

TEST(IntConversion_PartialNumber) {
    ParamValue pv("123abc");
    int value = pv;
    ASSERT_EQ(value, 0);
    ASSERT_TRUE(pv.hasError());
}

TEST(IntConversion_Empty) {
    ParamValue pv("");
    int value = pv;
    ASSERT_EQ(value, 0);
    ASSERT_TRUE(pv.hasError());
}

TEST(Int64Conversion_Max) {
    ParamValue pv("9223372036854775807");
    int64_t value = pv;
    ASSERT_EQ(value, 9223372036854775807LL);
    ASSERT_FALSE(pv.hasError());
}

TEST(Int64Conversion_Min) {
    ParamValue pv("-9223372036854775808");
    int64_t value = pv;
    ASSERT_EQ(value, -9223372036854775807LL - 1);
    ASSERT_FALSE(pv.hasError());
}

TEST(IntConversion_Overflow) {
    ParamValue pv("99999999999999999999");
    int64_t value = pv;
    ASSERT_TRUE(pv.hasError());
}

TEST(IntConversion_Underflow) {
    ParamValue pv("-99999999999999999999");
    int64_t value = pv;
    ASSERT_TRUE(pv.hasError());
}

TEST(IntConversion_RangeOverflow) {
    ParamValue pv("2147483648");
    int value = pv;
    ASSERT_TRUE(pv.hasError());
}

// ========== 浮点数转换测试 ==========

TEST(DoubleConversion_Zero) {
    ParamValue pv("0.0");
    double value = pv;
    ASSERT_DOUBLE_EQ(value, 0.0);
    if (pv.hasError()) {
        std::cerr << "  Error message: " << pv.errorMessage() << std::endl;
    }
    ASSERT_FALSE(pv.hasError());
}

TEST(DoubleConversion_Positive) {
    ParamValue pv("123.456");
    double value = pv;
    ASSERT_DOUBLE_EQ(value, 123.456);
    ASSERT_FALSE(pv.hasError());
}

TEST(DoubleConversion_Negative) {
    ParamValue pv("-789.012");
    double value = pv;
    ASSERT_DOUBLE_EQ(value, -789.012);
    ASSERT_FALSE(pv.hasError());
}

TEST(DoubleConversion_InvalidFormat) {
    ParamValue pv("abc");
    double value = pv;
    ASSERT_DOUBLE_EQ(value, 0.0);
    ASSERT_TRUE(pv.hasError());
}

TEST(DoubleConversion_Empty) {
    ParamValue pv("");
    double value = pv;
    ASSERT_DOUBLE_EQ(value, 0.0);
    ASSERT_TRUE(pv.hasError());
}

TEST(DoubleConversion_ScientificNotation) {
    ParamValue pv("1.23e5");
    double value = pv;
    ASSERT_DOUBLE_EQ(value, 123000.0);
    ASSERT_FALSE(pv.hasError());
}

// ========== as<T>() 方法测试 ==========

TEST(AsMethod_IntSuccess) {
    ParamValue pv("123");
    optional<int> result = pv.as<int>();
    ASSERT_TRUE(result.hasValue());
    ASSERT_EQ(result.value(), 123);
    ASSERT_FALSE(pv.hasError());
}

TEST(AsMethod_IntFailure) {
    ParamValue pv("abc");
    optional<int> result = pv.as<int>();
    ASSERT_FALSE(result.hasValue());
    ASSERT_TRUE(pv.hasError());
}

TEST(AsMethod_NoValue) {
    ParamValue pv;
    optional<int> result = pv.as<int>();
    ASSERT_FALSE(result.hasValue());
}

TEST(AsMethod_BoolSuccess) {
    ParamValue pv("true");
    optional<bool> result = pv.as<bool>();
    ASSERT_TRUE(result.hasValue());
    ASSERT_TRUE(result.value());
    ASSERT_FALSE(pv.hasError());
}

TEST(AsMethod_BoolFailure) {
    ParamValue pv("1");
    optional<bool> result = pv.as<bool>();
    ASSERT_FALSE(result.hasValue());
    ASSERT_TRUE(pv.hasError());
}

TEST(AsMethod_DoubleSuccess) {
    ParamValue pv("123.456");
    optional<double> result = pv.as<double>();
    ASSERT_TRUE(result.hasValue());
    ASSERT_DOUBLE_EQ(result.value(), 123.456);
    ASSERT_FALSE(pv.hasError());
}

TEST(AsMethod_StringSuccess) {
    ParamValue pv("hello");
    optional<std::string> result = pv.as<std::string>();
    ASSERT_TRUE(result.hasValue());
    ASSERT_EQ(result.value(), "hello");
    ASSERT_FALSE(pv.hasError());
}

// ========== ParamAccessor 测试 ==========

TEST(ParamAccessor_ExistingKey) {
    std::map<std::string, std::string> params;
    params["page"] = "1";
    params["limit"] = "10";
    
    ParamAccessor accessor(params);
    ParamValue pv = accessor["page"];
    
    ASSERT_TRUE(pv.hasValue());
    ASSERT_EQ(pv.value(), "1");
}

TEST(ParamAccessor_MissingKey) {
    std::map<std::string, std::string> params;
    params["page"] = "1";
    
    ParamAccessor accessor(params);
    ParamValue pv = accessor["missing"];
    
    ASSERT_FALSE(pv.hasValue());
    ASSERT_TRUE(pv.empty());
}

TEST(ParamAccessor_EmptyStringValue) {
    std::map<std::string, std::string> params;
    params["empty"] = "";
    
    ParamAccessor accessor(params);
    ParamValue pv = accessor["empty"];
    
    ASSERT_TRUE(pv.hasValue());
    ASSERT_TRUE(pv.empty());
    ASSERT_EQ(pv.value(), "");
}

// ========== 边界条件测试 ==========

TEST(Boundary_IntMax) {
    ParamValue pv("2147483647");
    int value = pv;
    ASSERT_EQ(value, INT_MAX);
    ASSERT_FALSE(pv.hasError());
}

TEST(Boundary_IntMin) {
    ParamValue pv("-2147483648");
    int value = pv;
    ASSERT_EQ(value, INT_MIN);
    ASSERT_FALSE(pv.hasError());
}

// ========== 错误消息质量测试 ==========

TEST(ErrorMessage_InvalidInt) {
    ParamValue pv("abc123");
    int value = pv;
    
    ASSERT_TRUE(pv.hasError());
    std::string msg = pv.errorMessage();
    
    ASSERT_FALSE(msg.empty());
    ASSERT_NE(msg.find("Invalid"), std::string::npos);
}

TEST(ErrorMessage_InvalidBool) {
    ParamValue pv("maybe");
    bool value = pv;
    
    ASSERT_TRUE(pv.hasError());
    std::string msg = pv.errorMessage();
    
    ASSERT_FALSE(msg.empty());
    ASSERT_NE(msg.find("Invalid boolean"), std::string::npos);
}

// ========== 主函数 ==========

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "ParamValue and ParamAccessor Unit Tests" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    // ParamValue 基本功能测试
    std::cout << "ParamValue Basic Tests:" << std::endl;
    RUN_TEST(HasValue_DefaultConstructor);
    RUN_TEST(HasValue_ValueConstructor);
    RUN_TEST(Empty_EmptyString);
    
    // ParamValue 错误报告测试
    std::cout << std::endl << "Error Reporting Tests:" << std::endl;
    RUN_TEST(ErrorState_InitialState);
    RUN_TEST(ErrorState_AfterFailedConversion);
    
    // 字符串转换测试
    std::cout << std::endl << "String Conversion Tests:" << std::endl;
    RUN_TEST(StringConversion_Success);
    RUN_TEST(StringConversion_Empty);
    
    // 布尔转换测试
    std::cout << std::endl << "Boolean Conversion Tests:" << std::endl;
    RUN_TEST(BoolConversion_True);
    RUN_TEST(BoolConversion_False);
    RUN_TEST(BoolConversion_CaseInsensitive);
    RUN_TEST(BoolConversion_RejectOne);
    RUN_TEST(BoolConversion_RejectYes);
    RUN_TEST(BoolConversion_RejectOn);
    RUN_TEST(BoolConversion_RejectInvalid);
    
    // 整数转换测试
    std::cout << std::endl << "Integer Conversion Tests:" << std::endl;
    RUN_TEST(IntConversion_Zero);
    RUN_TEST(IntConversion_Positive);
    RUN_TEST(IntConversion_Negative);
    RUN_TEST(IntConversion_InvalidFormat);
    RUN_TEST(IntConversion_PartialNumber);
    RUN_TEST(IntConversion_Empty);
    RUN_TEST(Int64Conversion_Max);
    RUN_TEST(Int64Conversion_Min);
    RUN_TEST(IntConversion_Overflow);
    RUN_TEST(IntConversion_Underflow);
    RUN_TEST(IntConversion_RangeOverflow);
    
    // 浮点数转换测试
    std::cout << std::endl << "Double Conversion Tests:" << std::endl;
    RUN_TEST(DoubleConversion_Zero);
    RUN_TEST(DoubleConversion_Positive);
    RUN_TEST(DoubleConversion_Negative);
    RUN_TEST(DoubleConversion_InvalidFormat);
    RUN_TEST(DoubleConversion_Empty);
    RUN_TEST(DoubleConversion_ScientificNotation);
    
    // as<T>() 方法测试
    std::cout << std::endl << "as<T>() Method Tests:" << std::endl;
    RUN_TEST(AsMethod_IntSuccess);
    RUN_TEST(AsMethod_IntFailure);
    RUN_TEST(AsMethod_NoValue);
    RUN_TEST(AsMethod_BoolSuccess);
    RUN_TEST(AsMethod_BoolFailure);
    RUN_TEST(AsMethod_DoubleSuccess);
    RUN_TEST(AsMethod_StringSuccess);
    
    // ParamAccessor 测试
    std::cout << std::endl << "ParamAccessor Tests:" << std::endl;
    RUN_TEST(ParamAccessor_ExistingKey);
    RUN_TEST(ParamAccessor_MissingKey);
    RUN_TEST(ParamAccessor_EmptyStringValue);
    
    // 边界条件测试
    std::cout << std::endl << "Boundary Tests:" << std::endl;
    RUN_TEST(Boundary_IntMax);
    RUN_TEST(Boundary_IntMin);
    
    // 错误消息质量测试
    std::cout << std::endl << "Error Message Quality Tests:" << std::endl;
    RUN_TEST(ErrorMessage_InvalidInt);
    RUN_TEST(ErrorMessage_InvalidBool);
    
    // 打印结果
    std::cout << std::endl << "========================================" << std::endl;
    std::cout << "Test Results:" << std::endl;
    std::cout << "  Total: " << test_count << std::endl;
    std::cout << "  Passed: " << passed_count << std::endl;
    std::cout << "  Failed: " << (test_count - passed_count) << std::endl;
    std::cout << "========================================" << std::endl;
    
    if (test_count == passed_count) {
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "Some tests failed!" << std::endl;
        return 1;
    }
}
