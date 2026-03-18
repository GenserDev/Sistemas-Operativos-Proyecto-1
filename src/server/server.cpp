#include "server.h"
#include "tcp_handler.h"
#include "message_handler.h"

// Importar proto files
#include "register.pb.h"
#include "message_general.pb.h"
#include "message_dm.pb.h"
#include "change_status.pb.h"
#include "list_users.pb.h"
#include "get_user_info.pb.h"
#include "quit.pb.h"
#include "server_response.pb.h"
#include "all_users.pb.h"
#include "for_dm.pb.h"
#include "broadcast_messages.pb.h"
#include "get_user_info_response.pb.h"

#include <iostream>
#include <sstream>
#include <algorithm>

ChatServer::ChatServer(uint16_t port)
    : port(port), server_socket(-1), running(false) {
}

ChatServer::~ChatServer() {
    stop();
}

bool ChatServer::start() {
    TCPHandler tcp;
    
    if (!tcp.listen("0.0.0.0", port)) {
        std::cerr << "Error: No se pudo inicializar el servidor" << std::endl;
        return false;
    }

    server_socket = tcp.get_socket();
    running = true;

    std::cout << "\n=== SERVIDOR DE CHAT INICIADO ===" << std::endl;
    std::cout << "Escuchando en puerto: " << port << std::endl;
    std::cout << "Esperando conexiones..." << std::endl;

    return true;
}

void ChatServer::stop() {
    running = false;
    
    for (auto& thread : client_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    std::lock_guard<std::mutex> lock(users_mutex);
    users.clear();

    std::cout << "Servidor detenido" << std::endl;
}

void ChatServer::run() {
    TCPHandler tcp;
    
    if (!tcp.listen("0.0.0.0", port)) {
        std::cerr << "Error: No se pudo iniciar el servidor" << std::endl;
        return;
    }

    running = true;
    std::cout << "\n=== SERVIDOR DE CHAT INICIADO ===" << std::endl;
    std::cout << "Puerto: " << port << std::endl;

    while (running) {
        int client_socket = tcp.accept_connection();
        if (client_socket == -1) {
            continue;
        }

        // Lanzar thread para manejar cliente
        client_threads.emplace_back(&ChatServer::handle_client, this, client_socket);
    }
}

bool ChatServer::register_user(const std::string& username, const std::string& ip, int socket_fd) {
    std::lock_guard<std::mutex> lock(users_mutex);

    if (users.find(username) != users.end()) {
        std::cout << "[REGISTRO] Usuario " << username << " ya existe" << std::endl;
        return false;
    }

    User user;
    user.username = username;
    user.ip = ip;
    user.socket_fd = socket_fd;
    user.status = 0;  // ACTIVE

    users[username] = user;
    socket_to_user[socket_fd] = username;

    std::cout << "[REGISTRO] Usuario " << username << " (" << ip << ") registrado" << std::endl;
    return true;
}

bool ChatServer::unregister_user(const std::string& username) {
    std::lock_guard<std::mutex> lock(users_mutex);

    auto it = users.find(username);
    if (it == users.end()) {
        return false;
    }

    int socket_fd = it->second.socket_fd;
    socket_to_user.erase(socket_fd);
    users.erase(it);

    std::cout << "[LOGOUT] Usuario " << username << " desconectado" << std::endl;
    return true;
}

User* ChatServer::get_user(const std::string& username) {
    auto it = users.find(username);
    if (it == users.end()) {
        return nullptr;
    }
    return &it->second;
}

std::vector<User> ChatServer::get_all_users() {
    std::lock_guard<std::mutex> lock(users_mutex);
    
    std::vector<User> user_list;
    for (const auto& pair : users) {
        user_list.push_back(pair.second);
    }
    return user_list;
}

void ChatServer::broadcast_message(const std::string& message, const std::string& username_origin) {
    std::lock_guard<std::mutex> lock(users_mutex);

    chat::BroadcastDelivery broadcast;
    broadcast.set_message(message);
    broadcast.set_username_origin(username_origin);

    std::string payload = broadcast.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::BROADCAST_MESSAGES, payload);

    std::cout << "[BROADCAST] " << username_origin << ": " << message << std::endl;

    // Enviar a todos los clientes excepto el origen
    for (const auto& pair : users) {
        if (pair.first != username_origin && pair.second.status != 2) {  // No invisible
            TCPHandler tcp;
            tcp.send_message(pair.second.socket_fd, wrapped);
        }
    }
}

void ChatServer::send_dm(const std::string& message, const std::string& username_des,
                         const std::string& username_origin) {
    std::lock_guard<std::mutex> lock(users_mutex);

    auto it = users.find(username_des);
    if (it == users.end()) {
        std::cout << "[DM] Destinatario " << username_des << " no encontrado" << std::endl;
        return;
    }

    chat::ForDm dm;
    dm.set_username_des(username_origin);
    dm.set_message(message);

    std::string payload = dm.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::FOR_DM, payload);

    std::cout << "[DM] " << username_origin << " -> " << username_des << ": " << message << std::endl;

    TCPHandler tcp;
    tcp.send_message(it->second.socket_fd, wrapped);
}

