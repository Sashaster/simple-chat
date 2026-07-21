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
}

