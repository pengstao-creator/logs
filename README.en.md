# logs - Modern C++ Logging System

## Description
A high-performance, flexible, and easy-to-use C++ logging system that supports both synchronous and asynchronous logging modes.

## Features

### Core Features
- **Dual Mode Support**: Both synchronous and asynchronous logging modes available
- **Multiple Log Levels**: DEBUG, INFO, WARNING, ERRNO, FATAL
- **Thread Safety**: Safe for multi-threaded applications
- **Customizable Formatting**: Support for custom log message formats
- **Multiple Sinks**: Log to stdout, files, or rolling files
- **Thread Pool Support**: Asynchronous logging with thread pool for high performance
- **Logger Management**: Global and local logger instances
- **Curly Brace Placeholder Support**: Use `{}` as universal placeholders that support any type of parameters and handle parameter count mismatches gracefully

### Advanced Features
- **Log Level Filtering**: Only log messages above a specified level
- **Rich Format Options**: Date, time, log level, filename, line number, thread ID, etc.
- **Builder Pattern**: Easy logger configuration with builder pattern
- **Singleton Manager**: Global logger management
- **Exception Safety**: Safe handling of invalid inputs and error conditions

## Project Structure

```
logs/
├── include/              # Header files
│   ├── ansyctrl.hpp     # Asynchronous control
│   ├── buffer.hpp       # Buffer management
│   ├── ConfigManager.hpp # Configuration management
│   ├── format.hpp       # Log formatting
│   ├── level.hpp        # Log levels
│   ├── logdata.hpp      # Log data structures
│   ├── logger.hpp       # Logger core
│   ├── log.hpp          # Main header (user interface)
│   ├── message.hpp      # Message handling
│   ├── ParseFormat.hpp  # Format parser 
│   ├── sink.hpp         # Log sinks
│   ├── threadpool.hpp   # Thread pool
│   └── tool.hpp         # Utility functions
├── tests/               # Test files
│   ├── test.cpp         # Basic tests
│   └── testlog.cpp      # Comprehensive tests
├── bin/                 # Build output directory
├── build/               # Build system files
├── config/              # Configuration files
├── logs/                # Default log output directory
├── LICENSE              # License file
├── README.md            # Chinese documentation
└── README.en.md         # English documentation
```

## Installation

### Prerequisites
- C++11 or higher
- CMake (optional, for building)
- Git

### Getting Started

1. Clone the repository
   ```bash
   git clone <repository-url>
   cd logs
   ```

2. Build the project
   ```bash
   mkdir -p build
   cd build
   cmake ..
   make
   ```

3. Install (optional)
   ```bash
   sudo make install
   ```

## Usage

### Basic Usage

```cpp
#include "log.hpp"

int main() {
    // Use default asynchronous logger with macro
    DEBUG_A("This is a debug message");
    INFO_A("This is an info message");
    WARNING_A("This is a warning message");
    ERRNO_A("This is an error message");
    FATAL_A("This is a fatal error message");
    
    // Use default synchronous logger with macro
    DEBUG_S("Synchronous debug message");
    INFO_S("Synchronous info message");
    
    return 0;
}
```

### Creating Custom Loggers

```cpp
#include "log.hpp"

int main() {
    Log::Director d;
    
    // Create a custom synchronous logger
    auto custom_logger = d.LocalLogder(
        "custom_logger",
        Log::Data::LogGerType::SYNCLOGGER,
        Log::LogLevel::DEBUG,
        "[%Y-%m-%d %H:%M:%S] [%L] %f:%l - %c%n",
        Log::Data::AnsyCtrlType::COMMON
    );
    
    custom_logger->Debug(__LINE__, __FILE__, "Custom logger debug message");
    custom_logger->Info(__LINE__, __FILE__, "Custom logger info message with number: {}", 42);
    
    return 0;
}
```

### Multiple Sinks

```cpp
#include "log.hpp"

int main() {
    Log::Director d;
    
    // Add multiple sinks
    d.AddSink<Log::SinkWay::StdoutSink>();
    d.AddSink<Log::SinkWay::FiletSink>("./app.log");
    d.AddSink<Log::SinkWay::RollFileSink>(1000000, "./roll.log"); // 1MB per file
    
    auto multi_sink_logger = d.LocalLogder("multi_sink");
    multi_sink_logger->Info(__LINE__, __FILE__, "This message will go to all sinks");
    
    return 0;
}
```

### Global Logger

```cpp
#include "log.hpp"

int main() {
    Log::Director d;
    
    // Create a global logger
    auto global_logger = d.GlobalLogder(
        "app_global",
        Log::Data::LogGerType::ASYNLOGGER,
        Log::LogLevel::INFO
    );
    
    // Use it anywhere in your application
    auto logger = Log::LogGer::SingleManage::getInstance().getLoger("app_global");
    if (logger) {
        logger->Info(__LINE__, __FILE__, "Global logger message");
    }
    
    return 0;
}
```

## Format Specification

### Supported Format Tokens

| Token | Description |
|-------|-------------|
| `%L`   | Log level (DEBUG, INFO, WARNING, ERRNO, FATAL) |
| `%N`   | Logger name |
| `%t`   | Thread ID |
| `%f`   | Filename |
| `%l`   | Line number |
| `%c`   | Log content |
| `%n`   | Newline |
| `%d`   | Current date |
| `%T`   | Current time |
| `{%Y-%m-%d %H:%M:%S}` | Custom date/time format (strftime style) |
| `{}`   | Universal type placeholder, automatically matches subsequent parameters (supports integers, strings, floats, etc.) |

### Example Formats

1. Simple format:
   ```
   "[%L] %c%n"
   ```

2. Detailed format with timestamp:
   ```
   "{%Y-%m-%d %H:%M:%S} [%L] %f:%l - %c%n"
   ```

3. Thread-aware format:
   ```
   "Thread %t | [%L] %c%n"
   ```

## Performance

The logging system is optimized for performance:

- **Synchronous Mode**: Low overhead for simple applications
- **Asynchronous Mode**: Minimal blocking with background processing
- **Thread Pool**: High throughput for high-load applications
- **Buffer Management**: Efficient memory usage

## Testing

The project includes comprehensive test cases covering:

- Basic functionality
- Log level filtering
- Multiple sinks
- Thread safety
- Performance
- Error conditions
- Format strings
- Logger management


To run the tests:

```bash
cd build
./tests/testlog

```

## Examples

### Example 1: Basic Logging

```cpp
#include "log.hpp"

int main() {
    // Use default asynchronous logger
    INFO_A("Application started");
    
    int result = 42;
    INFO_A("Computation result: {}", result);
    
    WARNING_A("Low memory warning");
    
    INFO_A("Application exiting");
    
    return 0;
}
```

### Example 2: Multi-threaded Application

```cpp
#include "log.hpp"
#include <thread>
#include <vector>

void worker(int id, Log::LogGer::Logger::ptr logger) {
    for (int i = 0; i < 10; ++i) {
        logger->Info(__LINE__, __FILE__, "Thread {}: Message {}", id, i);
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

## Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository
2. Create a new branch (`git checkout -b feature/AmazingFeature`)
3. Make your changes
4. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
5. Push to the branch (`git push origin feature/AmazingFeature`)
6. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Inspired by modern logging libraries like spdlog and glog
- Built with modern C++ best practices
- Designed for performance and usability
