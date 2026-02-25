cmake_minimum_required(VERSION 3.10)
project(uvapi CXX)

# 设置构建类型
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

# 设置分配器类型
set(UVHTTP_ALLOCATOR_TYPE 1 CACHE STRING "Allocator type: 0=system, 1=mimalloc, 2=custom")

# 编译警告选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow -Wold-style-cast")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align -Wunused")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -Wnon-virtual-dtor")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnull-dereference")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdouble-promotion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmissing-include-dirs")

# 在 Debug 模式下启用地址清理器和未定义行为清理器
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置项目根目录
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# 包含目录
include_directories(
    ${PROJECT_ROOT}/include
    ${PROJECT_ROOT}/deps/uvhttp/include
    ${PROJECT_ROOT}/deps/uvhttp/deps/cjson
    ${PROJECT_ROOT}/deps/uvhttp/deps/libuv/include
    ${PROJECT_ROOT}/deps/uvhttp/deps/cllhttp
    ${PROJECT_ROOT}/deps/uvhttp/deps/mbedtls/include
    ${PROJECT_ROOT}/deps/uvhttp/deps/uthash/src
)

# 链接库目录
link_directories(
    ${PROJECT_ROOT}/deps/uvhttp/deps/libuv/build
    ${PROJECT_ROOT}/deps/uvhttp/deps/cjson/build
    ${PROJECT_ROOT}/deps/uvhttp/deps/cllhttp/build
    ${PROJECT_ROOT}/deps/uvhttp/deps/mbedtls/build/library
    ${PROJECT_ROOT}/deps/uvhttp/deps/mimalloc/build
    ${PROJECT_ROOT}/deps/uvhttp/build/dist/lib
    ${PROJECT_ROOT}/deps/uvhttp/dist/lib
)

# Benchmark 性能测试服务器
# add_executable(benchmark_server
#     examples/benchmark_server.cpp
#     src/framework_uvhttp.cpp
#     src/multipart.cpp
# )

# # CRUD 服务器示例 - 需要更新 DSL API
# add_executable(crud_server
#     examples/crud_server.cpp
#     src/framework_uvhttp.cpp
#     src/multipart.cpp
# )

# # 认证服务器示例 - 需要更新 DSL API
# add_executable(auth_server
#     examples/auth_server.cpp
#     src/framework_uvhttp.cpp
#     src/multipart.cpp
# )

# # 参数示例 - 需要更新 DSL API
# add_executable(params_example
#     examples/params_example.cpp
#     src/framework_uvhttp.cpp
#     src/multipart.cpp
# )

# UVHTTP 中间件示例
add_executable(uvhttp_middleware_example
    examples/uvhttp_middleware_example.cpp
    src/framework_uvhttp.cpp
    src/multipart.cpp
)

# # DSL 示例 - 需要更新 DSL API
# add_executable(dsl_example
#     examples/dsl_example.cpp
#     src/framework_uvhttp.cpp
#     src/multipart.cpp
# )

# # Schema 示例 - 需要更新 DSL API
# add_executable(schema_example
#     examples/schema_example.cpp
#     src/framework_uvhttp.cpp
#     src/multipart.cpp
# )

# 文件上传示例
add_executable(upload_example
    examples/upload_example.cpp
    src/framework_uvhttp.cpp
    src/multipart.cpp
)

# 静态文件示例 - 需要实现应用层静态文件服务
# add_executable(static_files_example
#     examples/static_files_example.cpp
#     src/framework_uvhttp.cpp
#     src/multipart.cpp
# )

# target_link_libraries(static_files_example
#     -Wl,--start-group
#     uvhttp
#     uv
#     llhttp
#     cjson
#     mbedtls
#     mbedx509
#     mbedcrypto
#     mimalloc
#     -Wl,--end-group
#     pthread
#     dl
# )

# 声明式 DSL 示例
add_executable(declarative_dsl_example
    examples/declarative_dsl_example.cpp
    src/framework_uvhttp.cpp
    src/multipart.cpp
)

# 声明式 DSL 自动解析示例
add_executable(declarative_dsl_auto_parse
    examples/declarative_dsl_auto_parse.cpp
    src/framework_uvhttp.cpp
    src/multipart.cpp
)

# JSON 使用示例
add_executable(json_usage_example
    examples/json_usage_example.cpp
    src/framework_uvhttp.cpp
    src/multipart.cpp
)

# 性能测试服务器
add_executable(benchmark_server
    examples/benchmark_server.cpp
    src/framework_uvhttp.cpp
    src/multipart.cpp
)

# 链接库
target_link_libraries(declarative_dsl_example
    -Wl,--start-group
    uvhttp
    uv
    llhttp
    cjson
    mbedtls
    mbedx509
    mbedcrypto
    mimalloc
    -Wl,--end-group
    pthread
    dl
)

target_link_libraries(declarative_dsl_auto_parse
    -Wl,--start-group
    uvhttp
    uv
    llhttp
    cjson
    mbedtls
    mbedx509
    mbedcrypto
    mimalloc
    -Wl,--end-group
    pthread
    dl
)

target_link_libraries(json_usage_example
    -Wl,--start-group
    uvhttp
    uv
    llhttp
    cjson
    mbedtls
    mbedx509
    mbedcrypto
    mimalloc
    -Wl,--end-group
    pthread
    dl
)

target_link_libraries(benchmark_server
    -Wl,--start-group
    uvhttp
    uv
    llhttp
    cjson
    mbedtls
    mbedx509
    mbedcrypto
    mimalloc
    -Wl,--end-group
    pthread
    dl
)

target_link_libraries(uvhttp_middleware_example
    -Wl,--start-group
    uvhttp
    uv
    llhttp
    cjson
    mbedtls
    mbedx509
    mbedcrypto
    mimalloc
    -Wl,--end-group
    pthread
    dl
)

target_link_libraries(upload_example
    -Wl,--start-group
    uvhttp
    uv
    llhttp
    cjson
    mbedtls
    mbedx509
    mbedcrypto
    mimalloc
    -Wl,--end-group
    pthread
    dl
)

# ========== 单元测试 ==========

# 启用测试
enable_testing()

# 声明式 DSL 自动解析单元测试
add_executable(test_declarative_dsl_auto_parse
    test/unit/test_declarative_dsl_auto_parse.cpp
    src/framework_uvhttp.cpp
)

target_link_libraries(test_declarative_dsl_auto_parse
    -Wl,--start-group
    uvhttp
    uv
    llhttp
    cjson
    mbedtls
    mbedx509
    mbedcrypto
    mimalloc
    -Wl,--end-group
    pthread
    dl
)

# 添加测试用例
add_test(NAME declarative_dsl_auto_parse_test COMMAND test_declarative_dsl_auto_parse)

# 编译选项已在全局设置 CMAKE_CXX_FLAGS 中定义