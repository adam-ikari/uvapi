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
    ${PROJECT_ROOT}/deps/uvhttp/deps/mbedtls/library
    ${PROJECT_ROOT}/deps/uvhttp/build/dist/lib
    ${PROJECT_ROOT}/deps/uvhttp/dist/lib
)

# Benchmark 性能测试服务器
add_executable(benchmark_server
    examples/benchmark_server.cpp
    src/framework_uvhttp.cpp
)

# 多服务器测试
add_executable(test_multi_server
    examples/test_multi_server.cpp
    src/framework_uvhttp.cpp
)

# 测试服务器
add_executable(test_server
    examples/test_server.cpp
    src/framework_uvhttp.cpp
)

# 链接库
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

target_link_libraries(test_multi_server
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

target_link_libraries(test_server
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
target_compile_options(test_multi_server PRIVATE -Wall -Wextra -O2)
target_compile_options(test_server PRIVATE -Wall -Wextra -O2)
