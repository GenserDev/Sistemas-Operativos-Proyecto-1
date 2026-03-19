#include "server.h"
#include "tcp_handler.h"
#include "message_handler.h"

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
#include <algorithm>

ChatServer::ChatServer(uint16_t port)
    : port(port), server_socket(-1), running(false) {
}

ChatServer::~ChatServer() {
    stop();
}

void ChatServer::stop() {
    running = false;

    if (server_socket >= 0) {
        TCPHandler::close_socket(server_socket);
        server_socket = -1;
    }

    if (inactivity_thread.joinable()) {
        inactivity_thread.join();
    }

    for (auto& t : client_threads) {
        if (t.joinable()) {
            t.detach();
        }
    }

    std::lock_guard<std::mutex> lock(users_mutex);
    users.clear();
    socket_to_user.clear();

    std::cout << "[SERVER] Servidor detenido" << std::endl;
}

void ChatServer::run() {
    server_socket = TCPHandler::create_server("0.0.0.0", port);
    if (server_socket < 0) {
        std::cerr << "Error: No se pudo iniciar el servidor" << std::endl;
        return;
    }

    running = true;

    std::cout << "\n=== SERVIDOR DE CHAT INICIADO ===" << std::endl;
    std::cout << "Puerto: " << port << std::endl;
    std::cout << "Timeout inactividad: " << INACTIVITY_SECONDS << " segundos" << std::endl;
    std::cout << "Esperando conexiones...\n" << std::endl;

    // Thread para verificar inactividad
    inactivity_thread = std::thread(&ChatServer::check_inactivity, this);

    while (running) {
        int client_socket = TCPHandler::accept_connection(server_socket);
        if (client_socket < 0) {
            continue;
        }

        client_threads.emplace_back(&ChatServer::handle_client, this, client_socket);
    }
}

bool ChatServer::register_user(const std::string& username, const std::string& ip, int socket_fd) {
    std::lock_guard<std::mutex> lock(users_mutex);

    // Verificar nombre duplicado
    if (users.find(username) != users.end()) {
        std::cout << "[REGISTRO] Rechazado: nombre '" << username << "' ya existe" << std::endl;
        return false;
    }

    // Verificar IP duplicada
    for (const auto& pair : users) {
        if (pair.second.ip == ip) {
            std::cout << "[REGISTRO] Rechazado: IP " << ip << " ya registrada por " << pair.first << std::endl;
            return false;
        }
    }

    User user;
    user.username = username;
    user.ip = ip;
    user.socket_fd = socket_fd;
    user.status = 0; // ACTIVE
    user.last_activity = std::chrono::steady_clock::now();

    users[username] = user;
    socket_to_user[socket_fd] = username;

    std::cout << "[REGISTRO] " << username << " (" << ip << ") registrado exitosamente" << std::endl;
    return true;
}

bool ChatServer::unregister_user(const std::string& username) {
    std::lock_guard<std::mutex> lock(users_mutex);

    auto it = users.find(username);
    if (it == users.end()) {
        return false;
    }

    int sock = it->second.socket_fd;
    socket_to_user.erase(sock);
    users.erase(it);

    std::cout << "[LOGOUT] " << username << " desconectado" << std::endl;
    return true;
}

User* ChatServer::get_user(const std::string& username) {
    auto it = users.find(username);
    if (it == users.end()) return nullptr;
    return &it->second;
}

std::vector<User> ChatServer::get_all_users() {
    std::lock_guard<std::mutex> lock(users_mutex);
    std::vector<User> list;
    for (const auto& pair : users) {
        list.push_back(pair.second);
    }
    return list;
}

void ChatServer::touch_user(const std::string& username) {
    std::lock_guard<std::mutex> lock(users_mutex);
    auto it = users.find(username);
    if (it != users.end()) {
        it->second.last_activity = std::chrono::steady_clock::now();
        // Si estaba INACTIVE (2) por inactividad, restaurar a ACTIVE
        if (it->second.status == 2) {
            it->second.status = 0;
            std::cout << "[INACTIVIDAD] " << username << " restaurado a ACTIVE por actividad" << std::endl;
        }
    }
}

