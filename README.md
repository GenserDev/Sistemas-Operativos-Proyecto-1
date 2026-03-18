# Simple Chat Protocol

Aplicación de chat cliente-servidor en **C++** que utiliza **sockets TCP**, **multithreading** y **Protocol Buffers** para la serialización de mensajes. Proyecto del curso de Sistemas Operativos.

## ✨ Características

- ✅ Chat en tiempo real con múltiples usuarios
- ✅ Mensajes generales (broadcast) 
- ✅ Mensajes directos (DM) privados
- ✅ Gestión de estados (Activo, No molestar, Invisible)
- ✅ Servidor multithreaded que maneja múltiples clientes simultáneamente
- ✅ Protocolo eficiente con Protocol Buffers
- ✅ Compatible con Windows y Linux/macOS

## 📋 Requisitos Previos

- **CMake** 3.10+
- **Protobuf compiler** y librerías
- **C++ compiler** (g++, clang, MSVC)
- **Python 3.8+** (para scripting adicional)

## 🚀 Inicio Rápido

### 1. Compilación

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

Ver [COMPILACION.md](COMPILACION.md) para instrucciones detalladas.

### 2. Ejecutar el Servidor

```bash
./nombe_del_servidor 8080
```

### 3. Ejecutar Clientes (en otras terminales)

```bash
./nombe_del_cliente 127.0.0.1 8080
```

Ver [GUIA_USO.md](GUIA_USO.md) para instrucciones completas.

## 📁 Estructura del Proyecto

```
Simple-Chat-Protocol/
├── protos/                      # Definiciones Protocol Buffers
│   ├── common.proto             # StatusEnum compartido
│   ├── cliente-side/            # Mensajes: Cliente → Servidor (tipos 1-7)
│   │   ├── register.proto
│   │   ├── message_general.proto
│   │   ├── message_dm.proto
│   │   ├── change_status.proto
│   │   ├── list_users.proto
│   │   ├── get_user_info.proto
│   │   └── quit.proto
│   └── server-side/             # Mensajes: Servidor → Cliente (tipos 10-14)
│       ├── all_users.proto
│       ├── for_dm.proto
│       ├── broadcast_messages.proto
│       ├── get_user_info_response.proto
│       └── server_response.proto
│
├── src/                         # Código fuente
│   ├── server/                  # Servidor
│   │   ├── main.cpp
│   │   ├── server.cpp
│   │   └── client_handler.cpp
│   ├── client/                  # Cliente
│   │   ├── main.cpp
│   │   └── client.cpp
│   └── utils/                   # Utilidades compartidas
│       ├── tcp_handler.cpp
│       └── message_handler.cpp
│
├── include/                     # Headers
│   ├── tcp_handler.h            # Abstracción de sockets
│   ├── message_handler.h        # Protocolo TCP
│   ├── server.h                 # Clase servidor
│   └── client.h                 # Clase cliente
│
├── build/                       # Directorio de compilación
├── docs/                        # Documentación del proyecto
│   ├── instructions.md          # Requisitos técnicos
│   └── protocol_standard.md     # Especificación del protocolo
│
├── CMakeLists.txt               # Configuración de CMake
├── .gitignore
├── README.md                    # Este archivo
├── COMPILACION.md               # Guía de compilación
├── GUIA_USO.md                 # Guía de usuario
└── ARQUITECTURA.md             # Descripción arquitectónica
```

## 📡 Protocolo de Comunicación

### Encabezado TCP (5 bytes)
```
┌─────────┬──────────────────────────────┬─────────────────┐
│ 1 byte  │ 4 bytes (big-endian)         │ N bytes         │
│ Type    │ Payload Length               │ Protobuf        │
└─────────┴──────────────────────────────┴─────────────────┘
```

### Tipos de Mensaje
- **1-7**: Cliente → Servidor (registro, mensajes, cambios)
- **10-14**: Servidor → Cliente (respuestas, broadcasts, DMs)

Ver [docs/protocol_standard.md](docs/protocol_standard.md) para detalles completos.

## 🔧 Características Técnicas

### Servidor
- **Multithreading**: Un thread por cliente conectado
- **Thread-safe**: Sincronización con mutexes
- **Gestión de usuarios**: Registro, desconexión automática
- **Enrutamiento**: Broadcast y mensajes directos

### Cliente  
- **Threading**: Entrada del usuario + recepción simultánea
- **CLI Interactivo**: Menú con 6 opciones
- **Manejo de estado**: Cambio dinámico de disponibilidad

### Utilidades
- **TCPHandler**: Abstracción de sockets multiplataforma
- **MessageHandler**: Encapsulación de protocolo TCP
- **Portabilidad**: Soporta Windows (Winsock) y POSIX

## 📝 Ejemplos

### Chat General
```
Alice: Opción 1
Mensaje: ¡Hola a todos!

Bob recibe: [CHAT GENERAL] Alice: ¡Hola a todos!
```

