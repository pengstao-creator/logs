# logs - 现代C++日志系统

## 项目介绍
一个高性能、灵活且易于使用的C++日志系统，支持同步和异步两种日志模式。

## 功能特性

### 核心功能
- **双模式支持**：同时支持同步和异步日志模式
- **多种日志级别**：DEBUG、INFO、WARNING、ERRNO、FATAL
- **线程安全**：适用于多线程应用程序
- **可自定义格式化**：支持自定义日志消息格式
- **多种输出目标**：日志可输出到标准输出、文件或滚动文件
- **线程池支持**：异步日志使用线程池提高性能
- **日志器管理**：支持全局和局部日志器实例
- **大括号占位符支持**：使用 `{}` 作为万能占位符，支持任意类型参数和参数数量不匹配的情况

### 高级特性
- **日志级别过滤**：仅记录指定级别及以上的日志消息
- **丰富的格式选项**：日期、时间、日志级别、文件名、行号、线程ID等
- **建造者模式**：使用建造者模式简化日志器配置
- **单例管理器**：全局日志器管理
- **异常安全**：安全处理无效输入和错误条件

## 项目结构

```
logs/
├── include/              # 头文件目录
│   ├── ansyctrl.hpp     # 异步控制
│   ├── buffer.hpp       # 缓冲区管理
│   ├── ConfigManager.hpp # 配置管理
│   ├── format.hpp       # 日志格式化
│   ├── level.hpp        # 日志级别
│   ├── logdata.hpp      # 日志数据结构
│   ├── logger.hpp       # 日志器核心
│   ├── log.hpp          # 主头文件（用户接口）
│   ├── message.hpp      # 消息处理
│   ├── ParseFormat.hpp  # 格式解析器
│   ├── sink.hpp         # 日志输出目标
│   ├── threadpool.hpp   # 线程池
│   └── tool.hpp         # 工具函数
├── tests/               # 测试文件目录
│   ├── test.cpp         # 基本测试
│   └── testlog.cpp      # 综合测试
├── bin/                 # 构建输出目录
├── build/               # 构建系统文件
├── config/              # 配置文件目录
├── logs/                # 默认日志输出目录
├── LICENSE              # 许可证文件
├── README.md            # 中文文档
└── README.en.md         # 英文文档
```

## 安装

### 前提条件
- C++11或更高版本
- CMake（可选，用于构建）
- Git

### 快速开始

1. 克隆仓库
   ```bash
   git clone <仓库地址>
   cd logs
   ```

2. 构建项目
   ```bash
   mkdir -p build
   cd build
   cmake ..
   make
   ```

3. 安装（可选）
   ```bash
   sudo make install
   ```

## 使用方法

### 基本用法

```cpp
#include "log.hpp"

int main() {
    // 使用默认异步日志器（宏方式）
    DEBUG_A("这是一条调试信息");
    INFO_A("这是一条普通信息");
    WARNING_A("这是一条警告信息");
    ERRNO_A("这是一条错误信息");
    FATAL_A("这是一条致命错误信息");
    
    // 使用默认同步日志器（宏方式）
    DEBUG_S("同步调试信息");
    INFO_S("同步普通信息");
    
    return 0;
}
```

### 创建自定义日志器

```cpp
#include "log.hpp"

int main() {
    Log::Director d;
    
    // 创建自定义同步日志器
    auto custom_logger = d.LocalLogder(
        "custom_logger",
        Log::Data::LogGerType::SYNCLOGGER,
        Log::LogLevel::DEBUG,
        "[%Y-%m-%d %H:%M:%S] [%L] %f:%l - %c%n",
        Log::Data::AnsyCtrlType::COMMON
    );
    
    custom_logger->Debug(__LINE__, __FILE__, "自定义日志器调试信息");
    custom_logger->Info(__LINE__, __FILE__, "自定义日志器信息，包含数字: {}", 42);
    
    return 0;
}
```

### 多种输出目标

