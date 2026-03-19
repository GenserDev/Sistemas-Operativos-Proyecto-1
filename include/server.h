#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>

struct User {
    std::string username;
    std::string ip;
    int socket_fd;
    int status;  // 0=ACTIVE, 1=DO_NOT_DISTURB, 2=INVISIBLE
    std::chrono::steady_clock::time_point last_activity;
};

class ChatServer {
public:
    ChatServer(uint16_t port);
    ~ChatServer();

    void run();
    void stop();

    // Gestión de usuarios
    bool register_user(const std::string& username, const std::string& ip, int socket_fd);
    bool unregister_user(const std::string& username);
    User* get_user(const std::string& username);
    std::vector<User> get_all_users();

    // Mensajes
    void broadcast_message(const std::string& message, const std::string& username_origin);
    void send_dm(const std::string& message, const std::string& username_des,
                 const std::string& username_origin, int origin_socket);
    void update_user_status(const std::string& username, int new_status, int socket_fd);

    // Respuestas
    void send_server_response(int socket_fd, int status_code, const std::string& msg, bool success);
    void send_user_list(int socket_fd);
    void send_user_info(int socket_fd, const std::string& target_username);

private:
    uint16_t port;
    int server_socket;
    std::atomic<bool> running;

    std::vector<std::thread> client_threads;
    std::mutex users_mutex;

    std::map<std::string, User> users;        // username -> User
    std::map<int, std::string> socket_to_user; // socket_fd -> username

    // Inactivity
    std::thread inactivity_thread;
    static const int INACTIVITY_SECONDS = 60; // 1 minuto para fácil evaluación

    void handle_client(int client_socket);
    void check_inactivity();
    void touch_user(const std::string& username);
};

#endif // SERVER_H
