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

    ClientConnection::ClientConnection(ClientConnection &&other) noexcept: socket(other.socket), addr(std::move(other.addr)) {
        other.socket = -1;
    }

    const char* TimeoutException::what() const noexcept{
        return msg.c_str();
    }

    volatile bool Server::running = false;

    Server::Server(const Configuration &config): m_socket(0) {
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
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(m_socket, &rfds);

            timeval timeout{.tv_sec = 1, .tv_usec = 0};
            int ready = select(m_socket + 1, &rfds, nullptr, nullptr, &timeout);

            if (ready < 0) {
                if (errno == EINTR) continue;
                GetLogger().Error(strerror(errno));
                continue;
            }
            if (ready == 0) {
                continue;
            }
            try {
                ClientConnection client_conn = Accept();
                GetLogger().Info(std::format("Client connected: {}", client_conn.addr));
                std::thread(&Server::Handle, this, std::move(client_conn)).detach();
            }
            catch (const std::exception &e) {
                GetLogger().Error(e.what());
            }
        }
    }

    void Server::Stop(int) {
        running = false;
    }

    void Server::Handle(ClientConnection client) {
        while (running) {
            try {
                const auto message = Receive(client);
                if (!message)
                    break;
                GetLogger().Debug(std::format("{}-> {}", client.addr, *message));

            }catch (const TimeoutException &e) {
                GetLogger().Debug(e.what());
            }
            catch (const std::exception &e) {
                GetLogger().Error(e.what());
            }
        }
    }

    std::optional<std::string> Server::Receive(const ClientConnection &client){
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(client.socket, &rfds);

        timeval timeout{.tv_sec = 1, .tv_usec = 0};
        int ready = select(client.socket + 1, &rfds, nullptr, nullptr, &timeout);

        if (ready < 0) {
            if (errno == EINTR) throw TimeoutException("Timeout");
            throw std::runtime_error(strerror(errno));
        }
        if (ready == 0) {
            throw TimeoutException("Timeout");
        }
        std::string message(4096, ' ');
        int res = recv(client.socket, message.data(), message.size(), 0);
        if (res <= 0) {
            return {};
        }
        message.resize(res);
        return std::make_optional(message);
    }

    ClientConnection Server::Accept() const{
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
        close(m_socket);
        GetLogger().Info("Stopping server...");
    }
}