void ChatServer::check_inactivity() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(10)); // Revisar cada 10s

        std::lock_guard<std::mutex> lock(users_mutex);
        auto now = std::chrono::steady_clock::now();

        for (auto& pair : users) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - pair.second.last_activity).count();

            // Si lleva mas del tiempo limite y no esta ya INVISIBLE
            if (elapsed >= INACTIVITY_SECONDS && pair.second.status == 0) {
                pair.second.status = 2; // INVISIBLE (INACTIVO)
                std::cout << "[INACTIVIDAD] " << pair.first
                          << " marcado como INACTIVE (" << elapsed << "s)" << std::endl;

                // Notificar al cliente
                chat::ServerResponse resp;
                resp.set_status_code(200);
                resp.set_message("Estado cambiado a INACTIVE por inactividad");
                resp.set_is_successful(true);

                std::string payload = resp.SerializeAsString();
                std::string wrapped = MessageHandler::wrap_message(MessageType::SERVER_RESPONSE, payload);
                TCPHandler::send_all(pair.second.socket_fd, wrapped);
            }
        }
    }
}

void ChatServer::send_server_response(int socket_fd, int status_code, const std::string& msg, bool success) {
    chat::ServerResponse resp;
    resp.set_status_code(status_code);
    resp.set_message(msg);
    resp.set_is_successful(success);

    std::string payload = resp.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::SERVER_RESPONSE, payload);
    TCPHandler::send_all(socket_fd, wrapped);
}

void ChatServer::send_user_list(int socket_fd) {
    chat::AllUsers all_users;
    std::vector<User> list = get_all_users();
    for (const auto& u : list) {
        all_users.add_usernames(u.username);
        all_users.add_status(static_cast<chat::StatusEnum>(u.status));
    }

    std::string payload = all_users.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::ALL_USERS, payload);
    TCPHandler::send_all(socket_fd, wrapped);
}

void ChatServer::send_user_info(int socket_fd, const std::string& target_username) {
    std::lock_guard<std::mutex> lock(users_mutex);
    auto it = users.find(target_username);

    if (it != users.end()) {
        chat::GetUserInfoResponse resp;
        resp.set_ip_address(it->second.ip);
        resp.set_username(it->second.username);
        resp.set_status(static_cast<chat::StatusEnum>(it->second.status));

        std::string payload = resp.SerializeAsString();
        std::string wrapped = MessageHandler::wrap_message(MessageType::GET_USER_INFO_RESPONSE, payload);
        TCPHandler::send_all(socket_fd, wrapped);
    } else {
        // Desbloquear antes de llamar send_server_response
        // (necesitamos unlock manual aquí)
        chat::ServerResponse resp;
        resp.set_status_code(404);
        resp.set_message("Usuario no encontrado: " + target_username);
        resp.set_is_successful(false);

        std::string payload = resp.SerializeAsString();
        std::string wrapped = MessageHandler::wrap_message(MessageType::SERVER_RESPONSE, payload);
        TCPHandler::send_all(socket_fd, wrapped);
    }
}

void ChatServer::broadcast_message(const std::string& message, const std::string& username_origin) {
    std::lock_guard<std::mutex> lock(users_mutex);

    chat::BroadcastDelivery broadcast;
    broadcast.set_message(message);
    broadcast.set_username_origin(username_origin);

    std::string payload = broadcast.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::BROADCAST_MESSAGES, payload);

    std::cout << "[BROADCAST] " << username_origin << ": " << message << std::endl;

    for (const auto& pair : users) {
        if (pair.first != username_origin) {
            TCPHandler::send_all(pair.second.socket_fd, wrapped);
        }
    }
}

