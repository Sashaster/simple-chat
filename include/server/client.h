

#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <arpa/inet.h>

#include "core/logging.h"
#include "core/config.h"

namespace server {
    using namespace logging;
    using namespace config;


    class Client {
    private:
        sockaddr_in m_server_conn;
        int m_socket;
        bool m_connected;

    public:
        Client() = delete;
        Client(const Configuration &config);
        Client(const Client &) = default;
        Client& operator=(const Client &) = delete;
        void Connect();
        void Send(std::string_view message) const;
        bool IsConnected() const;
        void Disconnect();
        ~Client();
    };

}

#endif //CHAT_CLIENT_H
