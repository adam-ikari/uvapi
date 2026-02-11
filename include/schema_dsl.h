/**
 * @file schema_dsl.h
 * @brief Schema DSL - 自动计算字段偏移量的 Schema 定义系统
 * 
 * ## 概述
 * 
 * Schema DSL 提供了简洁、类型安全的 Schema 定义 API，通过成员指针自动计算字段偏移量，
 * 避免手动指定 `offsetof`，减少错误并提高代码可维护性。
 * 
 * ## 特性
 * 
 * 1. **自动偏移量计算** - 编译器自动计算字段偏移量，无需手动指定
 * 2. **类型安全** - 成员指针类型由编译器检查
 * 3. **重构友好** - 字段顺序改变时无需手动修改偏移量
 * 4. **链式 API** - 支持流式配置，代码简洁易读
 * 5. **完整验证** - 支持必填/可选、长度、范围、正则、枚举等验证规则
 * 
 * ## 使用示例
 * 
 * @code
 * #include "schema_dsl.h"
 * 
 * // 定义用户模型
 * struct User {
 *     int64_t id;
 *     std::string username;
 *     std::string email;
 *     int age;
 *     bool active;
 * };
 * 
 * // 定义 Schema
 * class UserSchema : public uvapi::DslBodySchema<User> {
 * public:
 *     void define() override {
 *         // 使用成员指针，自动计算偏移量
 *         this->field(&User::id, "id").asInt64().required();
 *         this->field(&User::username, "username").asString().required()
 *             .minLength(3).maxLength(20);
 *         this->field(&User::email, "email").asString().required()
 *             .pattern("^[^@]+@[^@]+$");
 *         this->field(&User::age, "age").asInt().required()
 *             .range(18, 120);
 *         this->field(&User::active, "active").asBool().required();
 *     }
 * };
 * 
 * // 使用 Schema
 * void createUser(const uvapi::Request& req) {
 *     auto result = uvapi::parseBody<User>(req);
 *     if (!result.success) {
 *         return uvapi::badRequest(result.error);
 *     }
 *     
 *     User user = result.instance;
 *     // 处理用户创建逻辑...
 *     
 *     return uvapi::ok("User created", user);
 * }
 * @endcode
 * 
 * ## 支持的字段类型
 * 
 * **基础类型**:
 * - **string()** - 字符串类型
 * - **int8()** - 8位有符号整数（-128 ~ 127）
 * - **int16()** - 16位有符号整数（-32768 ~ 32767）
 * - **int32()** - 32位有符号整数（-2147483648 ~ 2147483647）
 * - **int64()** - 64位有符号整数
 * - **uint8()** - 8位无符号整数（0 ~ 255）
 * - **uint16()** - 16位无符号整数（0 ~ 65535）
 * - **uint32()** - 32位无符号整数（0 ~ 4294967295）
 * - **uint64()** - 64位无符号整数
 * - **fp32()** - 32位浮点数（单精度 float）
 * - **fp64()** - 64位浮点数（双精度 double）
 * - **boolean()** - 布尔类型
 * 
 * **高级数据类型**:
 * - **date()** - 日期类型（YYYY-MM-DD），自动验证格式
 * - **datetime()** - 日期时间类型（YYYY-MM-DD HH:MM:SS），自动验证格式
 * - **email()** - 邮箱类型，自动验证邮箱格式
 * - **url()** - URL 类型，自动验证 URL 格式
 * - **uuid()** - UUID 类型，自动验证 UUID 格式
 * 
 * **复合类型**:
 * - **array()** - 数组类型
 * - **custom()** - 自定义类型，通过 ICustomTypeHandler 处理
 * 
 * ## 支持的验证规则
 * 
 * - **required() / optional()** - 必填/可选
 * - **minLength() / maxLength() / length()** - 字符串长度
 * - **min() / max() / range()** - 数值范围
 * - **pattern()** - 正则表达式匹配
 * - **oneOf()** - 枚举值
 */

#ifndef SCHEMA_DSL_H
#define SCHEMA_DSL_H

#include "framework.h"
#include "builtin_types.h"
#include <cstddef>
#include <utility>

namespace uvapi {

// ========== Schema DSL ==========

// Schema 定义器（自动计算偏移量）
template<typename T>
class Schema {
private:
    std::vector<FieldDefinition> fields_;
    
public:
    Schema() {}
    
    // ========== 字段定义方法 ==========
    // 这些方法自动计算字段偏移量
        
