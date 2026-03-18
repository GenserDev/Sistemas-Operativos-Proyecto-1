#include "client.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::string server_host = "127.0.0.1";
    uint16_t server_port = 8080;

    // Parsear argumentos
    if (argc > 1) {
        server_host = argv[1];
    }
    if (argc > 2) {
        try {
            server_port = std::stoi(argv[2]);
        } catch (...) {
            std::cerr << "Error: Puerto inválido" << std::endl;
            return 1;
        }
    }

    std::cout << "Conectando a " << server_host << ":" << server_port << "..." << std::endl;

    ChatClient client(server_host, server_port);

    if (!client.connect()) {
        std::cerr << "Error: No se pudo conectar al servidor" << std::endl;
        return 1;
    }

    try {
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
