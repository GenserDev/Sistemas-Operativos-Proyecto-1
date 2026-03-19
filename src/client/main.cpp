#include "client.h"
#include "tcp_handler.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Uso: " << argv[0] << " <nombre_usuario> <IP_servidor> <puerto_servidor>" << std::endl;
        return 1;
    }

    std::string username = argv[1];
    std::string server_ip = argv[2];
    uint16_t server_port;

    try {
        server_port = std::stoi(argv[3]);
    } catch (...) {
        std::cerr << "Error: Puerto invalido" << std::endl;
        return 1;
    }

    TCPHandler::init_network();

    ChatClient client(username, server_ip, server_port);

    if (!client.connect_to_server()) {
        std::cerr << "Error: No se pudo conectar al servidor" << std::endl;
        TCPHandler::cleanup_network();
        return 1;
    }

    try {
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    TCPHandler::cleanup_network();
    return 0;
}
