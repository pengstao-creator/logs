#include "log.hpp"
/*
    日志系统接口说明
*/

void test1()
{
    // 创建一个局部日志器
    Log::Director d;               // 指挥者
    auto locala = d.LocalLogder(); // 默认局部异步日志器
    // locala->Debug(__LINE__, __FILE__, "这是一个默认局部异步日志器");

    auto locals = d.LocalLogder(
        "局部同步日志器",                  // 日志器名称
        Log::Data::LogGerType::SYNCLOGGER, // 日志器类型,同步/异步
        Log::LogLevel::DEBUG,              // 日志器限制输出水平,小于其则禁止输出
        // enum VALUE{
        //     UNKNOW = 0,
        //     DEBUG,
        //     INFO,
        //     WARNING,
        //     ERRNO,
        //     FATAL,
        //     OFF};
        "[%L][%N][{%Y-%m-%d %H:%M:%S}][%f][%l][%c]%n", // 日志输出格式控制
        // 负责输出格式化样式控制
        //%L 日志等级
        //%N 日志名称
        //%Y %m %d %H %M %S %D时间相关
        //%f 文件名
        //%l 行号
        //%t 线程id
        //%c 文件内容
        //%n 换行
        //%T tab
        //%o 其他

        Log::Data::AnsyCtrlType::COMMON // 提供两种线程控制方式COMMON / THPool
        // 可继承 Log::ACtrl::AnsyCtrl 扩展自己的线程控制方式
    );
    // locals->Errno(__LINE__, __FILE__, "这是一个自定义局部同步日志器");

    // GlobalTPool::getInstance().initialize(5); // 设置异步全局线程池线程个数,只能设置一次,不设置采用默认配置
    Log::Director a;
    a.AddSink<Log::SinkWay::StdoutSink>(); // 改变落地方式,向屏幕,默认文件循环
    // 同一个指挥者对象第一次调用AddSink为改变落地方式,后续则是添加落地方式,一个日志器可拥有多种落地方式,且每种都会执行
    // 可自己扩展落地方式,继承Log::Sink重写即可
    a.AddSink<Log::SinkWay::FiletSink>("./log");
    a.AddSink<Log::SinkWay::RollFileSink>(10000 /*单位字节,默认在配置文件中,运行产生*/
                                          ,
                                          "./a/Log" /*日志文件部分文件名,生成log1_20251218000238.txt*/);

    // a.AddAnsyWay<Log::ACtrl::AnsyCtrlCommon>(); // 添加线程控制方式

    auto local = a.LocalLogder();

    local->Warning(__LINE__, __FILE__, "这是一个warning信息,有三种落地方式");
}

void test2()
{
    // 创建一个全局日志器,基本创建操作和上述局部日志器一致
    Log::Director d;
    auto global = d.GlobalLogder();

    // 全局日志对象将会管理在Log::LogGer::SingleManage中,其中提供两个默认全局日志器
    // 同步与异步,同名日志对象不会被重复创建
    auto global1 = Log::LogGer::SingleManage::getInstance().getLoger(Log::Data::ASYN);  // 默认全局异步日志器
    auto globals1 = Log::LogGer::SingleManage::getInstance().getLoger(Log::Data::SYNC); // 默认全局同步日志器

    auto global2 = mylog::DefaultAsynLogger();
    auto globals2 = mylog::DefaultSyncLogger();
    // global,global1,global2为同一全局异步日志器,globals1,globals2,为同一全局同步日志器

    // 宏操作
    global1->DEBUG("这是一个为debug信息的全局异步日志器的函数调用宏操作");
    globals1->INFO("这是一个为info信息全局同步日志器的宏操作的函数调用宏操作");
    // 或者直接
    DEBUG_A("这是一个为debug信息的全局异步日志器的宏操作");
    INFO_S("这是一个为info信息全局同步日志器的宏操作");

    //     INFO,
    //     WARNING,
    //     ERRNO,
    //     FATAL,
    // 同理
}

int main()
{
    test1();
    test2();
    return 0;
}