#include "message_handler.h"
#include <cstring>
#include <iostream>
#include <stdexcept>

std::string MessageHeader::serialize() const {
    std::string header(5, '\0');
    
    // 1 byte: type
    header[0] = type;
    
    // 4 bytes: length en big-endian
    header[1] = (length >> 24) & 0xFF;
    header[2] = (length >> 16) & 0xFF;
    header[3] = (length >> 8) & 0xFF;
    header[4] = length & 0xFF;
    
    return header;
}

MessageHeader MessageHeader::deserialize(const std::string& data) {
    if (data.size() < 5) {
        throw std::runtime_error("Buffer insuficiente para desserializar encabezado");
    }

    MessageHeader header;
    
    // 1 byte: type
    header.type = static_cast<uint8_t>(data[0]);
    
    // 4 bytes: length en big-endian
    header.length = 0;
    header.length |= (static_cast<uint8_t>(data[1]) << 24);
    header.length |= (static_cast<uint8_t>(data[2]) << 16);
    header.length |= (static_cast<uint8_t>(data[3]) << 8);
    header.length |= static_cast<uint8_t>(data[4]);
    
    return header;
}

std::string MessageHandler::wrap_message(MessageType type, const std::string& payload) {
    MessageHeader header;
    header.type = static_cast<uint8_t>(type);
    header.length = payload.size();
    
    std::string header_data = header.serialize();
    return header_data + payload;
}

bool MessageHandler::unwrap_message(
    const std::string& data,
    MessageType& type,
    std::string& payload
) {
    if (data.size() < 5) {
        std::cerr << "Error: Buffer insuficiente para desserializar mensaje" << std::endl;
        return false;
    }

    try {
        MessageHeader header = MessageHeader::deserialize(data);
        
        if (data.size() < 5 + header.length) {
            std::cerr << "Error: Buffer incompleto. Se esperaban "
                      << (5 + header.length) << " bytes, se recibieron "
                      << data.size() << std::endl;
            return false;
        }

        type = to_message_type(header.type);
        payload = data.substr(5, header.length);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error al desserializar mensaje: " << e.what() << std::endl;
        return false;
    }
}

MessageType MessageHandler::to_message_type(uint8_t value) {
    switch (value) {
        case 1: return MessageType::REGISTER;
        case 2: return MessageType::MESSAGE_GENERAL;
        case 3: return MessageType::MESSAGE_DM;
        case 4: return MessageType::CHANGE_STATUS;
        case 5: return MessageType::LIST_USERS;
        case 6: return MessageType::GET_USER_INFO;
        case 7: return MessageType::QUIT;
        case 10: return MessageType::SERVER_RESPONSE;
        case 11: return MessageType::ALL_USERS;
        case 12: return MessageType::FOR_DM;
        case 13: return MessageType::BROADCAST_MESSAGES;
        case 14: return MessageType::GET_USER_INFO_RESPONSE;
        default:
            throw std::runtime_error("Tipo de mensaje desconocido: " + std::to_string(value));
    }
}
