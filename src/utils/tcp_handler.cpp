#include "tcp_handler.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSE_SOCK closesocket
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #define CLOSE_SOCK ::close
#endif

void TCPHandler::init_network() {
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        std::cerr << "Error: WSAStartup failed" << std::endl;
    }
#endif
}

void TCPHandler::cleanup_network() {
#ifdef _WIN32
    WSACleanup();
#endif
}

int TCPHandler::create_server(const std::string& host, uint16_t port, int backlog) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        std::cerr << "Error: No se pudo crear socket del servidor" << std::endl;
        return -1;
    }

    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
#ifdef _WIN32
               (const char*)
#else
               (const void*)
#endif
               &reuse, sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (host == "0.0.0.0" || host == "*") {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
    }

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error: No se pudo hacer bind al puerto " << port << std::endl;
        CLOSE_SOCK(sock);
        return -1;
    }

    if (::listen(sock, backlog) < 0) {
        std::cerr << "Error: No se pudo escuchar en el socket" << std::endl;
        CLOSE_SOCK(sock);
        return -1;
    }

    return sock;
}

int TCPHandler::accept_connection(int server_socket) {
    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (client_socket < 0) {
        return -1;
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, INET_ADDRSTRLEN);
    std::cout << "[CONEXION] Cliente desde " << ip << ":" << ntohs(client_addr.sin_port) << std::endl;

    return client_socket;
}

int TCPHandler::connect_to(const std::string& host, uint16_t port) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        std::cerr << "Error: No se pudo crear socket" << std::endl;
        return -1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        std::cerr << "Error: Direccion IP invalida: " << host << std::endl;
        CLOSE_SOCK(sock);
        return -1;
    }

    if (::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error: No se pudo conectar a " << host << ":" << port << std::endl;
        CLOSE_SOCK(sock);
        return -1;
    }

    return sock;
}

bool TCPHandler::send_all(int socket_fd, const std::string& data) {
    size_t total_sent = 0;
    size_t data_size = data.size();

    while (total_sent < data_size) {
        int sent = send(socket_fd, data.c_str() + total_sent, data_size - total_sent, 0);
        if (sent <= 0) {
            return false;
        }
        total_sent += sent;
    }
    return true;
}

bool TCPHandler::recv_exact(int socket_fd, char* buffer, size_t n) {
    size_t total = 0;
    while (total < n) {
        int received = recv(socket_fd, buffer + total, n - total, 0);
        if (received <= 0) {
            return false;
        }
        total += received;
    }
    return true;
}

std::string TCPHandler::receive_full_message(int socket_fd) {
    // Leer header: 1 byte type + 4 bytes length (big-endian)
    char header[5];
    if (!recv_exact(socket_fd, header, 5)) {
        return "";
    }

    uint32_t payload_len = 0;
    payload_len |= (static_cast<uint8_t>(header[1]) << 24);
    payload_len |= (static_cast<uint8_t>(header[2]) << 16);
    payload_len |= (static_cast<uint8_t>(header[3]) << 8);
    payload_len |= static_cast<uint8_t>(header[4]);

    // Sanity check
    if (payload_len > 1048576) { // 1MB max
        std::cerr << "Error: Payload demasiado grande: " << payload_len << std::endl;
        return "";
    }

    // Leer payload
    std::string message(header, 5);
    if (payload_len > 0) {
        std::string payload(payload_len, '\0');
        if (!recv_exact(socket_fd, &payload[0], payload_len)) {
            return "";
        }
        message += payload;
    }

    return message;
}

void TCPHandler::close_socket(int socket_fd) {
    if (socket_fd >= 0) {
        CLOSE_SOCK(socket_fd);
    }
}

std::string TCPHandler::get_local_ip() {
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

std::string TCPHandler::get_peer_ip(int socket_fd) {
    sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    if (getpeername(socket_fd, (struct sockaddr*)&addr, &len) == 0) {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
        return std::string(ip);
    }
    return "0.0.0.0";
}