### Mensaje Directo
```
Alice: Opción 2
Destinatario: Bob
Mensaje: ¿Cómo estás?

Bob recibe: [DM de Alice]: ¿Cómo estás?
```

### Cambiar Estado
```
Alice: Opción 5
0. ACTIVO
1. NO MOLESTAR
2. INVISIBLE
Nuevo estado: 1

Lista de usuarios actualizada para todos
```

## 🧵 Concurrencia y Sincronización

- **Servidor**: Maneja N clientes con threading
- **Mutexes**: Protegen el mapa de usuarios
- **Atomicidad**: Operaciones de registro/desconexión seguras
- **Prevención de deadlock**: Orden consistente de locks

## 📚 Documentación

- [COMPILACION.md](COMPILACION.md) - Guía paso a paso de compilación
- [GUIA_USO.md](GUIA_USO.md) - Instrucciones de uso y ejemplos
- [ARQUITECTURA.md](ARQUITECTURA.md) - Diseño técnico y decisiones
- [docs/protocol_standard.md](docs/protocol_standard.md) - Especificación del protocolo
- [docs/instructions.md](docs/instructions.md) - Requisitos del proyecto

## ⚙️ Configuración

### Puerto por defecto
- Servidor: `8080`
- Personalizable: `./nombe_del_servidor <puerto>`

### Conexión remota
```bash
./nombe_del_cliente <ip_servidor> <puerto>
```

## 🐛 Solución de Problemas

| Problema | Solución |
|----------|----------|
| "Connection refused" | Verificar que servidor esté corriendo |
| "Protobuf not found" | Instalar libprotobuf-dev |
| Usuario ve invisibles | Usuario tiene estado INVISIBLE |
| No recibe mensajes | Verificar conexión de red |

Ver [GUIA_USO.md](GUIA_USO.md) para más soluciones.

## 🎯 Objetivos del Proyecto

✅ Aplicar conceptos de sistemas operativos:
- Multithreading y sincronización
- Programación de sockets
- Protocolos de red
- Manejo de concurrencia
- Comunicación inter-proceso

## 📜 Estándares

- **Protocolo**: Protocol Buffers v3
- **Encabezado**: TCP framing personalizado (5 bytes)
- **Codificación**: UTF-8 para strings
- **Numeración**: Big-endian para longitudes

## 🚧 Limitaciones Conocidas

- No hay persistencia de datos
- Sin autenticación de contraseña
- Sin cifrado de mensajes
- Máximo de conexiones = límite del SO

## 💡 Mejoras Futuras

- [ ] Base de datos persistente
- [ ] Autenticación y autorización
- [ ] TLS/SSL encryption
- [ ] Salas de chat personalizadas
- [ ] Historial de mensajes
- [ ] Compartición de archivos

## 👥 Autor

Proyecto de curso - Sistemas Operativos

## 📄 Licencia

Este proyecto es parte de un curso académico.

---

**Para empezar:** Ver [COMPILACION.md](COMPILACION.md) y [GUIA_USO.md](GUIA_USO.md)

## Dependencias

- **protoc** (Protocol Buffers compiler) — se necesita para generar los archivos `.pb.h` y `.pb.cc` a partir de los `.proto`.

## Compilar los protos

Desde la raíz del proyecto:

```bash
# Cliente
protoc -I protos/ --cpp_out=. protos/common.proto protos/cliente-side/*.proto

# Servidor
protoc -I protos/ --cpp_out=. protos/common.proto protos/server-side/*.proto
```

Los archivos generados (`.pb.h` / `.pb.cc`) están en `.gitignore` y no se incluyen en el repositorio.

## Protocolo

### Tipo compartido

| Enum `StatusEnum` | Valor |
|---|---|
| ACTIVE | 0 |
| DO_NOT_DISTURB | 1 |
| INVISIBLE | 2 |

### Cliente → Servidor

Todos los mensajes incluyen el campo `ip` del cliente.

| Proto | Descripción | Campos |
|---|---|---|
| `register` | Registro de usuario | username, ip |
| `message_general` | Mensaje al chat general | message, status, username_origin, ip |
| `message_dm` | Mensaje directo | message, status, username_des, ip |
| `change_status` | Cambio de status | status, username, ip |
| `list_users` | Solicitar lista de usuarios | username, ip |
| `get_user_info` | Solicitar info de un usuario | username_des, username, ip |
| `quit` | Desconectarse | quit, ip |

### Servidor → Cliente

| Proto | Descripción | Campos |
|---|---|---|
| `all_users` | Lista de usuarios conectados | usernames[], status[] |
| `for_dm` | DM reenviado al destinatario | username_des, message |
| `broadcast_messages` | Mensaje del chat general | message, username_origin |
| `get_user_info_response` | Respuesta con info de usuario | ip_address, username, status |
| `server_response` | Respuesta general del servidor | status_code, message |

## Ejecución

```bash
# Servidor
./<nombredelservidor> <puertodelservidor>

# Cliente
./<nombredelcliente> <nombredeusuario> <IPdelservidor> <puertodelservidor>
```