        // 字符串字段
        Schema& string(std::string T::*field, const std::string& name) {
            size_t offset = getMemberOffset(field);
            fields_.push_back(FieldDefinition(name, FieldType::STRING, offset));
            return *this;
        }
        
        // 8位整数字段
        Schema& int8(int8_t T::*field, const std::string& name) {
            size_t offset = getMemberOffset(field);
            fields_.push_back(FieldDefinition(name, FieldType::INT8, offset));
            return *this;
        }
        
        // 16位整数字段
        Schema& int16(int16_t T::*field, const std::string& name) {
            size_t offset = getMemberOffset(field);
            fields_.push_back(FieldDefinition(name, FieldType::INT16, offset));
            return *this;
        }
        
        // 32位整数字段
        Schema& int32(int32_t T::*field, const std::string& name) {
            size_t offset = getMemberOffset(field);
            fields_.push_back(FieldDefinition(name, FieldType::INT32, offset));
            return *this;
        }
        
        // 64位整数字段
        Schema& int64(int64_t T::*field, const std::string& name) {
            size_t offset = getMemberOffset(field);
            fields_.push_back(FieldDefinition(name, FieldType::INT64, offset));
            return *this;
        }
        
        // 8位无符号整数字段
        Schema& uint8(uint8_t T::*field, const std::string& name) {
            size_t offset = getMemberOffset(field);
            fields_.push_back(FieldDefinition(name, FieldType::UINT8, offset));
            return *this;
        }
        
        // 16位无符号整数字段
        Schema& uint16(uint16_t T::*field, const std::string& name) {
            size_t offset = getMemberOffset(field);
            fields_.push_back(FieldDefinition(name, FieldType::UINT16, offset));
            return *this;
        }
        
        // 32位无符号整数字段
        Schema& uint32(uint32_t T::*field, const std::string& name) {
            size_t offset = getMemberOffset(field);
            fields_.push_back(FieldDefinition(name, FieldType::UINT32, offset));
            return *this;
        }
        
        // 64位无符号整数字段
        Schema& uint64(uint64_t T::*field, const std::string& name) {
            size_t offset = getMemberOffset(field);
            fields_.push_back(FieldDefinition(name, FieldType::UINT64, offset));
            return *this;
        }
        
        // 32位浮点数字段
        Schema& fp32(float T::*field, const std::string& name) {
            size_t offset = getMemberOffset(field);
            fields_.push_back(FieldDefinition(name, FieldType::FP32, offset));
            return *this;
        }
        
        // 64位浮点数字段
        Schema& fp64(double T::*field, const std::string& name) {
            size_t offset = getMemberOffset(field);
            fields_.push_back(FieldDefinition(name, FieldType::FP64, offset));
            return *this;
        }
        
        // 布尔字段
        Schema& boolean(bool T::*field, const std::string& name) {
            size_t offset = getMemberOffset(field);
            fields_.push_back(FieldDefinition(name, FieldType::BOOL, offset));
            return *this;
        }
        
        // 数组字段（使用 std::vector）
            template<typename U>
            Schema& array(std::vector<U> T::*field, const std::string& name) {
                size_t offset = getMemberOffset(field);
                fields_.push_back(FieldDefinition(name, FieldType::ARRAY, offset));
                return *this;
            }
            
            // 带元素类型的数组字段定义
            template<typename U>
            Schema& arrayOf(std::vector<U> T::*field, const std::string& name, FieldType element_type) {
                size_t offset = getMemberOffset(field);
                FieldDefinition def(name, FieldType::ARRAY, offset);
                def.element_type = element_type;  // 存储元素类型
                fields_.push_back(def);
                return *this;
            }
            
            // 字符串数组
            Schema& arrayOfString(std::vector<std::string> T::*field, const std::string& name) {
                return arrayOf(field, name, FieldType::STRING);
            }
            
            // 整数数组
            Schema& arrayOfInt(std::vector<int32_t> T::*field, const std::string& name) {
                return arrayOf(field, name, FieldType::INT32);
            }
            
            // 64位整数数组
            Schema& arrayOfInt64(std::vector<int64_t> T::*field, const std::string& name) {
                return arrayOf(field, name, FieldType::INT64);
            }
            
            // 浮点数数组
            Schema& arrayOfNumber(std::vector<double> T::*field, const std::string& name) {
                return arrayOf(field, name, FieldType::FP64);
            }
            
