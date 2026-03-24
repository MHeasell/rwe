#pragma once

// Minimal logging replacement for spdlog.
// This project only used spdlog for basic file logging with timestamps,
// so a full logging framework was unnecessary overhead and was blocking
// the C++20 upgrade. If more advanced logging features are needed in the
// future (async logging, log rotation, multiple sinks, etc.), consider
// re-evaluating spdlog or another logging library.
//
// Usage:
//   LOG_INFO << "Loaded " << count << " units from " << path;
//   LOG_DEBUG << "Position: " << x << ", " << y;
//   LOG_ERROR << "Failed to open file: " << filename;
//
// Log level is controlled at compile time via RWE_LOG_LEVEL.
// Set it in CMake (e.g. -DRWE_LOG_LEVEL=1) to strip lower levels
// from the binary entirely. Default is 0 (everything enabled).
//
//   0 = Debug   (all messages)
//   1 = Info
//   2 = Warn
//   3 = Error
//   4 = Critical
//   5 = Off     (no messages)

#include <cassert>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>

// Compile-time log level. Statements below this level are dead code
// and will be eliminated by the compiler.
#ifndef RWE_LOG_LEVEL
#define RWE_LOG_LEVEL 0
#endif

#define RWE_LEVEL_DEBUG    0
#define RWE_LEVEL_INFO     1
#define RWE_LEVEL_WARN     2
#define RWE_LEVEL_ERROR    3
#define RWE_LEVEL_CRITICAL 4
#define RWE_LEVEL_OFF      5

namespace rwe
{
    enum class LogLevel
    {
        Debug,
        Info,
        Warn,
        Error,
        Critical
    };

    class SimpleLogger
    {
    private:
        std::ofstream file;
        std::mutex mutex;

        static const char* levelString(LogLevel lvl)
        {
            switch (lvl)
            {
                case LogLevel::Debug: return "debug";
                case LogLevel::Info: return "info";
                case LogLevel::Warn: return "warn";
                case LogLevel::Error: return "error";
                case LogLevel::Critical: return "critical";
            }
            return "unknown";
        }

        static std::string timestamp()
        {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;

            std::tm tm{};
#ifdef _WIN32
            localtime_s(&tm, &time);
#else
            localtime_r(&time, &tm);
#endif

            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
                << '.' << std::setfill('0') << std::setw(3) << ms.count();
            return oss.str();
        }

    public:
        SimpleLogger(const std::string& filePath, bool truncate)
            : file(filePath, truncate ? std::ios::trunc : std::ios::app)
        {
            if (!file.is_open())
            {
                throw std::runtime_error("Failed to open log file: " + filePath);
            }
        }

        void write(LogLevel msgLevel, const std::string& msg)
        {
            auto line = "[" + timestamp() + "] [" + levelString(msgLevel) + "] " + msg + "\n";
            std::lock_guard<std::mutex> lock(mutex);
            file << line;
            file.flush();
        }
    };

    // RAII stream that accumulates a log message via operator<<
    // and writes it to the logger when destroyed.
    class LogStream
    {
    private:
        SimpleLogger& logger;
        LogLevel level;
        std::ostringstream oss;

    public:
        LogStream(SimpleLogger& logger, LogLevel level)
            : logger(logger), level(level) {}

        ~LogStream()
        {
            logger.write(level, oss.str());
        }

        LogStream(const LogStream&) = delete;
        LogStream& operator=(const LogStream&) = delete;
        LogStream(LogStream&&) = default;

        template <typename T>
        LogStream& operator<<(const T& value)
        {
            oss << value;
            return *this;
        }
    };

    // Global logger instance, set once at startup.
    inline std::shared_ptr<SimpleLogger>& globalLogger()
    {
        static std::shared_ptr<SimpleLogger> instance;
        return instance;
    }

    inline void setGlobalLogger(std::shared_ptr<SimpleLogger> logger)
    {
        globalLogger() = std::move(logger);
    }

    inline SimpleLogger& getLogger()
    {
        auto& logger = globalLogger();
        assert(logger && "Logger not initialized");
        return *logger;
    }
}

// Logging macros. Enabled levels expand to a plain LogStream expression.
// Disabled levels use if(true){}else to create a dead branch that the
// compiler eliminates entirely while still type-checking the << chain.
#if RWE_LOG_LEVEL <= RWE_LEVEL_DEBUG
#define LOG_DEBUG rwe::LogStream(rwe::getLogger(), rwe::LogLevel::Debug)
#else
#define LOG_DEBUG if (true) {} else rwe::LogStream(rwe::getLogger(), rwe::LogLevel::Debug)
#endif

#if RWE_LOG_LEVEL <= RWE_LEVEL_INFO
#define LOG_INFO rwe::LogStream(rwe::getLogger(), rwe::LogLevel::Info)
#else
#define LOG_INFO if (true) {} else rwe::LogStream(rwe::getLogger(), rwe::LogLevel::Info)
#endif

#if RWE_LOG_LEVEL <= RWE_LEVEL_WARN
#define LOG_WARN rwe::LogStream(rwe::getLogger(), rwe::LogLevel::Warn)
#else
#define LOG_WARN if (true) {} else rwe::LogStream(rwe::getLogger(), rwe::LogLevel::Warn)
#endif

#if RWE_LOG_LEVEL <= RWE_LEVEL_ERROR
#define LOG_ERROR rwe::LogStream(rwe::getLogger(), rwe::LogLevel::Error)
#else
#define LOG_ERROR if (true) {} else rwe::LogStream(rwe::getLogger(), rwe::LogLevel::Error)
#endif

#if RWE_LOG_LEVEL <= RWE_LEVEL_CRITICAL
#define LOG_CRITICAL rwe::LogStream(rwe::getLogger(), rwe::LogLevel::Critical)
#else
#define LOG_CRITICAL if (true) {} else rwe::LogStream(rwe::getLogger(), rwe::LogLevel::Critical)
#endif
