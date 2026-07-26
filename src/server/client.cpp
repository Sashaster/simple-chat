#include <unistd.h>
#include <thread>

#include "server/client.h"
#include "server/server.h"

namespace client {
    using namespace server;

    Client::Client(const Configuration &config): m_socket(0) {
        m_server_conn.sin_family = AF_INET;
        m_server_conn.sin_port = htons(config.port);
        inet_pton(AF_INET, config.host.c_str(), &m_server_conn.sin_addr);
    }

    std::optional<std::string> Client::Receive(const int timeout) const {
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
        std::string message(4096, ' ');
        const int res = recv(m_socket, message.data(), message.size(), 0);
        if (res == 0) {
            return {};
        }
        if (res < 0) {
            throw std::runtime_error(strerror(errno));
        }
        message.resize(res);
        return message;
    }

    void Client::Connect() {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket < 0) {
            throw std::runtime_error(strerror(errno));
        }
        if (connect(m_socket, reinterpret_cast<sockaddr *>(&m_server_conn), sizeof(m_server_conn)) < 0) {
            throw std::runtime_error(strerror(errno));
        }
        connected = true;
        m_reader = std::thread([this]() {
            while (connected) {
                try {
                    if (const auto message = Receive()) {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        std::cout <<'\r'<< *message << std::endl;
                        std::cout << ">" << std::flush;
                    }
                }catch (const timeout_exception &e) {
                    continue;
                }
                catch (const std::exception &e) {
                    GetLogger().Error(e.what());
                }
            }
        });

        while (connected) {
            try {
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    std::cout << "\r>" << std::flush;
                }
                std::string msg;
                std::getline(std::cin, msg);
                if (!msg.empty()) {
                    Send(msg);
                }
            }catch (const std::exception &e) {
                GetLogger().Error(e.what());
            }
        }
    }

    void Client::Disconnect(int) {
        connected = false;
    }

    void Client::Send(const std::string_view message) const {
        if (send(m_socket, message.data(), message.length(), 0) < 0) {
            throw std::runtime_error(strerror(errno));
        }
        GetLogger().Debug(std::format("server<- {}", message));
    }

    Client::~Client() {
        GetLogger().Info("Disconnecting from server...");
        if (m_reader.joinable()) {
            m_reader.join();
        }
        if (!connected) {
            close(m_socket);
            GetLogger().Info("Disconnected from server");
        }
    }

    std::atomic<bool> Client::connected = false;

}