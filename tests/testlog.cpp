#include "../include/log.hpp"
#include <thread>
#include <vector>
#include <chrono>
#include <cassert>

// 测试1：基本功能测试
void test_basic_functionality() {
    std::cout << "=== 测试1：基本功能测试 ===" << std::endl;
    
    Log::Director d;
    
    // 测试默认局部异步日志器
    auto default_logger = d.LocalLogder();
    default_logger->Debug(__LINE__, __FILE__, "这是一条调试信息");
    default_logger->Info(__LINE__, __FILE__, "这是一条信息");
    default_logger->Warning(__LINE__, __FILE__, "这是一条警告");
    default_logger->Errno(__LINE__, __FILE__, "这是一条错误");
    default_logger->Fatal(__LINE__, __FILE__, "这是一条致命错误");
    
    // 测试自定义格式
    auto custom_logger = d.LocalLogder(
        "自定义格式日志器",
        Log::Data::LogGerType::SYNCLOGGER,
        Log::LogLevel::DEBUG,
        "[等级:%L][名称:%N][时间:{%Y-%m-%d %H:%M:%S}][文件:%f:%l]%n内容:%c%n",
        Log::Data::AnsyCtrlType::COMMON
    );
    
    custom_logger->Info(__LINE__, __FILE__, "自定义格式测试");
}

// 测试2：日志级别过滤
void test_log_level_filter() {
    std::cout << "\n=== 测试2：日志级别过滤测试 ===" << std::endl;
    
    Log::Director d;
    
    // 创建只记录WARNING及以上级别的日志器
    auto warning_logger = d.LocalLogder(
        "警告级别日志器",
        Log::Data::LogGerType::SYNCLOGGER,
        Log::LogLevel::WARNING,
        "[%L] %c%n",
        Log::Data::AnsyCtrlType::COMMON
    );
    
    warning_logger->Debug(__LINE__, __FILE__, "这条调试信息不应该出现");
    warning_logger->Info(__LINE__, __FILE__, "这条信息不应该出现");
    warning_logger->Warning(__LINE__, __FILE__, "这条警告应该出现");
    warning_logger->Errno(__LINE__, __FILE__, "这条错误应该出现");
}

// 测试3：多种落地方式
void test_multiple_sinks() {
    std::cout << "\n=== 测试3：多种落地方式测试 ===" << std::endl;
    
    Log::Director d;
    
    // 添加多种落地方式
    d.AddSink<Log::SinkWay::StdoutSink>();
    d.AddSink<Log::SinkWay::FiletSink>("./test_logs/basic");
    d.AddSink<Log::SinkWay::RollFileSink>(5000, "./test_logs/roll/roll_log");
    
    auto multi_sink_logger = d.LocalLogder();
    
    // 生成足够多的日志以触发滚动
    for (int i = 0; i < 100; ++i) {
        multi_sink_logger->Info(__LINE__, __FILE__, 
            "测试多种落地方式的日志条目 " + std::to_string(i) + 
            " 这是一条比较长的日志消息，用于测试文件滚动功能");
    }
}

// 测试4：全局日志器
void test_global_loggers() {
    std::cout << "\n=== 测试4：全局日志器测试 ===" << std::endl;
    
    // 获取默认全局日志器
    auto async_global = mylog::DefaultAsynLogger();
    auto sync_global = mylog::DefaultSyncLogger();
    
    // 测试宏操作
    async_global->DEBUG("使用函数宏的异步日志");
    sync_global->INFO("使用函数宏的同步日志");
    
    // 测试直接宏
    DEBUG_A("直接宏的异步DEBUG日志");
    INFO_S("直接宏的同步INFO日志");
    WARNING_A("直接宏的异步WARNING日志");
    ERRNO_S("直接宏的同步ERROR日志");
    FATAL_A("直接宏的异步FATAL日志");
}

