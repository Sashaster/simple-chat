#ifndef CHAT_LOGGING_H
#define CHAT_LOGGING_H

#include <iostream>
#include <chrono>
#include <mutex>

namespace logging {
    enum class LogLevel {
        ERROR,
        INFO,
        DEBUG
    };

    std::ostream& operator<<(std::ostream &out, LogLevel level);

    class Logger {
    private:
        LogLevel m_level;
        static inline std::mutex m_mutex;

    public:
        Logger() = delete;
        Logger& operator=(const Logger &) = default;
        Logger(const Logger &) = default;
        constexpr Logger(LogLevel level) : m_level(level) {}

        template<typename... Args>
        void Log(const LogLevel level, const std::string_view message, const Args &... args) const {
            if (m_level >= level) {
                m_mutex.lock();
                const auto timestamp = std::chrono::system_clock::now();
                auto &out = (level == LogLevel::ERROR) ? std::cerr : std::cout;
                out << timestamp << " [" << level << "] " << message << " ";
                ((out << args), ...);
                out << std::endl;
                m_mutex.unlock();
            }
        }

        template<typename... Args>
        void Debug(const std::string_view message, const Args &... args) const {
            Log(LogLevel::DEBUG, message, args...);
        };

        template<typename... Args>
        void Info(const std::string_view message, const Args &... args) const {
            Log(LogLevel::INFO, message, args...);
        };

        template<typename... Args>
        void Error(const std::string_view message, const Args &... args) const {
            Log(LogLevel::ERROR, message, args...);
        };
    };

    Logger& GetLogger();
    void SetDefaultLogger(const Logger &logger);

}


#endif
