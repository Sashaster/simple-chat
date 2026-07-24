#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <arpa/inet.h>
#include <optional>

#include "core/config.h"
#include "core/logging.h"


namespace server {
    using namespace config;
    using namespace logging;

    struct ClientConnection {
        int socket;
        std::string addr;

        ClientConnection(const int socket, const std::string &ip, const int port): socket{socket}, addr(ip + ":" + std::to_string(port)) {}
        ClientConnection() = delete;
        ClientConnection& operator=(const ClientConnection &) = default;
        ClientConnection(const ClientConnection &) = delete;
        ClientConnection(ClientConnection &&other) noexcept;
        ~ClientConnection();
    };

    class TimeoutException : public std::exception {
    private:
        std::string msg;
    public:
        explicit TimeoutException(const std::string_view msg) : msg(msg){}
        const char *what() const noexcept override;
        TimeoutException() = delete;
        TimeoutException(const TimeoutException &) = default;
        TimeoutException &operator=(const TimeoutException &) = delete;
    };

    class Server {
    private:
        sockaddr_in m_conn;
        int m_socket;
        static volatile bool running;
        void Handle(ClientConnection client);
        ClientConnection Accept() const;
        std::optional<std::string> Receive(const ClientConnection &client);

    public:
        Server(const Configuration &config);
        Server(const Server &server) = delete;
        Server& operator=(const Server &server) = delete;
        Server() = delete;
        void Listen();
        static void Stop(int);
        ~Server();
    };

}

#endif
