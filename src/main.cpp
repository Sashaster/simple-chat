#include <format>
#include <vector>
#include <limits>
#include <csignal>

#include "core/config.h"
#include "core/logging.h"
#include "server/server.h"
#include "server/client.h"


static void ShowOptions() {
    std::cout << "1. Disconnect\n2. Send message" << std::endl;
}

static int GetOption() {
    int option = 0;
    std::cin >> option;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw std::runtime_error("Invalid option");
    }
    return option;
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
    std::signal(SIGPIPE, SIG_IGN);
    const auto cfg = config::Configuration(args);
    const auto logger = logging::Logger(cfg.log_level);
    logging::SetDefaultLogger(logger);
    logger.Info("", cfg);
    if (app == "server") {
        std::signal(SIGINT, &server::Server::Stop);
        std::signal(SIGTERM, &server::Server::Stop);
        auto server = server::Server(cfg);
        server.Listen();
    }
    else {
        auto client = server::Client(cfg);
        client.Connect();
        while (client.IsConnected()) {
            try {
                ShowOptions();
                switch (int option = GetOption()) {
                    case 1:
                        client.Disconnect();break;
                    case 2: {
                        std::string msg;
                        std::cin >> msg;
                        client.Send(msg); break;
                    }
                    default:
                        throw std::runtime_error("Invalid option");
                }
            }
            catch (const std::exception &e) {
                logging::GetLogger().Error(e.what());
            }
        }
    }
    return 0;
}
