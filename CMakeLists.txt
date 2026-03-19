cmake_minimum_required(VERSION 3.10)
project(SimpleChatProtocol CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontrar Protobuf
find_package(Protobuf REQUIRED)

# Proto files
set(PROTO_DIR "${CMAKE_SOURCE_DIR}/protos")
set(PROTO_GEN_DIR "${CMAKE_BINARY_DIR}/proto_gen")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

# Lista de archivos .proto
set(PROTO_FILES
    ${PROTO_DIR}/common.proto
    ${PROTO_DIR}/register.proto
    ${PROTO_DIR}/message_general.proto
    ${PROTO_DIR}/message_dm.proto
    ${PROTO_DIR}/change_status.proto
    ${PROTO_DIR}/list_users.proto
    ${PROTO_DIR}/get_user_info.proto
    ${PROTO_DIR}/quit.proto
    ${PROTO_DIR}/server_response.proto
    ${PROTO_DIR}/all_users.proto
    ${PROTO_DIR}/for_dm.proto
    ${PROTO_DIR}/broadcast_messages.proto
    ${PROTO_DIR}/get_user_info_response.proto
)

# Generar archivos C++ desde protos
set(PROTO_SRCS)
set(PROTO_HDRS)

foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    list(APPEND PROTO_SRCS "${PROTO_GEN_DIR}/${PROTO_NAME}.pb.cc")
    list(APPEND PROTO_HDRS "${PROTO_GEN_DIR}/${PROTO_NAME}.pb.h")

    add_custom_command(
        OUTPUT "${PROTO_GEN_DIR}/${PROTO_NAME}.pb.cc" "${PROTO_GEN_DIR}/${PROTO_NAME}.pb.h"
        COMMAND protobuf::protoc
        ARGS --cpp_out=${PROTO_GEN_DIR} -I${PROTO_DIR} ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generando ${PROTO_NAME}.pb.cc/h"
    )
endforeach()

# Incluir directorios
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${PROTO_GEN_DIR})
include_directories(${Protobuf_INCLUDE_DIRS})

# Utils
set(UTILS_SRCS
    src/utils/tcp_handler.cpp
    src/utils/message_handler.cpp
)

# Servidor
add_executable(chat_server
    src/server/main.cpp
    src/server/server.cpp
    ${UTILS_SRCS}
    ${PROTO_SRCS}
)

# Cliente
add_executable(chat_client
    src/client/main.cpp
    src/client/client.cpp
    ${UTILS_SRCS}
    ${PROTO_SRCS}
)

# Enlazar bibliotecas
if(WIN32)
    target_link_libraries(chat_server ${Protobuf_LIBRARIES} ws2_32)
    target_link_libraries(chat_client ${Protobuf_LIBRARIES} ws2_32)
else()
    target_link_libraries(chat_server pthread ${Protobuf_LIBRARIES})
    target_link_libraries(chat_client pthread ${Protobuf_LIBRARIES})
endif()
