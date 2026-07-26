#include <format>
#include <vector>
#include <csignal>

#include "core/config.h"
#include "core/logging.h"
#include "server/server.h"
#include "server/client.h"


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
        auto client = client::Client(cfg);
        std::signal(SIGINT, &client::Client::Disconnect);
        std::signal(SIGTERM, &client::Client::Disconnect);
        try {
            client.Connect();
        }catch (const std::exception& e) {
            logging::GetLogger().Error(e.what());
        }}

    return 0;
}
