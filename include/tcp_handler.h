#ifndef TCP_HANDLER_H
#define TCP_HANDLER_H

#include <string>
#include <vector>
#include <cstdint>

/**
 * Maneja las operaciones de red TCP
 * Encapsulación de sockets para Windows y Linux
 */
class TCPHandler {
public:
    TCPHandler();
    ~TCPHandler();

    // Servidor
    bool listen(const std::string& host, uint16_t port);
    int accept_connection();
    bool close_socket(int socket_fd);

    // Cliente
    bool connect(const std::string& host, uint16_t port);
    int get_socket() const { return socket_fd; }

    // Comunicación
    bool send_message(int socket_fd, const std::string& data);
    std::string receive_message(int socket_fd, int buffer_size = 4096);
    
    // Utilidades
    static std::string get_local_ip();

private:
    int socket_fd;
    int server_socket_fd;

#ifdef _WIN32
    void initialize_winsock();
    void cleanup_winsock();
#endif
};

#endif // TCP_HANDLER_H
