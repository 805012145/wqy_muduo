#pragma once

#include <string>
#include <iostream>
#include "noncopyable.h"

#define LOG_INFO(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::instance(); \
        logger.setLoggerLevel(INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(LogLevel::INFO, buf); \
    } while(0)

#define LOG_DEBUG(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::instance(); \
        logger.setLoggerLevel(DEBUG); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(LogLevel::DEBUG, buf); \
    } while(0)

#define LOG_ERROR(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::instance(); \
        logger.setLoggerLevel(ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(LogLevel::ERROR, buf); \
    } while(0)

#define LOG_FATAL(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::instance(); \
        logger.setLoggerLevel(FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(LogLevel::FATAL, buf); \
        exit(-1); \
    } while(0)

namespace mymuduo {
    namespace base {
        // 定义日志级别 INFO DEBUG ERROR FATAL
        enum LogLevel {
            INFO,
            DEBUG,
            ERROR,
            FATAL
        };

        class Logger : noncopyable{
        public:
            // 获取唯一实例
            static Logger& instance();
            void log(int logLevel, const std::string& msg);
            void setLoggerLevel(int logLevel);
        private:
            int logLevel_;
            void debug(const std::string& msg);
            void error(const std::string& msg);
            void info(const std::string& msg);
            void fatal(const std::string& msg);
            Logger(){}
        };
    }
}