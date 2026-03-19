#include "client.h"
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
#include <sstream>
#include <chrono>
#include <thread>

ChatClient::ChatClient(const std::string& username, const std::string& server_host, uint16_t server_port)
    : username(username), server_host(server_host), server_port(server_port),
      socket_fd(-1), running(false) {
    local_ip = TCPHandler::get_local_ip();
}

ChatClient::~ChatClient() {
    disconnect();
}

bool ChatClient::connect_to_server() {
    socket_fd = TCPHandler::connect_to(server_host, server_port);
    if (socket_fd < 0) {
        return false;
    }
    running = true;
    std::cout << "Conectado a " << server_host << ":" << server_port << std::endl;
    return true;
}

void ChatClient::disconnect() {
    running = false;

    if (socket_fd >= 0) {
        TCPHandler::close_socket(socket_fd);
        socket_fd = -1;
    }

    if (receive_thread.joinable()) {
        receive_thread.join();
    }
}

bool ChatClient::send_register() {
    chat::Register reg;
    reg.set_username(username);
    reg.set_ip(local_ip);

    std::string payload = reg.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::REGISTER, payload);
    return TCPHandler::send_all(socket_fd, wrapped);
}

bool ChatClient::send_broadcast(const std::string& message) {
    chat::MessageGeneral msg;
    msg.set_message(message);
    msg.set_status(static_cast<chat::StatusEnum>(0));
    msg.set_username_origin(username);
    msg.set_ip(local_ip);

    std::string payload = msg.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::MESSAGE_GENERAL, payload);
    return TCPHandler::send_all(socket_fd, wrapped);
}

bool ChatClient::send_dm(const std::string& recipient, const std::string& message) {
    chat::MessageDM dm;
    dm.set_message(message);
    dm.set_status(static_cast<chat::StatusEnum>(0));
    dm.set_username_des(recipient);
    dm.set_ip(local_ip);

    std::string payload = dm.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::MESSAGE_DM, payload);
    return TCPHandler::send_all(socket_fd, wrapped);
}

bool ChatClient::request_user_list() {
    chat::ListUsers req;
    req.set_username(username);
    req.set_ip(local_ip);

    std::string payload = req.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::LIST_USERS, payload);
    return TCPHandler::send_all(socket_fd, wrapped);
}

bool ChatClient::request_user_info(const std::string& target) {
    chat::GetUserInfo req;
    req.set_username_des(target);
    req.set_username(username);
    req.set_ip(local_ip);

    std::string payload = req.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::GET_USER_INFO, payload);
    return TCPHandler::send_all(socket_fd, wrapped);
}

bool ChatClient::change_status(int status) {
    chat::ChangeStatus cs;
    cs.set_status(static_cast<chat::StatusEnum>(status));
    cs.set_username(username);
    cs.set_ip(local_ip);

    std::string payload = cs.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::CHANGE_STATUS, payload);
    return TCPHandler::send_all(socket_fd, wrapped);
}

bool ChatClient::send_quit() {
    chat::Quit q;
    q.set_quit(true);
    q.set_ip(local_ip);

    std::string payload = q.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::QUIT, payload);
    return TCPHandler::send_all(socket_fd, wrapped);
}

void ChatClient::run() {
    std::cout << "\n=== CLIENTE DE CHAT ===" << std::endl;
    std::cout << "Usuario: " << username << std::endl;

    // Registrarse con el servidor
    if (!send_register()) {
        std::cerr << "Error: No se pudo enviar registro" << std::endl;
        return;
    }

    // Thread de recepcion
    receive_thread = std::thread(&ChatClient::receive_loop, this);

    // Esperar respuesta del servidor
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Input del usuario
    handle_user_input();

    // Enviar quit antes de desconectar
    send_quit();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    disconnect();
}

void ChatClient::receive_loop() {
    while (running) {
        std::string raw = TCPHandler::receive_full_message(socket_fd);

        if (raw.empty()) {
            if (running) {
                std::cout << "\n[DESCONEXION] Servidor cerro la conexion" << std::endl;
                running = false;
            }
            break;
        }

        process_server_message(raw);
        std::cout << "\n> " << std::flush; // Re-mostrar prompt
    }
}

