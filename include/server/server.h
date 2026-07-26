#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <arpa/inet.h>
#include <optional>
#include <vector>
#include <memory>
#include <atomic>
#include <thread>

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
        ClientConnection& operator=(const ClientConnection &) = delete;
        ClientConnection(const ClientConnection &) = delete;
        ClientConnection(ClientConnection &&other) noexcept;
        ~ClientConnection();
        std::string ReceivePrefix() const;
        std::string SendPrefix() const;
    };

    class timeout_exception : public std::exception {
    public:
        timeout_exception() = default;
        const char *what() const noexcept override;
    };

    class Server {
    private:
        sockaddr_in m_conn;
        int m_socket;
        static std::atomic<bool> running;
        std::mutex m_mutex;
        std::vector<std::shared_ptr<ClientConnection>> m_connections;
        std::vector<std::thread> m_workers;
        void Handle(std::shared_ptr<ClientConnection> client);
        ClientConnection Accept(int timeout = 1) const;
        std::optional<std::string> Receive(const ClientConnection &client, int timeout = 1) const;
        void Send(const std::string &message, const ClientConnection &client, const ClientConnection &sender) const;
        void Broadcast(const std::string &message, const ClientConnection &sender);

    public:
        explicit Server(const Configuration &config);
        Server(const Server &server) = delete;
        Server& operator=(const Server &server) = delete;
        void Listen();
        static void Stop(int);
        ~Server();
    };

}

#endif
