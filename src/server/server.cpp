#include <unistd.h>
#include <sys/socket.h>
#include <thread>

#include <server/server.h>
#include <core/logging.h>


namespace server{
    using namespace config;
    using namespace logging;

    ClientConnection::~ClientConnection() {
        if (socket >= 0) {
            close(socket);
            GetLogger().Info(std::format("{} disconnected", addr));
        }
    }

    std::string ClientConnection::ReceivePrefix() const {
        return addr + "<- ";
    }

    std::string ClientConnection::SendPrefix() const {
        return addr + "-> ";
    }

    ClientConnection::ClientConnection(ClientConnection &&other) noexcept: socket(other.socket), addr(std::move(other.addr)) {
        other.socket = -1;
    }

    const char* timeout_exception::what() const noexcept{
        return "Timeout";
    }

    std::atomic<bool> Server::running = false;

    Server::Server(const Configuration &config): m_socket(-1) {
        m_conn.sin_family = AF_INET;
        m_conn.sin_port = htons(config.port);
        inet_pton(AF_INET, config.host.c_str(), &m_conn.sin_addr);
    }

    void Server::Listen() {
        GetLogger().Info("Starting server...");
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket < 0)
            throw std::runtime_error(strerror(errno));
        if (bind(m_socket, reinterpret_cast<sockaddr *>(&m_conn), sizeof(m_conn)) < 0)
            throw std::runtime_error(strerror(errno));
        listen(m_socket, 5);
        running = true;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &m_conn.sin_addr, ip, INET_ADDRSTRLEN);
        int port = ntohs(m_conn.sin_port);
        GetLogger().Info(std::format("Server listening on {}:{}", ip, port));
        while (running) {
            try {
                auto client_conn = std::make_shared<ClientConnection>(Accept(1));
                {
                    m_connections.push_back(client_conn);
                }
                GetLogger().Info(std::format("Client connected: {}", client_conn->addr));
                m_workers.emplace_back(&Server::Handle, this, client_conn);
            }
            catch (const timeout_exception &e) {
                continue;
            }
            catch (const std::exception &e) {
                GetLogger().Error(e.what());
            }
        }
    }

    void Server::Broadcast(const std::string &message, const ClientConnection &sender){
        std::vector<std::shared_ptr<ClientConnection>> connections;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            connections = m_connections;
        }
        for (const auto &client : connections) {
            if (client->addr == sender.addr) {
                continue;
            }
            Send(message, *client, sender);
        }
    }

    void Server::Send(const std::string &message, const ClientConnection &client, const ClientConnection &sender) const {
        const auto msg = sender.SendPrefix() + message;
        if (send(client.socket, msg.data(), msg.length(), 0) < 0) {
            throw std::runtime_error(strerror(errno));
        }
        GetLogger().Debug(client.ReceivePrefix() + message);
    }

    void Server::Stop(int) {
        running = false;
    }

    void Server::Handle(const std::shared_ptr<ClientConnection> client){
        while (running) {
            try {
                const auto message = Receive(*client, 1);
                if (!message) {
                    break;
                }
                GetLogger().Debug(std::format("{}-> {}", client->addr, *message));
                Broadcast(*message, *client);

            }catch (const timeout_exception &e) {
                continue;
            }
            catch (const std::exception &e) {
                GetLogger().Error(e.what());
            }
        }
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::erase(m_connections, client);
        }
    }

    std::optional<std::string> Server::Receive(const ClientConnection &client, const int timeout) const{
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(client.socket, &rfds);

        timeval tm{.tv_sec = timeout, .tv_usec = 0};
        const int ready = select(client.socket + 1, &rfds, nullptr, nullptr, &tm);

        if (ready < 0) {
            if (errno == EINTR)
                throw timeout_exception();
            throw std::runtime_error(strerror(errno));
        }
        if (ready == 0) {
            throw timeout_exception();
        }
        std::string message(4096, ' ');
        const int res = recv(client.socket, message.data(), message.size(), 0);
        if (res == 0) {
            return {};
        }
        if (res < 0) {
            throw std::runtime_error(strerror(errno));
        }
        message.resize(res);

        return message;
    }

    ClientConnection Server::Accept(const int timeout) const{
        if (timeout < 0) {
            throw std::runtime_error("Timeout must be non-negative");
        }
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(m_socket, &rfds);

        timeval tm{.tv_sec = timeout, .tv_usec = 0};
        const int ready = select(m_socket + 1, &rfds, nullptr, nullptr, &tm);

        if (ready < 0) {
            if (errno == EINTR)
                throw timeout_exception();
            throw std::runtime_error(strerror(errno));
        }
        if (ready == 0) {
            throw timeout_exception();
        }
        sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);
        const int client_socket = accept(m_socket, reinterpret_cast<sockaddr*>(&client_address), &client_address_length);
        if (client_socket < 0)
            throw std::runtime_error(strerror(errno));
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_address.sin_addr, ip, INET_ADDRSTRLEN);
        const int port = ntohs(client_address.sin_port);
        return ClientConnection(client_socket, ip, port);
    }

    Server::~Server() {
        GetLogger().Info("Stopping server...");
        for (auto &worker: m_workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        if (!running) {
            close(m_socket);
            GetLogger().Info("Server stopped");
        }
    }

}