void ChatClient::process_server_message(const std::string& raw) {
    MessageType type;
    std::string payload;

    if (!MessageHandler::unwrap_message(raw, type, payload)) {
        return;
    }

    switch (type) {
        case MessageType::SERVER_RESPONSE: {
            chat::ServerResponse resp;
            resp.ParseFromString(payload);
            if (resp.is_successful()) {
                std::cout << "\n[OK] " << resp.message() << std::endl;
            } else {
                std::cout << "\n[ERROR] " << resp.message() << std::endl;
            }
            break;
        }

        case MessageType::ALL_USERS: {
            chat::AllUsers all;
            all.ParseFromString(payload);

            std::cout << "\n--- Usuarios conectados ---" << std::endl;
            for (int i = 0; i < all.usernames_size(); ++i) {
                std::string st;
                switch (all.status(i)) {
                    case 0: st = "ACTIVO"; break;
                    case 1: st = "OCUPADO"; break;
                    case 2: st = "INACTIVO"; break;
                    default: st = "DESCONOCIDO"; break;
                }
                std::cout << "  " << all.usernames(i) << " [" << st << "]" << std::endl;
            }
            std::cout << "---------------------------" << std::endl;
            break;
        }

        case MessageType::FOR_DM: {
            chat::ForDm dm;
            dm.ParseFromString(payload);
            std::cout << "\n[DM de " << dm.username_des() << "]: " << dm.message() << std::endl;
            break;
        }

        case MessageType::BROADCAST_MESSAGES: {
            chat::BroadcastDelivery bc;
            bc.ParseFromString(payload);
            std::cout << "\n[GENERAL] " << bc.username_origin() << ": " << bc.message() << std::endl;
            break;
        }

        case MessageType::GET_USER_INFO_RESPONSE: {
            chat::GetUserInfoResponse info;
            info.ParseFromString(payload);

            std::string st;
            switch (info.status()) {
                case 0: st = "ACTIVO"; break;
                case 1: st = "OCUPADO"; break;
                case 2: st = "INACTIVO"; break;
                default: st = "DESCONOCIDO"; break;
            }

            std::cout << "\n--- Info de usuario ---" << std::endl;
            std::cout << "  Usuario: " << info.username() << std::endl;
            std::cout << "  IP:      " << info.ip_address() << std::endl;
            std::cout << "  Estado:  " << st << std::endl;
            std::cout << "-----------------------" << std::endl;
            break;
        }

        default:
            break;
    }
}

void ChatClient::display_help() {
    std::cout << "\n========= AYUDA =========" << std::endl;
    std::cout << "Comandos disponibles:" << std::endl;
    std::cout << "  1 - Enviar mensaje al chat general (broadcasting)" << std::endl;
    std::cout << "  2 - Enviar mensaje directo (privado)" << std::endl;
    std::cout << "  3 - Cambiar estado (ACTIVO/OCUPADO/INACTIVO)" << std::endl;
    std::cout << "  4 - Ver lista de usuarios conectados" << std::endl;
    std::cout << "  5 - Ver informacion de un usuario" << std::endl;
    std::cout << "  6 - Ayuda (este menu)" << std::endl;
    std::cout << "  7 - Salir del chat" << std::endl;
    std::cout << "=========================" << std::endl;
}

void ChatClient::handle_user_input() {
    std::string input;

    display_help();

    while (running) {
        std::cout << "\n> ";
        std::getline(std::cin, input);

        if (input.empty() || !running) continue;

        if (input == "1") {
            // Broadcasting
            std::cout << "Mensaje: ";
            std::string msg;
            std::getline(std::cin, msg);
            if (!msg.empty()) {
                send_broadcast(msg);
            }
        }
        else if (input == "2") {
            // Mensaje directo
            std::cout << "Destinatario: ";
            std::string dest;
            std::getline(std::cin, dest);

            std::cout << "Mensaje: ";
            std::string msg;
            std::getline(std::cin, msg);

            if (!dest.empty() && !msg.empty()) {
                send_dm(dest, msg);
            }
        }
        else if (input == "3") {
            // Cambiar estado
            std::cout << "  0 - ACTIVO" << std::endl;
            std::cout << "  1 - OCUPADO" << std::endl;
            std::cout << "  2 - INACTIVO" << std::endl;
            std::cout << "Nuevo estado: ";
            std::getline(std::cin, input);

            try {
                int st = std::stoi(input);
                if (st >= 0 && st <= 2) {
                    change_status(st);
                } else {
                    std::cout << "Estado invalido" << std::endl;
                }
            } catch (...) {
                std::cout << "Estado invalido" << std::endl;
            }
        }
        else if (input == "4") {
            // Lista de usuarios
            request_user_list();
        }
        else if (input == "5") {
            // Info de usuario
            std::cout << "Nombre del usuario: ";
            std::string target;
            std::getline(std::cin, target);
            if (!target.empty()) {
                request_user_info(target);
            }
        }
        else if (input == "6") {
            display_help();
        }
        else if (input == "7") {
            running = false;
            return;
        }
        else {
            std::cout << "Opcion invalida. Escriba 6 para ayuda." << std::endl;
        }
    }
}
