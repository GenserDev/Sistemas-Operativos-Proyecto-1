#include "client.h"
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
#include <thread>
#include <sstream>
#include <chrono>

ChatClient::ChatClient(const std::string& server_host, uint16_t server_port)
    : server_host(server_host), server_port(server_port), socket_fd(-1),
      current_username(""), running(false) {
}

ChatClient::~ChatClient() {
    disconnect();
}

bool ChatClient::connect() {
    TCPHandler tcp;
    
    if (!tcp.connect(server_host, server_port)) {
        std::cerr << "Error: No se pudo conectar al servidor" << std::endl;
        return false;
    }

    socket_fd = tcp.get_socket();
    running = true;
    return true;
}

void ChatClient::disconnect() {
    running = false;

    if (receive_thread.joinable()) {
        receive_thread.join();
    }

    if (socket_fd != -1) {
        TCPHandler tcp;
        tcp.close_socket(socket_fd);
        socket_fd = -1;
    }

    std::cout << "\nDesconectado del servidor" << std::endl;
}

bool ChatClient::send_register(const std::string& username) {
    current_username = username;

    chat::Register reg;
    reg.set_username(username);
    reg.set_ip(get_local_ip());

    std::string payload = reg.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::REGISTER, payload);

    TCPHandler tcp;
    return tcp.send_message(socket_fd, wrapped);
}

bool ChatClient::send_message(const std::string& message) {
    if (current_username.empty()) {
        std::cerr << "Error: Primero debe registrarse" << std::endl;
        return false;
    }

    chat::MessageGeneral msg;
    msg.set_message(message);
    msg.set_status(static_cast<chat::StatusEnum>(0));  // ACTIVE
    msg.set_username_origin(current_username);
    msg.set_ip(get_local_ip());

    std::string payload = msg.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::MESSAGE_GENERAL, payload);

    TCPHandler tcp;
    return tcp.send_message(socket_fd, wrapped);
}

bool ChatClient::send_dm(const std::string& recipient, const std::string& message) {
    if (current_username.empty()) {
        std::cerr << "Error: Primero debe registrarse" << std::endl;
        return false;
    }

    chat::MessageDM dm;
    dm.set_message(message);
    dm.set_status(static_cast<chat::StatusEnum>(0));  // ACTIVE
    dm.set_username_des(recipient);
    dm.set_ip(get_local_ip());

    std::string payload = dm.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::MESSAGE_DM, payload);

    TCPHandler tcp;
    return tcp.send_message(socket_fd, wrapped);
}

bool ChatClient::request_user_list() {
    chat::ListUsers list_req;
    list_req.set_username(current_username);
    list_req.set_ip(get_local_ip());

    std::string payload = list_req.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::LIST_USERS, payload);

    TCPHandler tcp;
    return tcp.send_message(socket_fd, wrapped);
}

bool ChatClient::request_user_info(const std::string& username) {
    chat::GetUserInfo gui;
    gui.set_username_des(username);
    gui.set_username(current_username);
    gui.set_ip(get_local_ip());

    std::string payload = gui.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::GET_USER_INFO, payload);

    TCPHandler tcp;
    return tcp.send_message(socket_fd, wrapped);
}

bool ChatClient::change_status(int status) {
    chat::ChangeStatus cs;
    cs.set_status(static_cast<chat::StatusEnum>(status));
    cs.set_username(current_username);
    cs.set_ip(get_local_ip());

    std::string payload = cs.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::CHANGE_STATUS, payload);

    TCPHandler tcp;
    return tcp.send_message(socket_fd, wrapped);
}

bool ChatClient::send_quit() {
    chat::Quit quit_msg;
    quit_msg.set_quit(true);
    quit_msg.set_ip(get_local_ip());

    std::string payload = quit_msg.SerializeAsString();
    std::string wrapped = MessageHandler::wrap_message(MessageType::QUIT, payload);

    TCPHandler tcp;
    return tcp.send_message(socket_fd, wrapped);
}

void ChatClient::run() {
    std::string username;

    std::cout << "\n=== CLIENTE DE CHAT ===" << std::endl;
    std::cout << "Ingrese su nombre de usuario: ";
    std::getline(std::cin, username);

    if (!send_register(username)) {
        std::cerr << "Error: No se pudo enviar registro" << std::endl;
        return;
    }

    // Iniciar thread de recepción
    receive_thread = std::thread(&ChatClient::receive_loop, this);

    // Dar tiempo para recibir respuesta del servidor
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Loop de envío (entrada del usuario)
    handle_user_input();

    // Enviar quit
    send_quit();
    disconnect();
}

void ChatClient::receive_loop() {
    TCPHandler tcp;

    while (running) {
        std::string raw_message = tcp.receive_message(socket_fd);

        if (raw_message.empty()) {
            std::cout << "\n[DESCONEXIÓN] Servidor cerró la conexión" << std::endl;
            running = false;
            break;
        }

        MessageType type;
        std::string payload;

        if (!MessageHandler::unwrap_message(raw_message, type, payload)) {
            continue;
        }

        process_server_message(raw_message);
    }
}

