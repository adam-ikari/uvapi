/**
 * @file uvapi_allocator.h
 * @brief UVAPI 统一内存分配器
 * 
 * 提供统一的内存分配接口，支持编译期选择分配器类型：
 * - UVAPI_ALLOCATOR_TYPE == 0: 系统分配器（malloc/free）
 * - UVAPI_ALLOCATOR_TYPE == 1: mimalloc 分配器（通过 UVHTTP）
 * - UVAPI_ALLOCATOR_TYPE == 2: 自定义分配器（需要应用层实现）
 */

#ifndef UVAPI_ALLOCATOR_H
#define UVAPI_ALLOCATOR_H

#include <stddef.h>
#include <stdlib.h>

// 默认使用系统分配器
#ifndef UVAPI_ALLOCATOR_TYPE
#define UVAPI_ALLOCATOR_TYPE 0
#endif

#if UVAPI_ALLOCATOR_TYPE == 1
// 使用 mimalloc 分配器（通过 UVHTTP）
// 需要链接 uvhttp 库
#include "uvhttp/uvhttp.h"
#endif

namespace uvapi {

/**
 * @brief 分配内存
 * @param size 要分配的字节数
 * @return 分配的内存指针，失败返回 NULL
 */
static inline void* uvapi_alloc(size_t size) {
#if UVAPI_ALLOCATOR_TYPE == 1
    return uvhttp_alloc(size);
#elif UVAPI_ALLOCATOR_TYPE == 2
    // 自定义分配器 - 需要应用层实现
    extern void* uvapi_custom_alloc(size_t size);
    return uvapi_custom_alloc(size);
#else
    // 系统分配器（默认）
    return malloc(size);
#endif
}

/**
 * @brief 释放内存
 * @param ptr 要释放的内存指针
 */
static inline void uvapi_free(void* ptr) {
#if UVAPI_ALLOCATOR_TYPE == 1
    uvhttp_free(ptr);
#elif UVAPI_ALLOCATOR_TYPE == 2
    // 自定义分配器 - 需要应用层实现
    extern void uvapi_custom_free(void* ptr);
    uvapi_custom_free(ptr);
#else
    // 系统分配器（默认）
    free(ptr);
#endif
}

/**
 * @brief 重新分配内存
 * @param ptr 原内存指针
 * @param size 新的大小
 * @return 重新分配的内存指针，失败返回 NULL
 */
static inline void* uvapi_realloc(void* ptr, size_t size) {
#if UVAPI_ALLOCATOR_TYPE == 1
    return uvhttp_realloc(ptr, size);
#elif UVAPI_ALLOCATOR_TYPE == 2
    // 自定义分配器 - 需要应用层实现
    extern void* uvapi_custom_realloc(void* ptr, size_t size);
    return uvapi_custom_realloc(ptr, size);
#else
    // 系统分配器（默认）
    return realloc(ptr, size);
#endif
}

/**
 * @brief 分配并清零内存
 * @param count 元素数量
 * @param size 每个元素的大小
 * @return 分配的内存指针，失败返回 NULL
 */
static inline void* uvapi_calloc(size_t count, size_t size) {
#if UVAPI_ALLOCATOR_TYPE == 1
    return uvhttp_calloc(count, size);
#elif UVAPI_ALLOCATOR_TYPE == 2
    // 自定义分配器 - 需要应用层实现
    extern void* uvapi_custom_calloc(size_t count, size_t size);
    return uvapi_custom_calloc(count, size);
#else
    // 系统分配器（默认）
    return calloc(count, size);
#endif
}

/**
 * @brief 获取分配器名称
 * @return 分配器名称字符串
 */
static inline const char* uvapi_allocator_name() {
#if UVAPI_ALLOCATOR_TYPE == 1
    return "mimalloc (via UVHTTP)";
#elif UVAPI_ALLOCATOR_TYPE == 2
    return "custom";
#else
    return "system (malloc/free)";
#endif
}

} // namespace uvapi

#endif // UVAPI_ALLOCATOR_H