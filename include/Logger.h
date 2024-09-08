#pragma once
#include <string>
#include <iostream>

#include "Timestamp.h"
#include "noncopyable.h"
// 定义日志级别  INFO ERROR FATAL（崩溃） DEBUG

// LOG_INFO("%S %D,arg1, arg2")
// 防止宏较大产生错误，使用如下方式:保持宏为可读的语法块
#define LOG_INFO(logmsgFormat, ...)                   \
  do                                                  \
  {                                                   \
    Logger &logger = Logger::instance();              \
    logger.setLogLevel(INFO);                         \
    char buf[1024] = {0};                             \
    snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
    logger.log(buf);                                  \
  } while (0)

#define LOG_ERROR(logmsgFormat, ...)                   \
  do                                                  \
  {                                                   \
    Logger &logger = Logger::instance();              \
    logger.setLogLevel(ERROR);                         \
    char buf[1024] = {0};                             \
    snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
    logger.log(buf);                                  \
  } while (0)

#define LOG_FATAL(logmsgFormat, ...)                   \
  do                                                  \
  {                                                   \
    Logger &logger = Logger::instance();              \
    logger.setLogLevel(FATAL);                         \
    char buf[1024] = {0};                             \
    snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
    logger.log(buf);                                  \
    exit(-1);                                          \
  } while (0)

#ifdef MUDEBUG
#define LOG_DEBUG(logmsgFormat, ...)                   \
  do                                                  \
  {                                                   \
    Logger &logger = Logger::instance();              \
    logger.setLogLevel(DEBUG);                         \
    char buf[1024] = {0};                             \
    snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
    logger.log(buf);                                  \
  } while (0)
#else
  #define LOG_DEBUG(logmsgFormat, ...)
#endif
enum LogLevel
{
  INFO,  // 普通信息
  ERROR, // 错误信息
  FATAL, // core 信息
  DEBUG, // 调试信息
};

// 输出一个日志类
class Logger : noncopyable // 直接就是私有继承
{
public:
  // 获取日志的实例对象
  static Logger &instance();
  // 设置日志级别
  void setLogLevel(int level);
  // 写日志
  void log(std::string msg);

private:
  int logLevel_;
  Logger() {}
};