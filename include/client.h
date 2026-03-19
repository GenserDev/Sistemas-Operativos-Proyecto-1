#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <thread>
#include <atomic>

class ChatClient {
public:
    ChatClient(const std::string& username, const std::string& server_host, uint16_t server_port);
    ~ChatClient();

    bool connect_to_server();
    void disconnect();
    void run();

    // Envío de mensajes
    bool send_register();
    bool send_broadcast(const std::string& message);
    bool send_dm(const std::string& recipient, const std::string& message);
    bool request_user_list();
    bool request_user_info(const std::string& username);
    bool change_status(int status);
    bool send_quit();

private:
    std::string username;
    std::string server_host;
    uint16_t server_port;
    int socket_fd;
    std::atomic<bool> running;
    std::thread receive_thread;
    std::string local_ip;

    void receive_loop();
    void process_server_message(const std::string& raw_message);
    void display_help();
    void handle_user_input();
};

#endif // CLIENT_H