            // 布尔数组
            Schema& arrayOfBool(std::vector<bool> T::*field, const std::string& name) {
                return arrayOf(field, name, FieldType::BOOL);
            }            
            // ========== 高级数据类型 ==========
            
            // 日期类型（YYYY-MM-DD）
            Schema& date(std::string T::*field, const std::string& name) {
                size_t offset = getMemberOffset(field);
                fields_.push_back(FieldDefinition(name, FieldType::DATE, offset));
                return *this;
            }
            
            // 日期时间类型（YYYY-MM-DD HH:MM:SS）
            Schema& datetime(std::string T::*field, const std::string& name) {
                size_t offset = getMemberOffset(field);
                fields_.push_back(FieldDefinition(name, FieldType::DATETIME, offset));
                return *this;
            }
            
            // 邮箱类型
            Schema& email(std::string T::*field, const std::string& name) {
                size_t offset = getMemberOffset(field);
                fields_.push_back(FieldDefinition(name, FieldType::EMAIL, offset));
                return *this;
            }
            
            // URL 类型
            Schema& url(std::string T::*field, const std::string& name) {
                size_t offset = getMemberOffset(field);
                fields_.push_back(FieldDefinition(name, FieldType::URL, offset));
                return *this;
            }
            
            // UUID 类型
            Schema& uuid(std::string T::*field, const std::string& name) {
                size_t offset = getMemberOffset(field);
                fields_.push_back(FieldDefinition(name, FieldType::UUID, offset));
                return *this;
            }
            
            // 自定义类型
                Schema& custom(std::string T::*field, const std::string& name, ICustomTypeHandler* handler) {
                    size_t offset = getMemberOffset(field);
                    FieldDefinition def(name, FieldType::CUSTOM, offset);
                    def.custom_handler = handler;
                    fields_.push_back(def);
                    return *this;
                }        }
        
        Schema& number(double T::*field, const std::string& name) {
            return fp64(field, name);
        }    
    // ========== 链式配置方法 ==========
    
    // 必填
    Schema& required() {
        if (!fields_.empty()) {
            fields_.back().validation.required = true;
        }
        return *this;
    }
    
    // 可选
    Schema& optional() {
        if (!fields_.empty()) {
            fields_.back().validation.required = false;
        }
        return *this;
    }
    
    // 最小长度
    Schema& minLength(int len) {
        if (!fields_.empty()) {
            fields_.back().validation.min_length = len;
            fields_.back().validation.has_min_length = true;
        }
        return *this;
    }
    
    // 最大长度
    Schema& maxLength(int len) {
        if (!fields_.empty()) {
            fields_.back().validation.max_length = len;
            fields_.back().validation.has_max_length = true;
        }
        return *this;
    }
    
    // 长度范围
    Schema& length(int min_len, int max_len) {
        return minLength(min_len).maxLength(max_len);
    }
    
    // 最小值
    Schema& min(double val) {
        if (!fields_.empty()) {
            fields_.back().validation.min_value = val;
            fields_.back().validation.has_min_value = true;
        }
        return *this;
    }
    
    // 最大值
    Schema& max(double val) {
        if (!fields_.empty()) {
            fields_.back().validation.max_value = val;
            fields_.back().validation.has_max_value = true;
        }
        return *this;
    }
    
    // 数值范围
    Schema& range(double min_val, double max_val) {
        return min(min_val).max(max_val);
    }
    
    // 正则表达式
    Schema& pattern(const std::string& regex) {
        if (!fields_.empty()) {
            fields_.back().validation.pattern = regex;
            fields_.back().validation.has_pattern = true;
        }
        return *this;
    }
    
    // 枚举值
    Schema& oneOf(const std::vector<std::string>& values) {
        if (!fields_.empty()) {
            fields_.back().validation.enum_values = values;
            fields_.back().validation.has_enum = true;
        }
        return *this;
    }
    
    // 枚举值（可变参数）
    Schema& oneOf(const std::string& v1, const std::string& v2 = "", 
                    const std::string& v3 = "", const std::string& v4 = "") {
        std::vector<std::string> values;
        if (!v1.empty()) values.push_back(v1);
        if (!v2.empty()) values.push_back(v2);
        if (!v3.empty()) values.push_back(v3);
        if (!v4.empty()) values.push_back(v4);
        return oneOf(values);
    }
    