void ChatServer::send_dm(const std::string& message, const std::string& username_des,
                          const std::string& username_origin, int origin_socket) {
    std::lock_guard<std::mutex> lock(users_mutex);

    auto it = users.find(username_des);
    if (it == users.end()) {
        std::cout << "[DM] Destinatario " << username_des << " no encontrado" << std::endl;
        // Enviar error al remitente
        chat::ServerResponse resp;
        resp.set_status_code(404);
        resp.set_message("Usuario no encontrado: " + username_des);
        resp.set_is_successful(false);
        std::string rp = resp.SerializeAsString();
        std::string rw = MessageHandler::wrap_message(MessageType::SERVER_RESPONSE, rp);
        TCPHandler::send_all(origin_socket, rw);
        return;
    }

    chat::ForDm dm;
    dm.set_username_des(username_origin); // quien envia
    dm.set_message(message);

    std::string payload = dm.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::FOR_DM, payload);

    std::cout << "[DM] " << username_origin << " -> " << username_des << ": " << message << std::endl;
    TCPHandler::send_all(it->second.socket_fd, wrapped);
}

void ChatServer::update_user_status(const std::string& username, int new_status, int socket_fd) {
    {
        std::lock_guard<std::mutex> lock(users_mutex);
        auto it = users.find(username);
        if (it != users.end()) {
            it->second.status = new_status;

            std::string status_names[] = {"ACTIVE", "DO_NOT_DISTURB", "INVISIBLE"};
            std::cout << "[STATUS] " << username << " -> " << status_names[new_status] << std::endl;
        }
    }

    // Confirmar cambio al cliente
    send_server_response(socket_fd, 200, "Estado actualizado", true);
}

void ChatServer::handle_client(int client_socket) {
    std::string client_username;
    bool authenticated = false;
    std::string client_ip = TCPHandler::get_peer_ip(client_socket);

    while (running) {
        std::string raw_message = TCPHandler::receive_full_message(client_socket);

        if (raw_message.empty()) {
            break; // Cliente desconectado
        }

        MessageType type;
        std::string payload;

        if (!MessageHandler::unwrap_message(raw_message, type, payload)) {
            continue;
        }

        // Actualizar actividad del usuario
        if (authenticated) {
            touch_user(client_username);
        }

        switch (type) {
            case MessageType::REGISTER: {
                chat::Register reg;
                if (!reg.ParseFromString(payload)) {
                    send_server_response(client_socket, 400, "Mensaje malformado", false);
                    break;
                }

                std::string ip = reg.ip().empty() ? client_ip : reg.ip();

                if (register_user(reg.username(), ip, client_socket)) {
                    client_username = reg.username();
                    authenticated = true;

                    send_server_response(client_socket, 200, "Registro exitoso", true);
                } else {
                    send_server_response(client_socket, 409,
                        "Nombre de usuario o IP ya registrados", false);
                }
                break;
            }

            case MessageType::MESSAGE_GENERAL: {
                if (!authenticated) break;

                chat::MessageGeneral msg;
                msg.ParseFromString(payload);
                broadcast_message(msg.message(), client_username);
                break;
            }

            case MessageType::MESSAGE_DM: {
                if (!authenticated) break;

                chat::MessageDM dm;
                dm.ParseFromString(payload);
                send_dm(dm.message(), dm.username_des(), client_username, client_socket);
                break;
            }

            case MessageType::CHANGE_STATUS: {
                if (!authenticated) break;

                chat::ChangeStatus cs;
                cs.ParseFromString(payload);

                int new_status = cs.status();
                if (new_status >= 0 && new_status <= 2) {
                    update_user_status(client_username, new_status, client_socket);
                } else {
                    send_server_response(client_socket, 400, "Estado invalido", false);
                }
                break;
            }

            case MessageType::LIST_USERS: {
                if (!authenticated) break;
                send_user_list(client_socket);
                break;
            }

            case MessageType::GET_USER_INFO: {
                if (!authenticated) break;

                chat::GetUserInfo gui;
                gui.ParseFromString(payload);
                send_user_info(client_socket, gui.username_des());
                break;
            }

            case MessageType::QUIT: {
                if (authenticated) {
                    unregister_user(client_username);
                    authenticated = false;
                }
                goto done;
            }

            default:
                std::cout << "[ERROR] Tipo de mensaje desconocido: "
                          << static_cast<int>(type) << std::endl;
                break;
        }
    }

done:
    // Si se desconectó sin QUIT, limpiar
    if (authenticated) {
        unregister_user(client_username);
    }
    TCPHandler::close_socket(client_socket);
}
