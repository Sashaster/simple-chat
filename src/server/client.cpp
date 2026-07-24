#include <unistd.h>
#include <server/client.h>


namespace server {

    Client::Client(const Configuration &config): m_socket(0), m_connected(false) {
        m_server_conn.sin_family = AF_INET;
        m_server_conn.sin_port = htons(config.port);
        inet_pton(AF_INET, config.host.c_str(), &m_server_conn.sin_addr);
    }

    void Client::Connect() {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket < 0) {
            throw std::runtime_error(strerror(errno));
        }
        if (connect(m_socket, reinterpret_cast<sockaddr *>(&m_server_conn), sizeof(m_server_conn)) < 0) {
            throw std::runtime_error(strerror(errno));
        }
        m_connected = true;
    }

    bool Client::IsConnected() const {
        return m_connected;
    }

    void Client::Disconnect() {
        if (m_connected) {
            m_connected = false;
            close(m_socket);
            GetLogger().Info("Disconnecting from server...");
        }
    }

    void Client::Send(const std::string_view message) const {
        if (send(m_socket, message.data(), message.length(), 0) < 0) {
            throw std::runtime_error(strerror(errno));
        };
    }

    Client::~Client() {
        if (m_connected) {
            close(m_socket);
            GetLogger().Debug("Client socket closed");
        }
    }

}