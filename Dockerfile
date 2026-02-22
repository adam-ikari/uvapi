# 多阶段构建 Dockerfile

# ========== 构建阶段 ==========
FROM ubuntu:22.04 AS builder

# 安装构建依赖
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /build

# 复制项目文件
COPY . .

# 初始化子模块
RUN git submodule update --init --recursive

# 构建 UVHTTP 依赖
RUN cd deps/uvhttp && \
    mkdir -p build && cd build && \
    cmake .. -DUVHTTP_ALLOCATOR_TYPE=1 && \
    make -j$(nproc)

# 构建 UVAPI
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# ========== 运行阶段 ==========
FROM ubuntu:22.04

# 安装运行时依赖
RUN apt-get update && apt-get install -y \
    libuv1 \
    libmbedtls10 \
    && rm -rf /var/lib/apt/lists/*

# 创建非 root 用户
RUN useradd -m -u 1000 uvapi

# 设置工作目录
WORKDIR /app

# 从构建阶段复制必要的文件
COPY --from=builder /build/build/dist/bin/ /app/bin/
COPY --from=builder /build/build/dist/lib/ /app/lib/
COPY --from=builder /build/deps/uvhttp/build/dist/lib/ /app/lib/

# 设置库路径
ENV LD_LIBRARY_PATH=/app/lib:$LD_LIBRARY_PATH

# 切换到非 root 用户
USER uvapi

# 暴露端口
EXPOSE 8080

# 健康检查
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# 启动应用
CMD ["/app/bin/your_app"]