cmake_minimum_required(VERSION 3.10)
project(SimpleChatProtocol CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontrar Protobuf
find_package(Protobuf REQUIRED)

# Directorio de archivos generados
set(PROTO_SRC_DIR "${CMAKE_SOURCE_DIR}/protos")

# Incluir directorios
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${PROTO_SRC_DIR})
include_directories(${PROTOBUF_INCLUDE_DIRS})

# Archivos fuente comunes (utils)
set(UTILS_SRCS
    src/utils/tcp_handler.cpp
    src/utils/message_handler.cpp
)

# Archivos proto compilados
file(GLOB PROTO_GENERATED_SRCS 
    "${PROTO_SRC_DIR}/*.pb.cc"
)

# Servidor
add_executable(nombe_del_servidor 
    src/server/main.cpp
    src/server/server.cpp
    src/server/client_handler.cpp
    ${UTILS_SRCS}
    ${PROTO_GENERATED_SRCS}
)

# Cliente
add_executable(nombe_del_cliente
    src/client/main.cpp
    src/client/client.cpp
    ${UTILS_SRCS}
    ${PROTO_GENERATED_SRCS}
)

target_link_libraries(nombe_del_servidor pthread ${PROTOBUF_LIBRARIES})
target_link_libraries(nombe_del_cliente pthread ${PROTOBUF_LIBRARIES})