void ChatServer::update_user_status(const std::string& username, int new_status) {
    std::lock_guard<std::mutex> lock(users_mutex);

    auto it = users.find(username);
    if (it != users.end()) {
        it->second.status = new_status;
        std::cout << "[STATUS] " << username << " cambió estado a " << new_status << std::endl;
    }
}

void ChatServer::handle_client(int client_socket) {
    TCPHandler tcp;
    std::string client_username;
    bool authenticated = false;

    while (running) {
        std::string raw_message = tcp.receive_message(client_socket);
        
        if (raw_message.empty()) {
            break;  // Cliente desconectado
        }

        // Procesar mensaje completo
        MessageType type;
        std::string payload;

        if (!MessageHandler::unwrap_message(raw_message, type, payload)) {
            continue;
        }

        // Manejar según tipo
        switch (type) {
            case MessageType::REGISTER: {
                chat::Register reg;
                reg.ParseFromString(payload);
                
                if (register_user(reg.username(), reg.ip(), client_socket)) {
                    client_username = reg.username();
                    authenticated = true;

                    // Enviar confirmación
                    chat::ServerResponse response;
                    response.set_status_code(200);
                    response.set_message("Registro exitoso");
                    response.set_is_successful(true);

                    std::string resp_payload = response.SerializeAsString();
                    std::string wrapped = MessageHandler::wrap_message(
                        MessageType::SERVER_RESPONSE, resp_payload);
                    tcp.send_message(client_socket, wrapped);

                    // Enviar lista de usuarios
                    chat::AllUsers all_users;
                    std::vector<User> users = get_all_users();
                    for (const auto& user : users) {
                        all_users.add_usernames(user.username);
                        all_users.add_status(user.status);
                    }
                    std::string users_payload = all_users.SerializeAsString();
                    std::string users_wrapped = MessageHandler::wrap_message(
                        MessageType::ALL_USERS, users_payload);
                    tcp.send_message(client_socket, users_wrapped);
                } else {
                    chat::ServerResponse response;
                    response.set_status_code(409);
                    response.set_message("Usuario ya registrado");
                    response.set_is_successful(false);

                    std::string resp_payload = response.SerializeAsString();
                    std::string wrapped = MessageHandler::wrap_message(
                        MessageType::SERVER_RESPONSE, resp_payload);
                    tcp.send_message(client_socket, wrapped);
                }
                break;
            }

            case MessageType::MESSAGE_GENERAL: {
                if (!authenticated) break;

                chat::MessageGeneral msg;
                msg.ParseFromString(payload);
                broadcast_message(msg.message(), msg.username_origin());
                break;
            }

            case MessageType::MESSAGE_DM: {
                if (!authenticated) break;

                chat::MessageDM dm;
                dm.ParseFromString(payload);
                send_dm(dm.message(), dm.username_des(), client_username);
                break;
            }

            case MessageType::CHANGE_STATUS: {
                if (!authenticated) break;

                chat::ChangeStatus cs;
                cs.ParseFromString(payload);
                update_user_status(client_username, cs.status());
                break;
            }

            case MessageType::LIST_USERS: {
                if (!authenticated) break;

                chat::AllUsers all_users;
                std::vector<User> users = get_all_users();
                for (const auto& user : users) {
                    all_users.add_usernames(user.username);
                    all_users.add_status(user.status);
                }
                std::string payload = all_users.SerializeAsString();
                std::string wrapped = MessageHandler::wrap_message(
                    MessageType::ALL_USERS, payload);
                tcp.send_message(client_socket, wrapped);
                break;
            }

            case MessageType::GET_USER_INFO: {
                if (!authenticated) break;

                chat::GetUserInfo gui;
                gui.ParseFromString(payload);

                User* user = get_user(gui.username_des());
                if (user) {
                    chat::GetUserInfoResponse response;
                    response.set_ip_address(user->ip);
                    response.set_username(user->username);
                    response.set_status(user->status);

                    std::string resp_payload = response.SerializeAsString();
                    std::string wrapped = MessageHandler::wrap_message(
                        MessageType::GET_USER_INFO_RESPONSE, resp_payload);
                    tcp.send_message(client_socket, wrapped);
                } else {
                    chat::ServerResponse response;
                    response.set_status_code(404);
                    response.set_message("Usuario no encontrado");
                    response.set_is_successful(false);

                    std::string resp_payload = response.SerializeAsString();
                    std::string wrapped = MessageHandler::wrap_message(
                        MessageType::SERVER_RESPONSE, resp_payload);
                    tcp.send_message(client_socket, wrapped);
                }
                break;
            }

            case MessageType::QUIT: {
                if (authenticated) {
                    unregister_user(client_username);
                }
                goto exit_loop;
                break;
            }

            default:
                std::cout << "[ERROR] Tipo de mensaje desconocido: "
                          << static_cast<int>(type) << std::endl;
                break;
        }
    }

    exit_loop:
    if (authenticated) {
        unregister_user(client_username);
    }
    tcp.close_socket(client_socket);
}

void ChatServer::process_message(int client_socket, const std::string& raw_message) {
    // Este método se puede usar para procesamiento adicional si es necesario
}
