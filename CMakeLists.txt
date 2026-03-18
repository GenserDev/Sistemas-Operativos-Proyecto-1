cmake_minimum_required(VERSION 3.10)
project(SimpleChatProtocol CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontrar Protobuf
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

# Directorio de salida de protobuf
set(PROTO_SRC_DIR "${CMAKE_BINARY_DIR}/protos")
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})

# Compilar proto files
file(GLOB PROTO_FILES 
    "${CMAKE_SOURCE_DIR}/protos/common.proto"
    "${CMAKE_SOURCE_DIR}/protos/cliente-side/*.proto"
    "${CMAKE_SOURCE_DIR}/protos/server-side/*.proto"
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS 
    PROTOC_OUT_DIR ${PROTO_SRC_DIR}
    ${PROTO_FILES}
)

# Incluir directorios
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${PROTO_SRC_DIR})

# Archivos fuente comunes (utils)
set(UTILS_SRCS
    src/utils/tcp_handler.cpp
    src/utils/message_handler.cpp
)

# Servidor
add_executable(nombe_del_servidor 
    src/server/main.cpp
    src/server/server.cpp
    src/server/client_handler.cpp
    ${UTILS_SRCS}
    ${PROTO_SRCS}
)
target_link_libraries(nombe_del_servidor 
    ${PROTOBUF_LIBRARIES}
    pthread
    ws2_32  # Para Windows sockets
)

# Cliente
add_executable(nombe_del_cliente
    src/client/main.cpp
    src/client/client.cpp
    ${UTILS_SRCS}
    ${PROTO_SRCS}
)
target_link_libraries(nombe_del_cliente
    ${PROTOBUF_LIBRARIES}
    pthread
    ws2_32  # Para Windows sockets
)