    // 获取字段定义
    const std::vector<FieldDefinition>& fields() const {
        return fields_;
    }
    
private:
    // 计算成员指针的偏移量
    template<typename MemberType>
    static size_t getMemberOffset(MemberType T::*field) {
        // 使用 nullptr 获取偏移量
        return reinterpret_cast<size_t>(&(reinterpret_cast<T*>(0)->*field));
    }
};

// ========== DslBodySchema 扩展 ==========

// 基类 Schema 定义器
template<typename T>
class DslBodySchema : public BodySchemaBase {
public:
    DslBodySchema() : BodySchemaBase(typeid(T).name()) {}
    
    // 字段定义辅助方法
    DslBodySchema& field(std::string T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::STRING, offset);
        return *this;
    }
    
    DslBodySchema& field(int8_t T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::INT8, offset);
        return *this;
    }
    
    DslBodySchema& field(int16_t T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::INT16, offset);
        return *this;
    }
    
    DslBodySchema& field(int32_t T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::INT32, offset);
        return *this;
    }
    
    DslBodySchema& field(int64_t T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::INT64, offset);
        return *this;
    }
    
    DslBodySchema& field(uint8_t T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::UINT8, offset);
        return *this;
    }
    
    DslBodySchema& field(uint16_t T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::UINT16, offset);
        return *this;
    }
    
    DslBodySchema& field(uint32_t T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::UINT32, offset);
        return *this;
    }
    
    DslBodySchema& field(uint64_t T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::UINT64, offset);
        return *this;
    }
    
    DslBodySchema& field(float T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::FP32, offset);
        return *this;
    }
    
    DslBodySchema& field(double T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::FP64, offset);
        return *this;
    }
    
    DslBodySchema& field(bool T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::BOOL, offset);
        return *this;
    }
    
    template<typename U>
    DslBodySchema& field(std::vector<U> T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::ARRAY, offset);
        return *this;
    }
    
    // 兼容旧代码
    DslBodySchema& field(int T::*member, const std::string& name) {
        return field((int32_t T::*)member, name);
    }
    
    // ========== 高级数据类型字段定义 ==========
    
    DslBodySchema& fieldDate(std::string T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::DATE, offset);
        return *this;
    }
    
    DslBodySchema& fieldDatetime(std::string T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::DATETIME, offset);
        return *this;
    }
    
    DslBodySchema& fieldEmail(std::string T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::EMAIL, offset);
        return *this;
    }
    
    DslBodySchema& fieldUrl(std::string T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::URL, offset);
        return *this;
    }
    
    DslBodySchema& fieldUuid(std::string T::*member, const std::string& name) {
        size_t offset = getMemberOffset(member);
        addField(name, FieldType::UUID, offset);
        return *this;
    }
    
    DslBodySchema& fieldCustom(std::string T::*member, const std::string& name, ICustomTypeHandler* handler) {
        size_t offset = getMemberOffset(member);
        FieldDefinition def(name, FieldType::CUSTOM, offset);
        def.custom_handler = handler;
        if (fields_.empty()) {
            fields_.push_back(def);
        } else {
            fields_.back() = def;
        }
        return *this;
    }
    
    // 类型转换辅助方法
    DslBodySchema& asString() {
        setFieldType(FieldType::STRING);
        return *this;
    }
    
    DslBodySchema& asInt8() {
        setFieldType(FieldType::INT8);
        return *this;
    }
    
    DslBodySchema& asInt16() {
        setFieldType(FieldType::INT16);
        return *this;
    }
    
    DslBodySchema& asInt32() {
        setFieldType(FieldType::INT32);
        return *this;
    }
    
    DslBodySchema& asInt64() {
        setFieldType(FieldType::INT64);
        return *this;
    }
    
    DslBodySchema& asUint8() {
        setFieldType(FieldType::UINT8);
        return *this;
    }
    
    DslBodySchema& asUint16() {
        setFieldType(FieldType::UINT16);
        return *this;
    }
    
    DslBodySchema& asUint32() {
        setFieldType(FieldType::UINT32);
        return *this;
    }
    
    DslBodySchema& asUint64() {
        setFieldType(FieldType::UINT64);
        return *this;
    }
    
    DslBodySchema& asFp32() {
        setFieldType(FieldType::FP32);
        return *this;
    }
    
    DslBodySchema& asFp64() {
        setFieldType(FieldType::FP64);
        return *this;
    }
    
    DslBodySchema& asBool() {
        setFieldType(FieldType::BOOL);
        return *this;
    }
    
    // 兼容旧代码
    DslBodySchema& asInt() {
        return asInt32();
    }
    
    DslBodySchema& asNumber() {
        return asFp64();
    }
    
    // ========== 高级数据类型转换 ==========
    
    DslBodySchema& asDate() {
        setFieldType(FieldType::DATE);
        return *this;
    }
    
    DslBodySchema& asDatetime() {
        setFieldType(FieldType::DATETIME);
        return *this;
    }
    
    DslBodySchema& asEmail() {
        setFieldType(FieldType::EMAIL);
        return *this;
    }
    
    DslBodySchema& asUrl() {
        setFieldType(FieldType::URL);
        return *this;
    }
    
    DslBodySchema& asUuid() {
        setFieldType(FieldType::UUID);
        return *this;
    }
    
    DslBodySchema& asCustom(ICustomTypeHandler* handler) {
        setFieldType(FieldType::CUSTOM);
        if (!fields_.empty()) {
            fields_.back().custom_handler = handler;
        }
        return *this;
    }
    
    // 验证规则
    DslBodySchema& required() {
        setRequired(true);
        return *this;
    }
    
    DslBodySchema& optional() {
        setRequired(false);
        return *this;
    }
    
    DslBodySchema& minLength(int len) {
        setMinLength(len);
        return *this;
    }
    
    DslBodySchema& maxLength(int len) {
        setMaxLength(len);
        return *this;
    }
    
    DslBodySchema& length(int min_len, int max_len) {
        return minLength(min_len).maxLength(max_len);
    }
    
    DslBodySchema& min(double val) {
        setMinValue(val);
        return *this;
    }
    
    DslBodySchema& max(double val) {
        setMaxValue(val);
        return *this;
    }
    
    DslBodySchema& range(double min_val, double max_val) {
        return min(min_val).max(max_val);
    }
    
    DslBodySchema& pattern(const std::string& regex) {
        setPattern(regex);
        return *this;
    }
    
    DslBodySchema& oneOf(const std::vector<std::string>& values) {
        setEnumValues(values);
        return *this;
    }
    
    DslBodySchema& oneOf(const std::string& v1, const std::string& v2 = "", 
                          const std::string& v3 = "", const std::string& v4 = "") {
        std::vector<std::string> values;
        if (!v1.empty()) values.push_back(v1);
        if (!v2.empty()) values.push_back(v2);
        if (!v3.empty()) values.push_back(v3);
        if (!v4.empty()) values.push_back(v4);
        return oneOf(values);
    }
    