void ChatClient::process_server_message(const std::string& raw_message) {
    MessageType type;
    std::string payload;

    if (!MessageHandler::unwrap_message(raw_message, type, payload)) {
        return;
    }

    switch (type) {
        case MessageType::SERVER_RESPONSE: {
            chat::ServerResponse response;
            response.ParseFromString(payload);

            std::cout << "\n[SERVIDOR] " << response.message() << std::endl;
            if (response.is_successful()) {
                std::cout << "[✓] Exitoso" << std::endl;
            } else {
                std::cout << "[✗] Falló" << std::endl;
            }
            break;
        }

        case MessageType::ALL_USERS: {
            chat::AllUsers all_users;
            all_users.ParseFromString(payload);

            std::cout << "\n[USUARIOS CONECTADOS]" << std::endl;
            for (int i = 0; i < all_users.usernames_size(); ++i) {
                std::string status_str;
                switch (all_users.status(i)) {
                    case 0: status_str = "ACTIVO"; break;
                    case 1: status_str = "NO MOLESTAR"; break;
                    case 2: status_str = "INVISIBLE"; break;
                    default: status_str = "DESCONOCIDO"; break;
                }
                std::cout << "  - " << all_users.usernames(i) << " (" << status_str << ")" << std::endl;
            }
            break;
        }

        case MessageType::FOR_DM: {
            chat::ForDm dm;
            dm.ParseFromString(payload);

            std::cout << "\n[DM de " << dm.username_des() << "]: " << dm.message() << std::endl;
            break;
        }

        case MessageType::BROADCAST_MESSAGES: {
            chat::BroadcastDelivery broadcast;
            broadcast.ParseFromString(payload);

            std::cout << "\n[CHAT GENERAL] " << broadcast.username_origin() << ": "
                      << broadcast.message() << std::endl;
            break;
        }

        case MessageType::GET_USER_INFO_RESPONSE: {
            chat::GetUserInfoResponse response;
            response.ParseFromString(payload);

            std::cout << "\n[INFO DE USUARIO]" << std::endl;
            std::cout << "  Usuario: " << response.username() << std::endl;
            std::cout << "  IP: " << response.ip_address() << std::endl;
            
            std::string status_str;
            switch (response.status()) {
                case 0: status_str = "ACTIVO"; break;
                case 1: status_str = "NO MOLESTAR"; break;
                case 2: status_str = "INVISIBLE"; break;
                default: status_str = "DESCONOCIDO"; break;
            }
            std::cout << "  Estado: " << status_str << std::endl;
            break;
        }

        default:
            std::cout << "[?] Mensaje desconocido del servidor" << std::endl;
            break;
    }
}

void ChatClient::display_menu() {
    std::cout << "\n=== MENÚ ===" << std::endl;
    std::cout << "1. Enviar mensaje al chat general" << std::endl;
    std::cout << "2. Enviar mensaje directo" << std::endl;
    std::cout << "3. Ver lista de usuarios" << std::endl;
    std::cout << "4. Ver información de usuario" << std::endl;
    std::cout << "5. Cambiar estado" << std::endl;
    std::cout << "6. Salir" << std::endl;
    std::cout << "Opción: ";
}

void ChatClient::handle_user_input() {
    std::string input;

    while (running) {
        display_menu();
        std::getline(std::cin, input);

        if (input.empty()) continue;

        switch (input[0]) {
            case '1': {
                // Enviar mensaje al chat general
                std::cout << "Mensaje: ";
                std::getline(std::cin, input);
                send_message(input);
                break;
            }

            case '2': {
                // Enviar mensaje directo
                std::cout << "Destinatario: ";
                std::string recipient;
                std::getline(std::cin, recipient);
                
                std::cout << "Mensaje: ";
                std::string message;
                std::getline(std::cin, message);
                
                send_dm(recipient, message);
                break;
            }

            case '3': {
                // Ver lista de usuarios
                request_user_list();
                break;
            }

            case '4': {
                // Ver información de usuario
                std::cout << "Usuario a consultar: ";
                std::string username;
                std::getline(std::cin, username);
                request_user_info(username);
                break;
            }

            case '5': {
                // Cambiar estado
                std::cout << "\n0. ACTIVO" << std::endl;
                std::cout << "1. NO MOLESTAR" << std::endl;
                std::cout << "2. INVISIBLE" << std::endl;
                std::cout << "Nuevo estado: ";
                std::getline(std::cin, input);
                
                try {
                    int status = std::stoi(input);
                    if (status >= 0 && status <= 2) {
                        change_status(status);
                    }
                } catch (...) {
                    std::cout << "Estado inválido" << std::endl;
                }
                break;
            }

            case '6': {
                // Salir
                running = false;
                return;
            }

            default:
                std::cout << "Opción inválida" << std::endl;
                break;
        }
    }
}

std::string ChatClient::get_local_ip() {
    return TCPHandler::get_local_ip();
}
