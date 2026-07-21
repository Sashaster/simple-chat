#include <format>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "core/config.h"
#include "core/logging.h"

static int alloc_count = 0;

void* operator new(std::size_t size){
    void *ptr = std::malloc(size);
    if (ptr == nullptr)
        throw std::bad_alloc();
    alloc_count++;
    return ptr;
}

void operator delete(void *ptr) noexcept{
    std::free(ptr);
}

int main(int argc, char* argv[]) {
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    if (args.empty()) {
        std::exit(EXIT_FAILURE);
    }
    const auto app = args[0];
    if (app != "client" && app != "server") {
        std::exit(EXIT_FAILURE);
    }
    const auto cfg = config::Configuration(args);
    const auto logger = logging::Logger(cfg.log_level);
    logger.Info("", cfg);
    if (app == "server") {
        int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            logger.Error(strerror(errno));
            std::exit(EXIT_FAILURE);
        }
        sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(cfg.port);
        inet_pton(AF_INET, cfg.host.c_str(), &server_address.sin_addr);

        auto bind_result = bind(server_socket, reinterpret_cast<sockaddr *>(&server_address), sizeof(server_address));
        if (bind_result < 0) {
            logger.Error(strerror(errno));
            std::exit(EXIT_FAILURE);
        }
        listen(server_socket, 5);
        int client_socket = accept(server_socket, nullptr, nullptr);
        char buffer[1024] = {};
        recv(client_socket, buffer, sizeof(buffer), 0);
        logger.Info(buffer);
        close(server_socket);
    }
    else {
        int client_socket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(cfg.port);
        inet_pton(AF_INET, cfg.host.c_str(), &server_address.sin_addr);
        int connect_result = connect(client_socket, reinterpret_cast<sockaddr *>(&server_address), sizeof(server_address));
        if (connect_result < 0) {
            logger.Error(strerror(errno));
            std::exit(EXIT_FAILURE);
        }
        const char *message = "Hello, Sashaster";
        send(client_socket, message, strlen(message), 0);
        close(client_socket);
    }
    logger.Debug(std::format("Allocations count: {}", alloc_count));
    return 0;
}
