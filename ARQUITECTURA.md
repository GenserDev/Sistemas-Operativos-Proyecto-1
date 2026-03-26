# Arquitectura del Proyecto

## Visión General

```
┌─────────────┐         TCP Sockets        ┌──────────────┐
│  Cliente 1  │ ─────────────────────────  │              │
│  (CLI)      │                            │   Servidor   │
└─────────────┘                            │   (Central)  │
                                           │              │
┌─────────────┐         TCP Sockets        │   Thread     │
│  Cliente N  │ ─────────────────────────  │   Pool       │
│  (CLI)      │                            │              │
└─────────────┘                            └──────────────┘

Protocolo: Protocol Buffers + TCP Framing (encabezado de 5 bytes)
```

## Componentes Principales

### 1. Servidor (`src/server/`)

#### Archivo: `include/server.h` / `src/server/server.cpp`
Clase `ChatServer` - Gestiona toda la lógica del servidor.

**Responsabilidades:**
- Iniciar socket TCP servidor en puerto especificado
- Aceptar conexiones de clientes
- Crear un thread por cliente
- Gestionar usuario conectados
- Encaminar mensajes (broadcast, DM)
- Sincronización con mutexes

**Métodos clave:**
```cpp
void run();                            // Loop principal: listen + accept + threads
bool register_user();                  // Registrar nuevo usuario
bool unregister_user();                // Remover usuario
void broadcast_message();              // Enviar a todos (excepto el emisor)
void send_dm();                       // Mensaje directo
void handle_client();                 // Manejar cliente (un thread por conexión)
```

**Estructura de datos:**
```cpp
std::map<std::string, User> users;     // username → datos usuario
std::map<int, std::string> socket_to_user;  // socket → username
std::mutex users_mutex;                // Sincronización de acceso
```

#### Archivo: `main.cpp`
Punto de entrada del servidor. Maneja argumentos de línea de comandos.

### 2. Cliente (`src/client/`)

#### Archivo: `include/client.h` / `src/client/client.cpp`
Clase `ChatClient` - Maneja la conexión y lógica de cliente.

**Responsabilidades:**
- Conectar al servidor
- Enviar mensajes con encabezado
- Recibir mensajes en thread paralelo
- Procesar entrada del usuario
- Mostrar interfaz CLI

**Métodos clave:**
```cpp
bool connect_to_server();              // Conectar al servidor
void run();                            // Loop principal
bool send_register();                  // Registrarse
bool send_broadcast();                 // Mensaje general
bool send_dm();                        // Mensaje directo
void receive_loop();                   // Thread de recepción
```

**Threading:**
- Thread principal: Interfaz CLI y entrada del usuario
- Thread secundario: Recepción continua de mensajes

#### Archivo: `main.cpp`
Punto de entrada del cliente. Argumentos: `<nombre_usuario> <IP_servidor> <puerto>`

### 3. Utilidades (utils/)

#### Archivo: `tcp_handler.h` / `tcp_handler.cpp`
Clase `TCPHandler` - Abstracción de sockets TCP.

**Responsabilidades:**
- Encapsular operaciones de socket POSIX y Winsock
- Facilitar portabilidad Windows/Linux
- Manejar ciclo de vida de sockets

**Métodos clave:**
```cpp
static int create_server();            // Servidor: bind + listen
static int accept_connection();        // Servidor: accept
static int connect_to();               // Cliente: connect
static bool send_all();                // Enviar buffer completo
static std::string receive_full_message();  // Un mensaje con framing 5 bytes
```

**Portabilidad:**
```cpp
#ifdef _WIN32
    // Código específico de Windows (Winsock)
#else
    // Código POSIX (Linux/macOS)
#endif
```

#### Archivo: `message_handler.h` / `message_handler.cpp`
Clase `MessageHandler` - Encapsulación de protocolo TCP.

