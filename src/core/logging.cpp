#include "core/logging.h"


namespace logging {

    std::ostream& operator<< (std::ostream &out, const LogLevel level) {
        switch (level) {
            case LogLevel::INFO:
                return out << "INFO";
            case LogLevel::DEBUG:
                return out << "DEBUG";
            case LogLevel::ERROR:
                return out << "ERROR";
           default:
                return out << "INFO";
        }
    }

    static Logger default_logger = Logger(LogLevel::INFO);

    void SetDefaultLogger(const Logger &logger) {
        static auto logger_set = false;
        if (!logger_set) {
            default_logger = logger;
            logger_set = true;
        }
    }

    Logger& GetLogger() {
        return default_logger;
    }
}

