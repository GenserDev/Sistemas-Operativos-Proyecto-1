#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <memory>
#include <thread>
#include <atomic>

/**
 * Cliente de chat que se conecta a un servidor
 */
class ChatClient {
public:
    ChatClient(const std::string& server_host, uint16_t server_port);
    ~ChatClient();

    // Ciclo de vida
    bool connect();
    void disconnect();

    // Comunicación
    void run();
    bool send_register(const std::string& username);
    bool send_message(const std::string& message);
    bool send_dm(const std::string& recipient, const std::string& message);
    bool request_user_list();
    bool request_user_info(const std::string& username);
    bool change_status(int status);
    bool send_quit();

private:
    std::string server_host;
    uint16_t server_port;
    int socket_fd;
    std::string current_username;
    std::atomic<bool> running;
    std::thread receive_thread;

    // Métodos privados
    void receive_loop();
    void process_server_message(const std::string& raw_message);
    void display_menu();
    std::string get_local_ip();

    // Manejo de entrada del usuario
    void handle_user_input();
};

#endif // CLIENT_H
