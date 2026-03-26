# Simple Chat Protocol

Aplicación de chat cliente-servidor en **C++** con **sockets TCP**, **multithreading** y **Protocol Buffers** para serializar los payloads. Proyecto de Sistemas Operativos (CC3064).

## Características

- Chat en tiempo real con varios usuarios
- Mensajes generales (broadcast) y mensajes directos (DM)
- Estados: **ACTIVO**, **OCUPADO**, **INACTIVO** (incl. inactividad por tiempo en servidor)
- Servidor multihilo: un hilo por cliente y lógica de sesión concurrente
- Protocolo con `.proto` y encabezado TCP de 5 bytes (ver `docs/protocol_standard.md`)
- Compilación con **CMake**; scripts `build.sh` (Linux/macOS) y `build.bat` (Windows)

## Requisitos previos

- **CMake** 3.10 o superior  
- **Compilador C++** con **C++17**  
- **Protocol Buffers**: `protoc` y bibliotecas de desarrollo  

La generación de código a partir de los `.proto` la hace **CMake** al compilar; no es necesario ejecutar `protoc` manualmente para el flujo normal.

## Inicio rápido (compilación)

Instala CMake, el toolchain C++ y Protobuf según tu sistema (véase [COMPILACION.md](COMPILACION.md)).

**Linux y macOS** (desde la raíz del repo):

```bash
chmod +x build.sh
./build.sh
```

**Windows:** con las mismas dependencias disponibles, ejecuta `build.bat` en la raíz del proyecto.

Ejecutables generados en **`build/`**: `chat_server`, `chat_client` (`.exe` en Windows).

## Ejecución

```bash
# Servidor: solo puerto
./build/chat_server 8080

# Cliente: usuario, IP del servidor, puerto
./build/chat_client alice 127.0.0.1 8080
```

Uso detallado del menú interactivo: [GUIA_USO.md](GUIA_USO.md).

## Estructura del proyecto

```
├── protos/                 # Definiciones Protocol Buffers (.proto)
├── src/
│   ├── server/             # Servidor (main.cpp, server.cpp, …)
│   ├── client/             # Cliente (main.cpp, client.cpp)
│   └── utils/              # tcp_handler, message_handler
├── include/                # Cabeceras (.h)
├── docs/
│   ├── protocol_standard.md
│   └── instructions.md
├── build/                  # Generado al compilar (no versionar)
├── CMakeLists.txt
├── build.sh                # Build Linux/macOS
├── build.bat               # Build Windows
├── COMPILACION.md
├── GUIA_USO.md
└── ARQUITECTURA.md
```

## Protocolo de comunicación

### Encabezado TCP (5 bytes)

| 1 byte | 4 bytes (big-endian) | N bytes        |
|--------|----------------------|----------------|
| Tipo   | Longitud del payload | Payload protobuf |

### Tipos de mensaje (resumen)

- **1–7**: cliente → servidor (registro, mensajes, estado, listado, info, salida, etc.)
- **10–14**: servidor → cliente (respuestas, lista, DM reenviado, broadcast, info de usuario)

Especificación completa: [docs/protocol_standard.md](docs/protocol_standard.md).

## Documentación

| Archivo | Contenido |
|---------|-----------|
| [COMPILACION.md](COMPILACION.md) | Dependencias por SO, `build.sh` / `build.bat`, solución de problemas |
| [GUIA_USO.md](GUIA_USO.md) | Menú del cliente, ejemplos, ejecución |
| [ARQUITECTURA.md](ARQUITECTURA.md) | Diseño y flujos |
| [docs/protocol_standard.md](docs/protocol_standard.md) | Framing TCP y tipos protobuf |
| [docs/instructions.md](docs/instructions.md) | Enunciado / requisitos del curso |

## Notas técnicas breves

- **Servidor:** hilo por conexión; datos de usuarios protegidos con mutex.  
- **Cliente:** hilo de recepción + hilo principal para la entrada del menú.  
- **Portabilidad:** `tcp_handler` abstrae Winsock (Windows) y sockets POSIX.

## Licencia

Proyecto académico — curso de Sistemas Operativos.

---

**Para compilar:** [COMPILACION.md](COMPILACION.md) · **Para usar:** [GUIA_USO.md](GUIA_USO.md)
