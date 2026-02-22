cmake_minimum_required(VERSION 3.10)
project(uvapi CXX)

# 设置构建类型
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

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

# 添加示例程序以测试
add_executable(auth_server
    examples/auth_server.cpp
    src/framework_uvhttp.cpp
    src/multipart.cpp
)

add_executable(crud_server
    examples/crud_server.cpp
    src/framework_uvhttp.cpp
    src/multipart.cpp
)

add_executable(dsl_example
    examples/dsl_example.cpp
    src/framework_uvhttp.cpp
    src/multipart.cpp
)

add_executable(schema_example
    examples/schema_example.cpp
    src/framework_uvhttp.cpp
    src/multipart.cpp
)

add_executable(schema_dsl_example
    examples/schema_dsl_example.cpp
    src/framework_uvhttp.cpp
    src/multipart.cpp
)

add_executable(declarative_dsl_example
    examples/declarative_dsl_example.cpp
    src/framework_uvhttp.cpp
    src/multipart.cpp
)

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
    -Wl,--end-group
    pthread
    dl
)

target_link_libraries(crud_server
    -Wl,--start-group
    uvhttp
    uv
    llhttp
    cjson
    mbedtls
    mbedx509
    mbedcrypto
    -Wl,--end-group
    pthread
    dl
)

target_link_libraries(schema_dsl_example
    -Wl,--start-group
    uvhttp
    uv
    llhttp
    cjson
    mbedtls
    mbedx509
    mbedcrypto
    -Wl,--end-group
    pthread
    dl
)

target_link_libraries(auth_server
    -Wl,--start-group
    uvhttp
    uv
    llhttp
    cjson
    mbedtls
    mbedx509
    mbedcrypto
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
    -Wl,--end-group
    pthread
    dl
)

target_link_libraries(dsl_example
    -Wl,--start-group
    uvhttp
    uv
    llhttp
    cjson
    mbedtls
    mbedx509
    mbedcrypto
    -Wl,--end-group
    pthread
    dl
)

target_link_libraries(schema_example
    -Wl,--start-group
    uvhttp
    uv
    llhttp
    cjson
    mbedtls
    mbedx509
    mbedcrypto
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
    -Wl,--end-group
    pthread
    dl
)

# 编译选项
target_compile_options(benchmark_server PRIVATE -Wall -Wextra -O2)
target_compile_options(crud_server PRIVATE -Wall -Wextra -O2)
target_compile_options(auth_server PRIVATE -Wall -Wextra -O2)
target_compile_options(uvhttp_middleware_example PRIVATE -Wall -Wextra -O2)
target_compile_options(dsl_example PRIVATE -Wall -Wextra -O2)
target_compile_options(declarative_dsl_example PRIVATE -Wall -Wextra -O2)
target_compile_options(schema_dsl_example PRIVATE -Wall -Wextra -O2)
target_compile_options(schema_example PRIVATE -Wall -Wextra -O2)
target_compile_options(upload_example PRIVATE -Wall -Wextra -O2)