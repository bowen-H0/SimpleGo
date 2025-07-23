# SimpleGo

## 项目简介

SimpleGo 是一个基于 C++ 和 SFML 的简易围棋游戏示例。程序启动后会显示 19x19 的棋盘，允许玩家下黑子并与内置的简单 AI 对战。对局过程中会统计双方的吃子数，并在落子时播放提示音。

## 编译环境

- C++17 编译器
- [SFML](https://www.sfml-dev.org/) 2.5 或更高版本（需包含 `graphics`、`window`、`system`、`audio` 模块）

## 编译与运行

在项目根目录下执行以下命令即可生成可执行文件并启动游戏：

```bash
g++ main.cpp -std=c++17 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -o SimpleGo
./SimpleGo
```

## 版权与许可

本项目采用 MIT 许可证，摘录如下：

```
MIT License

Copyright (c) 2025 bowen-H0

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
```

完整内容请见 [LICENSE](LICENSE)。
