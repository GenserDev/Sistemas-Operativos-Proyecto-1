#ifndef TCP_HANDLER_H
#define TCP_HANDLER_H

#include <string>
#include <cstdint>

/**
 * Maneja las operaciones de red TCP
 * Encapsulación de sockets para Windows y Linux
 */
class TCPHandler {
public:
    // Servidor: crear socket, bind, listen
    static int create_server(const std::string& host, uint16_t port, int backlog = 10);

    // Servidor: aceptar conexión, devuelve socket del cliente
    static int accept_connection(int server_socket);

    // Cliente: conectar a servidor, devuelve socket
    static int connect_to(const std::string& host, uint16_t port);

    // Enviar datos completos por socket
    static bool send_all(int socket_fd, const std::string& data);

    // Recibir exactamente n bytes
    static bool recv_exact(int socket_fd, char* buffer, size_t n);

    // Recibir un mensaje completo con framing (5-byte header + payload)
    static std::string receive_full_message(int socket_fd);

    // Cerrar socket
    static void close_socket(int socket_fd);

    // Obtener IP local
    static std::string get_local_ip();

    // Obtener IP del peer conectado
    static std::string get_peer_ip(int socket_fd);

    // Inicializar/limpiar Winsock (solo Windows)
    static void init_network();
    static void cleanup_network();
};

#endif // TCP_HANDLER_H
