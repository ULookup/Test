#pragma once

#include <string>
#include <memory>
#include <sstream>
#include "LogLevel.h"
#include "LogSink.h"
#include "ConsoleSink.h"
#include "FileSink.h"
#include "TimeUtil.h"

namespace icelog
{
    class Logger
    {
    public:
        Logger();
        ~Logger();
        void _EnableConsoleSink();
        void _EnableFileSink();

        class LogMessage
        {
        public:
            LogMessage(LogLevel level, std::string &filename, int line, Logger &logger);

            template <typename T>
            LogMessage &operator<<(const T &info)
            {
                std::stringstream ss;
                ss << info;
                _loginfo += ss.str();
                return *this;
            }

            ~LogMessage();

        private:
            std::string _curr_time;
            LogLevel _level;
            pid_t _pid;
            std::string _filename;
            int _line;

            std::string _loginfo;
            Logger &_logger;
        };

        LogMessage operator()(LogLevel level, std::string filename, int line);

    private:
        std::unique_ptr<LogSink> _sink;
    };

    extern Logger logger;

#define LOG(level) icelog::logger(level, __FILE__, __LINE__)

#define LOG_INFO LOG(icelog::LogLevel::INFO)
#define LOG_DEBUG LOG(icelog::LogLevel::DEBUG)
#define LOG_WARNING LOG(icelog::LogLevel::WARNING)
#define LOG_ERROR LOG(icelog::LogLevel::ERROR)
#define LOG_FATAL LOG(icelog::LogLevel::FATAL)

#define EnableConsoleSink() icelog::logger._EnableConsoleSink()
#define EnableFileSink() icelog::logger._EnableFileSink()
}