#include "server.h"
#include "tcp_handler.h"
#include <iostream>
#include <csignal>
#include <memory>

std::shared_ptr<ChatServer> global_server;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\n[SHUTDOWN] Apagando servidor..." << std::endl;
        if (global_server) {
            global_server->stop();
        }
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto>" << std::endl;
        return 1;
    }

    uint16_t port;
    try {
        port = std::stoi(argv[1]);
    } catch (...) {
        std::cerr << "Error: Puerto invalido" << std::endl;
        return 1;
    }

    TCPHandler::init_network();
    std::signal(SIGINT, signal_handler);

    global_server = std::make_shared<ChatServer>(port);

    try {
        global_server->run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        TCPHandler::cleanup_network();
        return 1;
    }

    TCPHandler::cleanup_network();
    return 0;
}
