# DSL 使用方法

## 基本语法

声明式 DSL 使用初始化列表语法，验证规则作为参数的属性。

```cpp
#include "declarative_dsl.h"

using namespace uvapi::declarative;

ParamGroup params = {
    Int("page", false, 1).range(1, 1000),
    Int("limit", false, 10).range(1, 100)
};
```

## 参数定义函数

### 整数参数

```cpp
Int(name, required, default_value)
```

- `name`: 参数名
- `required`: 是否必需（true/false）
- `default_value`: 默认值

示例：
```cpp
Int("page", false, 1)      // 可选，默认值 1
Int("id", true, 0)         // 必需，默认值 0
```

### 字符串参数

```cpp
String(name, required, default_value)
```

示例：
```cpp
String("search", false, "")       // 可选，默认值 ""
String("username", true, "")      // 必需
```

### 布尔参数

```cpp
Bool(name, required, default_value)
```

示例：
```cpp
Bool("active", false, true)   // 可选，默认值 true
Bool("verified", true, false) // 必需，默认值 false
```

### 双精度浮点参数

```cpp
Double(name, required, default_value)
```

示例：
```cpp
Double("price", false, 0.0)   // 可选，默认值 0.0
Double("rate", true, 1.0)     // 必需，默认值 1.0
```

## 验证规则

### 范围验证

```cpp
ParamDef.range(min, max)
```

适用于整数和浮点数参数。

示例：
```cpp
Int("page", false, 1).range(1, 1000)
Double("price", false, 0.0).range(0.0, 1000000.0)
```

### 长度验证

```cpp
ParamDef.length(min, max)
```

适用于字符串参数。

示例：
```cpp
String("username", true, "").length(3, 20)
```

### 正则验证

```cpp
ParamDef.pattern(regex)
```

适用于字符串参数。

示例：
```cpp
String("email", true, "").pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")
```

### 枚举验证

```cpp
ParamDef.oneOf({value1, value2, ...})
```

适用于所有类型参数。

示例：
```cpp
String("status", false, "active").oneOf({"active", "inactive", "pending"})
Int("level", false, 1).oneOf({1, 2, 3})
```

## 完整示例

### 用户列表 API

```cpp
ParamGroup userListParams = {
    Int("page", false, 1).range(1, 1000),
    Int("limit", false, 10).range(1, 100),
    String("status", false, "active").oneOf({"active", "inactive", "pending"}),
    String("search", false, "")
};
```

### 用户创建 API

```cpp
ParamGroup createUserParams = {
    String("username", true, "").length(3, 20),
    String("email", true, "").pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"),
    Int("age", false, 18).range(18, 120),
    Bool("active", false, true)
};
```

### 产品列表 API

```cpp
ParamGroup productListParams = {
    Int("page", false, 1).range(1, 1000),
    Int("limit", false, 20).range(1, 100),
    String("sort", false, "id").oneOf({"id", "name", "price", "created_at"}),
    String("order", false, "asc").oneOf({"asc", "desc"}),
    Int("min_price", false, 0).range(0, 1000000),
    Int("max_price", false, 1000000).range(0, 1000000),
    String("category", false, "").oneOf({"electronics", "clothing", "books", "food", "other"})
};
```

## 组合规则

1. **可选参数可以设置默认值**
   ```cpp
   Int("page", false, 1)  // 可选，默认值 1
   ```

2. **必需参数不能设置默认值**
   ```cpp
   Int("id", true, 0)  // 必需，默认值无效
   ```

3. **验证规则可以组合**
   ```cpp
   length(pattern(String("username", true, ""), "^[a-z]+$"), 3, 20)
   ```

4. **初始化列表中的参数顺序不影响验证**
   ```cpp
   ParamGroup params = {
       String("search", false, ""),  // 顺序无关
       range(Int("page", false, 1), 1, 1000)
   };
   ```

## 参数类型说明

| 类型 | 函数 | 说明 |
|------|------|------|
| 整数 | `Int()` | 32位有符号整数 |
| 字符串 | `String()` | 字符串 |
| 布尔 | `Bool()` | 布尔值 |
| 双精度浮点 | `Double()` | 64位浮点数 |

## 验证规则说明

| 规则 | 函数 | 适用类型 |
|------|------|----------|
| 范围 | `range()` | 整数、浮点数 |
| 长度 | `length()` | 字符串 |
| 正则 | `pattern()` | 字符串 |
| 枚举 | `oneOf()` | 所有类型 |