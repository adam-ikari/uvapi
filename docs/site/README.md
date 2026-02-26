# UVAPI 文档网站

这是一个基于 VitePress 的文档网站，提供中英双语支持。

## 快速开始

### 安装依赖

```bash
cd docs/site
npm install
```

### 启动开发服务器

```bash
npm run docs:dev
```

访问 http://localhost:5173/ 或 http://localhost:5174/ 查看文档网站（具体端口取决于可用性）。注意：这些 URL 仅在开发服务器运行时可用。

### 构建静态网站

```bash
npm run docs:build
```

构建后的静态文件将在 `docs/site/.vitepress/dist/` 目录中。

### 预览构建结果

```bash
npm run docs:preview
```

## 文档结构

```
docs/site/
├── index.md              # 首页
├── guide/                # 指南文档
│   ├── quick-start.md    # 快速开始
│   ├── response-dsl.md   # Response DSL 指南
│   └── json-usage.md     # JSON 使用指南
├── api/                  # API 参考
├── examples/             # 示例代码
└── design/               # 设计文档
```

## 设计哲学

1. **文档和网站内容完全一致** - 文档内容是网站的唯一源
2. **支持中英文两个版本的文档** - 所有文档都提供中英双语版本
3. **网站支持中英文切换** - 用户可以轻松切换语言
4. **文档提供覆盖常见场景的使用示例** - 提供实用的代码示例
5. **提供快速上手的文档** - 帮助用户快速开始
6. **详细使用方法有一个系列教程** - 从基础到高级的完整教程

## 添加新文档

### 添加中文文档

在相应的目录下创建 `.md` 文件，例如 `guide/new-feature.md`。

### 添加英文文档

在 `en/` 子目录下创建对应的 `.md` 文件，例如 `en/guide/new-feature.md`。

### 更新配置

在 `.vitepress/config.mjs` 中更新导航栏和侧边栏配置。

## 技术栈

- **VitePress**: 基于 Vue 的静态网站生成器
- **Markdown**: 使用 Markdown 格式编写文档
- **Node.js**: 需要 Node.js 16 或更高版本

## 注意事项

- 所有文档使用 UTF-8 编码
- 文件名使用小写字母和连字符（kebab-case）
- 确保中文和英文文档同步更新