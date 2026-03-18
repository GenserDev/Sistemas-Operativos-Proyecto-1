#include "server.h"
#include <iostream>
#include <csignal>
#include <memory>

std::shared_ptr<ChatServer> global_server;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\n\n[SHUTDOWN] Apagando servidor..." << std::endl;
        if (global_server) {
            global_server->stop();
        }
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    uint16_t port = 8080;

    // Parsear argumentos
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            std::cerr << "Error: Puerto inválido" << std::endl;
            return 1;
        }
    }

    // Configurar manejador de signal
    std::signal(SIGINT, signal_handler);

    // Crear y ejecutar servidor
    global_server = std::make_shared<ChatServer>(port);
    
    try {
        global_server->run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
