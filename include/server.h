#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

/**
 * Estructura para almacenar información de un usuario conectado
 */
struct User {
    std::string username;
    std::string ip;
    int socket_fd;
    int status;  // StatusEnum (0=ACTIVE, 1=DO_NOT_DISTURB, 2=INVISIBLE)
};

/**
 * Servidor de chat que gestiona múltiples clientes
 */
class ChatServer {
public:
    ChatServer(uint16_t port = 8080);
    ~ChatServer();

    // Ciclo de vida
    bool start();
    void stop();
    void run();

    // Gestión de usuarios
    bool register_user(const std::string& username, const std::string& ip, int socket_fd);
    bool unregister_user(const std::string& username);
    User* get_user(const std::string& username);
    std::vector<User> get_all_users();

    // Gestión de mensajes
    void broadcast_message(const std::string& message, const std::string& username_origin);
    void send_dm(const std::string& message, const std::string& username_des, 
                 const std::string& username_origin);
    void update_user_status(const std::string& username, int new_status);

private:
    uint16_t port;
    int server_socket;
    bool running;
    
    // Thread pool y sincronización
    std::vector<std::thread> client_threads;
    std::mutex users_mutex;
    std::mutex sockets_mutex;

    // Almacenamiento de usuarios
    std::map<std::string, User> users;  // username -> User
    std::map<int, std::string> socket_to_user;  // socket_fd -> username

    // Métodos privados
    void handle_client(int client_socket);
    void process_message(int client_socket, const std::string& raw_message);
};

#endif // SERVER_H