private:
    template<typename MemberType>
    static size_t getMemberOffset(MemberType T::*field) {
        return reinterpret_cast<size_t>(&(reinterpret_cast<T*>(0)->*field));
    }
    
    void addField(const std::string& name, FieldType type, size_t offset) {
        if (fields_.empty()) {
            fields_.push_back(FieldDefinition(name, type, offset));
        } else {
            // 更新最后一个字段的类型
            fields_.back().name = name;
            fields_.back().type = type;
            fields_.back().offset = offset;
        }
    }
    
    void setFieldType(FieldType type) {
        if (!fields_.empty()) {
            fields_.back().type = type;
        }
    }
    
    void setRequired(bool required) {
        if (!fields_.empty()) {
            fields_.back().validation.required = required;
        }
    }
    
    void setMinLength(int len) {
        if (!fields_.empty()) {
            fields_.back().validation.min_length = len;
            fields_.back().validation.has_min_length = true;
        }
    }
    
    void setMaxLength(int len) {
        if (!fields_.empty()) {
            fields_.back().validation.max_length = len;
            fields_.back().validation.has_max_length = true;
        }
    }
    
    void setMinValue(double val) {
        if (!fields_.empty()) {
            fields_.back().validation.min_value = val;
            fields_.back().validation.has_min_value = true;
        }
    }
    
    void setMaxValue(double val) {
        if (!fields_.empty()) {
            fields_.back().validation.max_value = val;
            fields_.back().validation.has_max_value = true;
        }
    }
    
    void setPattern(const std::string& regex) {
        if (!fields_.empty()) {
            fields_.back().validation.pattern = regex;
            fields_.back().validation.has_pattern = true;
        }
    }
    
    void setEnumValues(const std::vector<std::string>& values) {
        if (!fields_.empty()) {
            fields_.back().validation.enum_values = values;
            fields_.back().validation.has_enum = true;
        }
    }
};

} // namespace uvapi

#endif // SCHEMA_DSL_H