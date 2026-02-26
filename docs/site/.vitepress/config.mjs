import { defineConfig } from 'vitepress'

export default defineConfig({
  title: 'UVAPI',
  description: 'A high-performance RESTful framework built on UVHTTP',

  // 忽略 localhost 链接检查
  ignoreDeadLinks: true,

  // GitHub Pages 基础路径
  base: '/',

  locales: {
    root: {
      label: '简体中文',
      lang: 'zh-CN'
    },
    en: {
      label: 'English',
      lang: 'en-US'
    }
  },

  themeConfig: {
    // 语言下拉菜单
    languageDropdowns: [
      {
        text: '简体中文',
        link: '/zh/',
        items: [
          { text: 'English', link: '/en/' }
        ]
      },
      {
        text: 'English',
        link: '/en/',
        items: [
          { text: '简体中文', link: '/zh/' }
        ]
      }
    ],

    // 导航栏
    nav: [
      {
        text: 'Guide',
        items: [
          { text: 'Quick Start', link: '/zh/guide/quick-start' },
          { text: 'Response DSL', link: '/zh/guide/response-dsl' },
          { text: 'JSON Usage', link: '/zh/guide/json-usage' }
        ]
      },
      {
        text: 'API Reference',
        link: '/zh/api/'
      },
      {
        text: 'Examples',
        link: '/zh/examples/'
      },
      {
        text: 'Design',
        link: '/zh/design/'
      }
    ],

    // 社交链接
    socialLinks: [
      { icon: 'github', link: 'https://github.com/adam-ikari/uvapi' }
    ],

    // 页脚
    footer: {
      message: 'Released under the MIT License.',
      copyright: 'Copyright © 2026-present'
    }
  },

  // 侧边栏配置
  sidebar: {
    '/zh/guide/': [
      {
        text: 'Guide',
        items: [
          { text: 'Quick Start', link: '/zh/guide/quick-start' },
          { text: 'Response DSL', link: '/zh/guide/response-dsl' },
          { text: 'JSON Usage', link: '/zh/guide/json-usage' }
        ]
      }
    ],
    '/zh/api/': [
      {
        text: 'API Reference',
        items: [
          { text: 'Framework', link: '/zh/api/framework' },
          { text: 'Response DSL', link: '/zh/api/response-dsl' },
          { text: 'JSON', link: '/zh/api/json' }
        ]
      }
    ],
    '/zh/examples/': [
      {
        text: 'Examples',
        items: [
          { text: 'Response DSL', link: '/zh/examples/response-dsl' },
          { text: 'JSON Usage', link: '/zh/examples/json-usage' }
        ]
      }
    ],
    '/zh/design/': [
      {
        text: 'Design',
        items: [
          { text: 'DSL Design', link: '/zh/design/dsl-design' },
          { text: 'Architecture', link: '/zh/design/architecture' }
        ]
      }
    ],
    '/en/guide/': [
      {
        text: 'Guide',
        items: [
          { text: 'Quick Start', link: '/en/guide/quick-start' },
          { text: 'Response DSL', link: '/en/guide/response-dsl' },
          { text: 'JSON Usage', link: '/en/guide/json-usage' }
        ]
      }
    ]
  }
})