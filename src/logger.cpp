#include "logger.h"
#include "timestamp.h"

namespace mymuduo {
    namespace base {
        Logger& Logger::instance() {
            static Logger logger;
            return logger;
        }
        void Logger::setLoggerLevel(int logLevel) {
            logLevel_ = logLevel;
        }
        void Logger::log(int logLevel, const std::string& msg) {
            if (logLevel < logLevel_) {
                return;
            }
            switch (logLevel) {
                case LogLevel::INFO:
                    info(msg);
                    break;
                case LogLevel::DEBUG:
                    debug(msg);
                    break;
                case LogLevel::ERROR:
                    error(msg);
                    break;
                case LogLevel::FATAL:
                    fatal(msg);
                    break;
            }
        }
        void Logger::debug(const std::string& msg) {
            std::cout << "[DEBUG]";
            Timestamp ts;
            std::cout << ts.now().toString() << " : " << msg << std::endl;
        }
        void Logger::error(const std::string& msg) {
            std::cout << "[ERROR]";
            Timestamp ts;
            std::cout << ts.now().toString() << " : " << msg << std::endl;
        }
        void Logger::info(const std::string& msg) {
            std::cout << "[INFO]";
            Timestamp ts;
            std::cout << ts.now().toString() << " : " << msg << std::endl;
        }
        void Logger::fatal(const std::string& msg) {
            std::cout << "[FATAL]";
            Timestamp ts;
            std::cout << ts.now().toString() << " : " << msg << std::endl;
        }
    }
}