#include "tcp_handler.h"
#include <iostream>
#include <cstring>
#include <stdexcept>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define CLOSE_SOCKET closesocket
    #define SOCKET_ERROR_VAL SOCKET_ERROR
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #define INVALID_SOCKET_VALUE -1
    #define CLOSE_SOCKET close
    #define SOCKET_ERROR_VAL -1
    #define SOCKET int
#endif

TCPHandler::TCPHandler() : socket_fd(-1), server_socket_fd(-1) {
#ifdef _WIN32
    initialize_winsock();
#endif
}

TCPHandler::~TCPHandler() {
    if (socket_fd != -1) {
        CLOSE_SOCKET(socket_fd);
    }
    if (server_socket_fd != -1) {
        CLOSE_SOCKET(server_socket_fd);
    }
#ifdef _WIN32
    cleanup_winsock();
#endif
}

#ifdef _WIN32
void TCPHandler::initialize_winsock() {
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
}

void TCPHandler::cleanup_winsock() {
    WSACleanup();
}
#endif

bool TCPHandler::listen(const std::string& host, uint16_t port) {
    server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket_fd == INVALID_SOCKET_VALUE) {
        std::cerr << "Error: No se pudo crear el socket del servidor" << std::endl;
        return false;
    }

    // Permitir reutilización del puerto
    int reuse = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR,
#ifdef _WIN32
                   (const char*)
#else
                   (const void*)
#endif
                   &reuse, sizeof(reuse)) == SOCKET_ERROR_VAL) {
        std::cerr << "Error: No se pudo establecer SO_REUSEADDR" << std::endl;
        return false;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (host == "0.0.0.0" || host == "*") {
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);
    }

    if (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR_VAL) {
        std::cerr << "Error: No se pudo hacer bind al puerto " << port << std::endl;
        CLOSE_SOCKET(server_socket_fd);
        server_socket_fd = -1;
        return false;
    }

    if (::listen(server_socket_fd, 5) == SOCKET_ERROR_VAL) {
        std::cerr << "Error: No se pudo escuchar en el socket" << std::endl;
        CLOSE_SOCKET(server_socket_fd);
        server_socket_fd = -1;
        return false;
    }

    std::cout << "Servidor escuchando en " << host << ":" << port << std::endl;
    return true;
}

int TCPHandler::accept_connection() {
    if (server_socket_fd == -1) {
        std::cerr << "Error: Server socket no inicializado" << std::endl;
        return -1;
    }

    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);
    
    int client_socket = accept(server_socket_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_socket == INVALID_SOCKET_VALUE) {
        std::cerr << "Error: No se pudo aceptar conexión" << std::endl;
        return -1;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    std::cout << "Cliente conectado desde " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

    return client_socket;
}

bool TCPHandler::connect(const std::string& host, uint16_t port) {
    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == INVALID_SOCKET_VALUE) {
        std::cerr << "Error: No se pudo crear el socket" << std::endl;
        return false;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Error: Dirección IP inválida: " << host << std::endl;
        CLOSE_SOCKET(socket_fd);
        socket_fd = -1;
        return false;
    }

    if (::connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR_VAL) {
        std::cerr << "Error: No se pudo conectar al servidor" << std::endl;
        CLOSE_SOCKET(socket_fd);
        socket_fd = -1;
        return false;
    }

    std::cout << "Conectado al servidor " << host << ":" << port << std::endl;
    return true;
}

bool TCPHandler::send_message(int socket_fd, const std::string& data) {
    if (socket_fd == -1) {
        std::cerr << "Error: Socket no válido" << std::endl;
        return false;
    }

    size_t total_sent = 0;
    size_t data_size = data.size();

    while (total_sent < data_size) {
        int sent = send(socket_fd, data.c_str() + total_sent, data_size - total_sent, 0);
        if (sent == SOCKET_ERROR_VAL) {
            std::cerr << "Error: No se pudo enviar datos" << std::endl;
            return false;
        }
        total_sent += sent;
    }

    return true;
}

std::string TCPHandler::receive_message(int socket_fd, int buffer_size) {
    if (socket_fd == -1) {
        std::cerr << "Error: Socket no válido" << std::endl;
        return "";
    }

    char buffer[4096];
    int received = recv(socket_fd, buffer, std::min(buffer_size, 4096), 0);
    
    if (received == SOCKET_ERROR_VAL) {
        std::cerr << "Error: No se pudo recibir datos" << std::endl;
        return "";
    }

    if (received == 0) {
        // Conexión cerrada
        return "";
    }

    return std::string(buffer, received);
}

bool TCPHandler::close_socket(int socket_fd) {
    if (socket_fd == -1) {
        return true;
    }

    if (CLOSE_SOCKET(socket_fd) == SOCKET_ERROR_VAL) {
        std::cerr << "Error: No se pudo cerrar el socket" << std::endl;
        return false;
    }

    return true;
}

std::string TCPHandler::get_local_ip() {
    // Obtener IP local del sistema
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    struct hostent* host_entry = gethostbyname(hostname);
    if (!host_entry) {
        return "127.0.0.1";
    }

    struct in_addr** addr_list = (struct in_addr**)host_entry->h_addr_list;
    if (addr_list[0]) {
        return inet_ntoa(*addr_list[0]);
    }

    return "127.0.0.1";
}