**Responsabilidades:**
- Encapsular/desencapsular mensagens con encabezado
- Serializar/deserializar estructuras protobuf
- Validar formato de mensajes

**Estructura del encabezado:**
```
Byte 0:    uint8_t  - Tipo de mensaje (1-14)
Bytes 1-4: uint32_t - Longitud del payload (big-endian)
Bytes 5+:  ...     - Payload protobuf serializado
```

## Flujo de Comunicación

### Registro de Usuario
```
Cliente                              Servidor
  │                                    │
  ├─ Register (username, ip)           │
  ├─ Enviar REGISTER (tipo 1)          │
  ├────────────────────────────────────>
  │                                    ├─ Recibir, parsear, register_user()
  │                                    ├─ (El thread del cliente ya existe)
  │                    ServerResponse  │
  │ <─────────────────────────────────┤  (éxito o error, p. ej. nombre/IP duplicados)
  │                                    │
```

### Mensaje General (Broadcast)
```
Cliente A                           Servidor                    Cliente B
    │                                 │                            │
    ├─ Enviar MessageGeneral          │                            │
    │─────────────────────────────────>                            │
    │                                 ├─ Recibir y parsear        │
    │                                 ├─ Por cada usuario (excepto el emisor)
    │                                 ├─ Encapsular BroadcastDelivery
    │                                 ├─ Enviar a todos los demás
    │                                 ├───────────────────────────>
    │                                 │                            ├─ Recibir
    │                                 │                            ├─ Mostrar
    │                                 │                            │
```

### Mensaje Directo (DM)
```
Cliente A                           Servidor                    Cliente B
    │                                 │                            │
    ├─ Enviar MessageDM               │                            │
    │  (username_des = Bruno)         │                            │
    ├─────────────────────────────────>                            │
    │                                 ├─ Recibir y parsear        │
    │                                 ├─ Encontrar usuario Bruno  │
    │                                 ├─ Crear ForDm              │
    │                                 ├─ Enviar a Bruno           │
    │                                 ├───────────────────────────>
    │                                 │                            ├─ Recibir
    │                                 │                            ├─ Mostrar "[DM de A]"
    │                                 │                            │
```

## Tipos de Mensaje (Protocolo)

### Cliente → Servidor (1-7)
- **1 - REGISTER**: Registro inicial
- **2 - MESSAGE_GENERAL**: Mensaje para broadcast
- **3 - MESSAGE_DM**: Mensaje directo
- **4 - CHANGE_STATUS**: Cambiar estado de usuario
- **5 - LIST_USERS**: Solicitar lista de usuarios
- **6 - GET_USER_INFO**: Solicitar info de usuario
- **7 - QUIT**: Desconectar

### Servidor → Cliente (10-14)
- **10 - SERVER_RESPONSE**: Respuesta de operación
- **11 - ALL_USERS**: Lista de usuarios
- **12 - FOR_DM**: Mensaje directo recibido
- **13 - BROADCAST_MESSAGES**: Mensaje broadcast
- **14 - GET_USER_INFO_RESPONSE**: Info de usuario solicitado

## Sincronización y Concurrencia

### Problema: Acceso concurrente a datos
- N threads de cliente accediendo simultáneamente

### Solución: Mutexes
```cpp
std::mutex users_mutex;  // Protege mapa de usuarios y socket_to_user

std::lock_guard<std::mutex> lock(users_mutex);
// Acceso seguro a usuarios
```

### Deadlock Prevention
- Siempre adquirir locks en el mismo orden
- Locks de corta duración
- Evitar nested locks

## Estructura de Directorios

