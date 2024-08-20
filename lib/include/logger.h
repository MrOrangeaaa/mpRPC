#pragma once

#include "logqueue.hpp"
#include <string>

// 使用方法：LOG_INFO("xxx %d %s", 20, "xxxx")
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

// 日志级别
enum LogLevel
{
    INFO,   // 普通信息
    ERROR,  // 错误信息
};

// mprpc框架的异步日志系统
class Logger
{
public:
    // 单例模式
    static Logger& GetInstance();

    // 设置日志级别
    void SetLogLevel(LogLevel level);
    // 写日志
    void Log(std::string msg);

private:
    // 单例模式
    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;

private:
    int m_logLevel;  // 日志级别
    LogQueue<std::string> m_logQue;  // 日志缓冲队列
};