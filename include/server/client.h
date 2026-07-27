#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <arpa/inet.h>
#include <atomic>

#include "core/logging.h"
#include "core/config.h"

namespace client {
    using namespace logging;
    using namespace config;

    class Client {
    private:
        sockaddr_in m_server_conn;
        int m_socket;
        static std::atomic<bool> connected;
        std::optional<std::string> Receive(int timeout = 1) const;
        void Send(std::string_view message) const;
        std::mutex m_mutex;
        std::thread m_reader;

    public:
        Client(const Configuration &config);
        Client(const Client &) = delete;
        Client& operator=(const Client &) = delete;
        void Connect();
        static void Disconnect(int);
        ~Client();
    };

}

#endif //CHAT_CLIENT_H
