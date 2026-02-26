import { defineConfig } from 'vitepress'

export default defineConfig({
  title: 'UVAPI',
  description: 'A high-performance RESTful framework built on UVHTTP',

  // 忽略 localhost 链接检查
  ignoreDeadLinks: true,

  // GitHub Pages 基础路径
  base: '/uvapi/',

  themeConfig: {
    // 导航栏
    nav: [
      {
        text: 'Guide',
        items: [
          { text: 'Quick Start', link: '/guide/quick-start' },
          { text: 'Request DSL', link: '/guide/request-dsl' },
          { text: 'Response DSL', link: '/guide/response-dsl' },
          { text: 'JSON Usage', link: '/guide/json-usage' }
        ]
      },
      {
        text: 'API Reference',
        items: [
          { text: 'Framework', link: '/api/framework' },
          { text: 'Response DSL', link: '/api/response-dsl' },
          { text: 'JSON', link: '/api/json' }
        ]
      },
      {
        text: 'Examples',
        link: '/examples/'
      },
      {
        text: 'Design',
        link: '/design/'
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
    },

    // 侧边栏配置
    sidebar: {
      '/guide/': [
        {
          text: 'Guide',
          items: [
            { text: 'Quick Start', link: '/guide/quick-start' },
            { text: 'Request DSL', link: '/guide/request-dsl' },
            { text: 'Response DSL', link: '/guide/response-dsl' },
            { text: 'JSON Usage', link: '/guide/json-usage' }
          ]
        }
      ],
      '/api/': [
        {
          text: 'API Reference',
          items: [
            { text: 'Framework', link: '/api/framework' },
            { text: 'Response DSL', link: '/api/response-dsl' },
            { text: 'JSON', link: '/api/json' }
          ]
        }
      ],
      '/examples/': [
        {
          text: 'Examples',
          items: [
            { text: 'Response DSL', link: '/examples/response-dsl' }
          ]
        }
      ],
      '/design/': [
        {
          text: 'Design',
          items: [
            { text: 'DSL Design', link: '/design/dsl-design' },
            { text: 'Architecture', link: '/design/architecture' }
          ]
        }
      ]
    }
  }
})