// 测试5：线程安全测试
void test_thread_safety() {
    std::cout << "\n=== 测试5：线程安全测试 ===" << std::endl;
    
    Log::Director d;
    d.AddSink<Log::SinkWay::StdoutSink>();
    auto thread_logger = d.LocalLogder("线程测试日志器");
    
    const int thread_count = 10;
    const int logs_per_thread = 20;
    
    std::vector<std::thread> threads;
    
    auto worker = [&](int thread_id) {
        for (int i = 0; i < logs_per_thread; ++i) {
            thread_logger->Info(__LINE__, __FILE__, 
                "线程" + std::to_string(thread_id) + 
                " - 日志" + std::to_string(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };
    
    // 创建并启动线程
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(worker, i);
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "所有线程日志完成" << std::endl;
}

// 测试6：性能测试
void test_performance() {
    std::cout << "\n=== 测试6：性能测试 ===" << std::endl;
    
    const int log_count = 10000;
    
    // 测试同步日志器性能
    {
        Log::Director d;
        auto sync_logger = d.LocalLogder(
            "性能测试同步",
            Log::Data::LogGerType::SYNCLOGGER,
            Log::LogLevel::INFO,
            "[%N] %c%n",
            Log::Data::AnsyCtrlType::COMMON
        );
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < log_count; ++i) {
            sync_logger->Info(__LINE__, __FILE__, "性能测试消息 " + std::to_string(i));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "同步日志器写入 " << log_count << " 条日志耗时: " 
                  << duration.count() << "ms" << std::endl;
    }
    
    // 测试异步日志器性能
    {
        Log::Director d;
        d.AddSink<Log::SinkWay::FiletSink>("./test_logs/performance");
        auto async_logger = d.LocalLogder(
            "性能测试异步",
            Log::Data::LogGerType::ASYNLOGGER,
            Log::LogLevel::INFO,
            "[%N] %c%n",
            Log::Data::AnsyCtrlType::COMMON
        );
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < log_count; ++i) {
            async_logger->Info(__LINE__, __FILE__, "性能测试消息 " + std::to_string(i));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "异步日志器写入 " << log_count << " 条日志耗时: " 
                  << duration.count() << "ms" << std::endl;
        
        // 等待异步日志器完成所有写入
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

// 测试7：异常情况测试
void test_error_cases() {
    std::cout << "\n=== 测试7：异常情况测试 ===" << std::endl;
    
    // 测试无效路径
    try {
        Log::Director d;
        d.AddSink<Log::SinkWay::FiletSink>("/invalid/path/log.txt");
        auto logger = d.LocalLogder();
        logger->Info(__LINE__, __FILE__, "这条日志应该会失败或使用默认路径");
    } catch (const std::exception& e) {
        std::cout << "捕获到异常: " << e.what() << std::endl;
    }
    
    // 测试空消息
    Log::Director d2;
    d2.AddSink<Log::SinkWay::StdoutSink>();
    auto logger2 = d2.LocalLogder();
    logger2->Info(__LINE__, __FILE__, "");  // 空消息
}

// 测试8：自定义线程池
void test_thread_pool() {
    std::cout << "\n=== 测试8：线程池测试 ===" << std::endl;
    
    // 初始化全局线程池
    GlobalTPool::getInstance().initialize(3);
    
    Log::Director d;
    d.AddSink<Log::SinkWay::StdoutSink>();
    
    // 使用线程池的异步日志器
    auto tp_logger = d.LocalLogder(
        "线程池日志器",
        Log::Data::LogGerType::ASYNLOGGER,
        Log::LogLevel::DEBUG,
        "[%L][%t] %c%n",
        Log::Data::AnsyCtrlType::THPOOL
    );
    
    for (int i = 0; i < 10; ++i) {
        tp_logger->Info(__LINE__, __FILE__, "线程池测试消息 " + std::to_string(i));
    }
    
    // 等待日志处理完成
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// 测试9：格式字符串测试
void test_format_strings() {
    std::cout << "\n=== 测试9：格式字符串测试 ===" << std::endl;
    
    Log::Director d;
    d.AddSink<Log::SinkWay::StdoutSink>();
    
    // 测试不同的格式字符串
    std::vector<std::string> formats = {
        "[%L][%N] %c%n",
        "{%Y-%m-%d %H:%M:%S} [%f:%l] %c%n",
        "线程:%t 文件:%f 行:%l%n内容:%c%n",
        "%c - 来自 %f 的第 %l 行%n"
    };
    
    for (size_t i = 0; i < formats.size(); ++i) {
        auto logger = d.LocalLogder(
            "格式测试" + std::to_string(i),
            Log::Data::LogGerType::SYNCLOGGER,
            Log::LogLevel::INFO,
            formats[i],
            Log::Data::AnsyCtrlType::COMMON
        );
        
        logger->Info(__LINE__, __FILE__, 
            "这是格式测试 " + std::to_string(i) + " 的日志内容");
    }
}

// 测试10：日志器管理测试
void test_logger_management() {
    std::cout << "\n=== 测试10：日志器管理测试 ===" << std::endl;
    
    auto& manager = Log::LogGer::SingleManage::getInstance();
    
    // 获取不存在的日志器
    auto logger1 = manager.getLoger("不存在的日志器");
    assert(logger1 == nullptr && "获取不存在的日志器应该返回nullptr");
    
    // 创建并获取日志器
    Log::Director d;
    d.AddSink<Log::SinkWay::StdoutSink>();
    
    auto local_logger = d.GlobalLogder(
        "测试管理日志器",
        Log::Data::LogGerType::SYNCLOGGER,
        Log::LogLevel::DEBUG,
        "[%N] %c%n",
        Log::Data::AnsyCtrlType::COMMON
    );
    
    // 通过管理器获取相同的日志器
    auto same_logger = manager.getLoger("测试管理日志器");
    assert(same_logger == local_logger && "应该获取到相同的日志器对象");
    
    // 测试默认日志器
    auto default_async = manager.getLoger(Log::Data::ASYN);
    auto default_sync = manager.getLoger(Log::Data::SYNC);
    
    assert(default_async != nullptr && "默认异步日志器不应为空");
    assert(default_sync != nullptr && "默认同步日志器不应为空");
    
    default_async->Info(__LINE__, __FILE__, "通过管理器获取的默认异步日志器");
    default_sync->Info(__LINE__, __FILE__, "通过管理器获取的默认同步日志器");
}

// 主测试函数
int main() {
    std::cout << "开始日志系统测试..." << std::endl;
    
    // 创建测试目录
    system("mkdir -p ./test_logs");
    system("mkdir -p ./test_logs/roll");
    
    try {
        test_basic_functionality();
        test_log_level_filter();
        test_multiple_sinks();
        test_global_loggers();
        test_thread_safety();
        test_performance();
        test_error_cases();
        test_thread_pool();
        test_format_strings();
        test_logger_management();
        
        std::cout << "\n=== 所有测试完成 ===" << std::endl;
        std::cout << "请检查以下目录的日志文件：" << std::endl;
        std::cout << "1. ./test_logs/" << std::endl;
        std::cout << "2. ./test_logs/roll/" << std::endl;
        std::cout << "3. ./log/ (默认目录)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "测试失败，异常信息: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}