```
proyecto/
├── protos/                 # Todos los .proto (CMake/protoc genera .pb.cc/.pb.h en build)
│   ├── common.proto
│   ├── register.proto
│   ├── message_general.proto
│   ├── message_dm.proto
│   ├── change_status.proto
│   ├── list_users.proto
│   ├── get_user_info.proto
│   ├── quit.proto
│   ├── server_response.proto
│   ├── all_users.proto
│   ├── for_dm.proto
│   ├── broadcast_messages.proto
│   └── get_user_info_response.proto
├── src/
│   ├── server/
│   │   ├── main.cpp
│   │   ├── server.cpp
│   │   └── client_handler.cpp   # reservado / sin lógica activa
│   ├── client/
│   │   ├── main.cpp
│   │   └── client.cpp
│   └── utils/
│       ├── tcp_handler.cpp
│       └── message_handler.cpp
├── include/
├── docs/
├── build/                  # Generado: chat_server, chat_client
├── CMakeLists.txt
├── build.sh
├── build.bat
├── README.md
├── COMPILACION.md
├── GUIA_USO.md
└── ARQUITECTURA.md
```

## Compilación

Recomendado en Linux/macOS: `chmod +x build.sh` y `./build.sh`. En Windows: `build.bat`. CMake genera los fuentes protobuf y enlaza `libprotobuf`. Ver [COMPILACION.md](COMPILACION.md).

## Flujo de Ejecución

### Servidor
```
main()
  ├─ Parsear argumentos (puerto)
  ├─ Crear ChatServer
  ├─ Llamar server.run()
  │   ├─ Crear TCPHandler
  │   ├─ Escuchar en puerto
  │   └─ While running:
  │       ├─ accept_connection() → socket cliente
  │       └─ Lanzar thread: handle_client(cliente_socket)
  │           ├─ Receive mensaje REGISTER
  │           ├─ register_user()
  │           └─ While running:
  │               ├─ Recibir mensaje TCP
  │               ├─ Desencapsular (unwrap_message)
  │               ├─ Parsear protobuf según tipo
  │               ├─ Procesar (broadc., DM, etc.)
  │               └─ Responder si es necesario
  └─ Fin
```

### Cliente
```
main()
  ├─ Parsear argumentos (usuario, IP servidor, puerto)
  ├─ Crear ChatClient
  ├─ connect_to_server()
  └─ client.run()
      ├─ send_register()
      ├─ Lanzar receive_thread → receive_loop()
      │   └─ receive_full_message + unwrap + process_server_message()
      ├─ handle_user_input() (thread principal, menú 1–7)
      │   ├─ 1: send_broadcast()
      │   ├─ 2: send_dm()
      │   ├─ 3: change_status()
      │   ├─ 4: request_user_list()
      │   ├─ 5: request_user_info()
      │   ├─ 6: display_help()
      │   └─ 7: salir
      ├─ send_quit()
      └─ disconnect()
```

## Decisiones de Diseño

### ¿Por qué Protocol Buffers?
- ✓ Eficiente en tamaño y velocidad
- ✓ Versionable y extensible
- ✓ Soportado en múltiples lenguajes
- ✓ Requerimiento del proyecto

### ¿Por qué Encabezado de 5 bytes?
- 1 byte para tipo (0-255 tipos posibles)
- 4 bytes para longitud (hasta 4GB de payload)
- Necesario porque TCP es byte-stream (sin límites de mensaje)

### ¿Por qué Threading?
- Servidor: Manejar múltiples clientes simultáneamente
- Cliente: Recibir mensajes mientras usuario escribe

### ¿Por qué Mutex?
- Sincronizar acceso concurrente al mapa de usuarios
- Prevenir race conditions

## Limitaciones y Mejoras Futuras

### Limitaciones Actuales
- Sin autenticación de contraseñas
- Sin persistencia de datos
- Sin cifrado de mensajes
- Sin balance de carga
- Max conexiones = límite del SO

### Mejoras Posibles
- [ ] Base de datos (SQLite/PostgreSQL)
- [ ] Encriptación TLS/SSL
- [ ] Autenticación JWT
- [ ] Salas de chat
- [ ] Historial persistente
- [ ] Indicador de "escribiendo..."
- [ ] Confirmación de entrega
- [ ] Notificaciones push
