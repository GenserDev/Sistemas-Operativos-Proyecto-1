# Guía de uso — Simple Chat Protocol

## Descripción general

Aplicación de chat cliente-servidor en C++ con **sockets TCP**, **multithreading** y **Protocol Buffers**. Permite:

- Chat general (broadcast) entre usuarios conectados
- Mensajes directos (DM) privados
- Estados de usuario: **ACTIVO**, **OCUPADO**, **INACTIVO**
- Listado de usuarios e información (incluida IP) de un usuario concreto
- Ayuda y salida ordenada

## Compilación

Instala **CMake**, un **compilador C++17** y **Protobuf** según tu sistema operativo (ver [COMPILACION.md](COMPILACION.md)). Luego, en Linux o macOS:

```bash
chmod +x build.sh
./build.sh
```

En Windows, con las mismas dependencias configuradas, ejecuta `build.bat` desde la raíz del proyecto.

Los ejecutables quedan en **`build/chat_server`** y **`build/chat_client`**.

## Inicio rápido

### 1. Terminal — servidor

```bash
./build/chat_server 8080
```

Salida esperada (resumen):

```
=== SERVIDOR DE CHAT INICIADO ===
Puerto: 8080
Timeout inactividad: 60 segundos
```

### 2. Terminal — cliente 1

El **nombre de usuario** se pasa por línea de comandos (no se pide después):

```bash
./build/chat_client Alice 127.0.0.1 8080
```

### 3. Terminal — cliente 2

```bash
./build/chat_client Bob 127.0.0.1 8080
```

## Menú de opciones

Tras conectarse, el cliente muestra un menú numérico:

| Opción | Acción |
|--------|--------|
| **1** | Enviar mensaje al chat general (broadcast) |
| **2** | Enviar mensaje directo (privado) |
| **3** | Cambiar estado (ACTIVO / OCUPADO / INACTIVO) |
| **4** | Ver lista de usuarios conectados |
| **5** | Ver información de un usuario (IP, estado, etc.) |
| **6** | Ayuda (vuelve a mostrar el menú) |
| **7** | Salir (avisa al servidor y cierra) |

### Opción 1: chat general

Todos los demás clientes conectados reciben el mensaje (el emisor no lo recibe de vuelta por broadcast).

```
> 1
Mensaje: Hola a todos!
```

En pantalla del resto aparece una línea similar a: `[GENERAL] Alice: Hola a todos!`

### Opción 2: mensaje directo

```
> 2
Destinatario: Bob
Mensaje: Hola Bob
```

Bob verá algo como: `[DM de Alice]: Hola Bob`

### Opción 3: cambiar estado

```
> 3
  0 - ACTIVO
  1 - OCUPADO
  2 - INACTIVO
Nuevo estado: 1
```

El servidor confirma con un mensaje `[OK]` o `[ERROR]` según el caso.

### Opción 4: lista de usuarios

Muestra nombres y estado entre líneas tipo:

```
--- Usuarios conectados ---
  Alice [ACTIVO]
  Bob [OCUPADO]
---------------------------
```

### Opción 5: información de un usuario

```
> 5
Nombre del usuario: Bob
```

Muestra usuario, IP y estado si el usuario está conectado.

### Opción 6: ayuda

Vuelve a imprimir el menú de comandos.

### Opción 7: salir

Cierra la sesión en el servidor y termina el cliente.

## Mensajes que verás en pantalla

| Prefijo / formato | Significado |
|-------------------|-------------|
| `[OK] ...` | Operación correcta (respuesta del servidor) |
| `[ERROR] ...` | Error (por ejemplo nombre duplicado, usuario no encontrado) |
| `[GENERAL] usuario: texto` | Mensaje del chat general |
| `[DM de usuario]: texto` | Mensaje privado recibido |
| Bloques `--- Usuarios conectados ---` / `--- Info de usuario ---` | Respuestas a listado o consulta de usuario |

## Comportamiento útil

### Multithreading en el cliente

Un hilo recibe mensajes del servidor mientras escribes en el menú; por eso pueden aparecer líneas de chat entre tus `>`.

### Registro

El registro con el servidor ocurre **automáticamente** al iniciar, usando el nombre que pasaste en la línea de comandos.

### Inactividad

El servidor puede marcar un usuario como **INACTIVO** tras un periodo sin actividad (valor configurable en código; por defecto suele ser del orden de un minuto). Ver consola del servidor y mensajes `[OK]` en el cliente.

### Sin persistencia

No hay base de datos: al desconectar, no queda historial en el servidor.

### Encabezado TCP (5 bytes)

Cada mensaje lógico lleva: 1 byte de tipo, 4 bytes de longitud (big-endian) y el payload protobuf. Detalle en [docs/protocol_standard.md](docs/protocol_standard.md).

## Solución de problemas

### "No se pudo conectar al servidor"

- Comprueba que el servidor esté en ejecución.
- Revisa IP y puerto.
- En red local, revisa firewall y que la IP del servidor sea la correcta.

### "Nombre de usuario o IP ya registrados"

El servidor no permite dos usuarios con el mismo nombre ni el mismo IP registrado a la vez (según la lógica del proyecto).

### No ves mensajes

- Comprueba que sigas conectado.
- Para DM, el destinatario debe existir en el servidor.

## Ejemplos rápidos

**Dos usuarios en chat general**

1. Servidor: `./build/chat_server 8080`
2. Cliente A: `./build/chat_client Alice 127.0.0.1 8080` → opción `1` y un mensaje.
3. Cliente B: `./build/chat_client Bob 127.0.0.1 8080` → B recibe el broadcast como `[GENERAL] Alice: ...`

**Mensaje directo**

1. Alice opción `2`, destinatario `Bob`, mensaje `Hola`.
2. Bob ve `[DM de Alice]: Hola`.

---

Más detalles de compilación: [COMPILACION.md](COMPILACION.md).  
Protocolo en red: [docs/protocol_standard.md](docs/protocol_standard.md).