```cpp
#include "log.hpp"

int main() {
    Log::Director d;
    
    // 添加多种输出目标
    d.AddSink<Log::SinkWay::StdoutSink>();
    d.AddSink<Log::SinkWay::FiletSink>("./app.log");
    d.AddSink<Log::SinkWay::RollFileSink>(1000000, "./roll.log"); // 每个文件1MB
    
    auto multi_sink_logger = d.LocalLogder("multi_sink");
    multi_sink_logger->Info(__LINE__, __FILE__, "这条消息将输出到所有目标");
    
    return 0;
}
```

### 全局日志器

```cpp
#include "log.hpp"

int main() {
    Log::Director d;
    
    // 创建全局日志器
    auto global_logger = d.GlobalLogder(
        "app_global",
        Log::Data::LogGerType::ASYNLOGGER,
        Log::LogLevel::INFO
    );
    
    // 在应用程序的任何地方使用它
    auto logger = Log::LogGer::SingleManage::getInstance().getLoger("app_global");
    if (logger) {
        logger->Info(__LINE__, __FILE__, "全局日志器消息");
    }
    
    return 0;
}
```

## 格式规范

### 支持的格式标记

| 标记 | 描述 |
|------|------|
| `%L`   | 日志级别（DEBUG、INFO、WARNING、ERRNO、FATAL） |
| `%N`   | 日志器名称 |
| `%t`   | 线程ID |
| `%f`   | 文件名 |
| `%l`   | 行号 |
| `%c`   | 日志内容 |
| `%n`   | 换行符 |
| `%d`   | 当前日期 |
| `%T`   | 当前时间 |
| `{%Y-%m-%d %H:%M:%S}` | 自定义日期/时间格式（strftime风格） |
| `{}`   | 万能类型占位符，自动匹配后续参数（支持整数、字符串、浮点数等） |

### 格式示例

1. 简单格式：
   ```
   "[%L] %c%n"
   ```

2. 带时间戳的详细格式：
   ```
   "{%Y-%m-%d %H:%M:%S} [%L] %f:%l - %c%n"
   ```

3. 线程感知格式：
   ```
   "线程%t | [%L] %c%n"
   ```

## 性能

日志系统针对性能进行了优化：

- **同步模式**：简单应用程序的开销较低
- **异步模式**：最小化阻塞，后台处理日志
- **线程池**：高负载应用的高吞吐量
- **缓冲区管理**：高效的内存使用

## 测试

项目包含全面的测试用例，涵盖：

- 基本功能
- 日志级别过滤
- 多种输出目标
- 线程安全
- 性能测试
- 错误条件处理
- 格式字符串
- 日志器管理


运行测试：

```bash
cd build
./tests/testlog

```

## 示例代码

### 示例1：基本日志记录

```cpp
#include "log.hpp"

int main() {
    // 使用默认异步日志器
    INFO_A("应用程序启动");
    
    int result = 42;
    INFO_A("计算结果: {}", result);
    
    WARNING_A("内存不足警告");
    
    INFO_A("应用程序退出");
    
    return 0;
}
```

### 示例2：多线程应用

```cpp
#include "log.hpp"
#include <thread>
#include <vector>

void worker(int id, Log::LogGer::Logger::ptr logger) {
    for (int i = 0; i < 10; ++i) {
        logger->Info(__LINE__, __FILE__, "线程{}: 消息{}", id, i);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    Log::Director d;
    d.AddSink<Log::SinkWay::StdoutSink>();
    auto logger = d.LocalLogder("thread_safe");
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(worker, i, logger);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

## 贡献指南

欢迎贡献代码！请按照以下步骤：

1. Fork本仓库
2. 创建新分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开Pull Request

## 许可证

本项目采用MIT许可证 - 查看LICENSE文件了解详情。

## 致谢

- 受spdlog和glog等现代日志库启发
- 采用现代C++最佳实践构建
- 设计注重性能和易用性
