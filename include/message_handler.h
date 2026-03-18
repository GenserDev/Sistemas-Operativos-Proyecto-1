#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <string>
#include <cstdint>
#include <memory>

/**
 * Constantes para tipos de mensaje según el protocolo
 */
enum class MessageType : uint8_t {
    // Cliente -> Servidor
    REGISTER = 1,
    MESSAGE_GENERAL = 2,
    MESSAGE_DM = 3,
    CHANGE_STATUS = 4,
    LIST_USERS = 5,
    GET_USER_INFO = 6,
    QUIT = 7,

    // Servidor -> Cliente
    SERVER_RESPONSE = 10,
    ALL_USERS = 11,
    FOR_DM = 12,
    BROADCAST_MESSAGES = 13,
    GET_USER_INFO_RESPONSE = 14
};

/**
 * Estructura del encabezado TCP (5 bytes)
 * 1 byte: type (MessageType)
 * 4 bytes: payload length (big-endian uint32)
 */
struct MessageHeader {
    uint8_t type;
    uint32_t length;

    // Serializar encabezado a 5 bytes
    std::string serialize() const;
    
    // Deserializar encabezado desde 5 bytes
    static MessageHeader deserialize(const std::string& data);
};

/**
 * Maneja la serialización y deserialización de mensajes
 * Encapsula la lógica de protobuf y encabezados TCP
 */
class MessageHandler {
public:
    /**
     * Serializa un mensaje protobuf con encabezado
     * @param type Tipo de mensaje (MessageType)
     * @param payload Datos serializados de protobuf
     * @return Mensaje completo con encabezado + payload
     */
    static std::string wrap_message(MessageType type, const std::string& payload);

    /**
     * Deserializa un mensaje TCP completo
     * @param data Datos completos (encabezado + payload)
     * @param type Referencia donde guardar el tipo
     * @param payload Referencia donde guardar el payload
     * @return true si la deserialización fue exitosa
     */
    static bool unwrap_message(
        const std::string& data,
        MessageType& type,
        std::string& payload
    );

    // Utilidades para conversión de tipos
    static MessageType to_message_type(uint8_t value);
};

#endif // MESSAGE_HANDLER_H
