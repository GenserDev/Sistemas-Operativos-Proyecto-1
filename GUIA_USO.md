# Guía de Uso - Simple Chat Protocol

## Descripción General

Simple Chat Protocol es una aplicación de chat cliente-servidor en C++ que utiliza Protocol Buffers para la comunicación. Permite:

- ✓ Chat general entre múltiples usuarios
- ✓ Mensajes directos (DM) privados
- ✓ Gestión de estado de usuario (Activo, No molestar, Invisible)
- ✓ Solicitud de información de otros usuarios
- ✓ Multithreading para conexiones simultáneas

## Inicio Rápido

### 1. Terminal Servidor
```bash
cd build
./nombe_del_servidor 8080
```

Salida esperada:
```
=== SERVIDOR DE CHAT INICIADO ===
Puerto: 8080
```

### 2. Terminal Cliente 1
```bash
cd build
./nombe_del_cliente 127.0.0.1 8080
=== CLIENTE DE CHAT ===
Ingrese su nombre de usuario: Alice
```

### 3. Terminal Cliente 2 (en otra ventana)
```bash
cd build
./nombe_del_cliente 127.0.0.1 8080
=== CLIENTE DE CHAT ===
Ingrese su nombre de usuario: Bob
```

## Menú de Opciones

Una vez conectado, cada cliente vera el menú:

```
=== MENÚ ===
1. Enviar mensaje al chat general
2. Enviar mensaje directo
3. Ver lista de usuarios
4. Ver información de usuario
5. Cambiar estado
6. Salir
```

### Opción 1: Chat General
Todos los usuarios activos (no invisibles) reciben el mensaje.

```
Opción: 1
Mensaje: Hola a todos!
```

### Opción 2: Mensaje Directo (DM)
Envía un mensaje privado a un usuario específico.

```
Opción: 2
Destinatario: Bob
Mensaje: Hola Bob, ¿cómo estás?
```

### Opción 3: Ver Lista de Usuarios
Muestra todos los usuarios conectados y sus estados.

```
Opción: 3

[USUARIOS CONECTADOS]
  - Alice (ACTIVO)
  - Bob (ACTIVO)
  - Charlie (NO MOLESTAR)
```

### Opción 4: Ver Información de Usuario
Obtiene detalles sobre un usuario específico (IP, estado).

```
Opción: 4
Usuario a consultar: Bob

[INFO DE USUARIO]
  Usuario: Bob
  IP: 192.168.1.50
  Estado: ACTIVO
```

### Opción 5: Cambiar Estado
Modifica tu estado de disponibilidad.

```
Opción: 5

0. ACTIVO
1. NO MOLESTAR
2. INVISIBLE
Nuevo estado: 1
```

**Estados:**
- **0 - ACTIVO**: Disponible, recibe todos los mensajes
- **1 - NO MOLESTAR**: Disponible pero no desea interrupciones
- **2 - INVISIBLE**: Conectado pero aparece como desconectado

### Opción 6: Salir
Desconecta del servidor limpiamente.

```
Opción: 6
Desconectado del servidor
```

## Tipos de Mensajes del Sistema

### [SERVIDOR]
Respuestas del servidor a solicitudes.

```
[SERVIDOR] Registro exitoso
[✓] Exitoso
```

### [DM de usuario]
Mensaje directo recibido.

```
[DM de Alice]: Hola, ¿recibiste mi mensaje?
```

### [CHAT GENERAL]
Mensaje del chat general.

```
[CHAT GENERAL] Alice: ¿Alguien quiere jugar?
```

### [INFO DE USUARIO]
Información solicitada sobre otro usuario.

```
[INFO DE USUARIO]
  Usuario: Charlie
  IP: 192.168.1.60
  Estado: NO MOLESTAR
```

### [USUARIOS CONECTADOS]
Lista de usuarios activos.

```
[USUARIOS CONECTADOS]
  - Alice (ACTIVO)
  - Bob (ACTIVO)
  - Charlie (NO MOLESTAR)
```

## Notas de Comportamiento

### Usuarios Invisibles
- No aparecen en la lista de usuarios
- Pueden enviar mensajes
- Reciben DMs pero no broadcast

### Multithreading
- El servidor maneja cada cliente en un thread separado
- Múltiples clientes pueden conectarse simultáneamente
- Acceso concurrente a datos está sincronizado con mutexes

### Persistencia
- Los usuarios se registran solo mientras están conectados
- Al desconectar, los datos se pierde
- No hay base de datos persistente

### Encabezado TCP (5 bytes)
```
Byte 0: Tipo de mensaje (1-14)
Bytes 1-4: Longitud del payload (big-endian)
```

## Solución de Problemas

### "Error: No se pudo conectar al servidor"
- Verificar que el servidor esté ejecutándose
- Verificar IP y puerto correctos
- Si es remoto, verificar cortafuegos

### "Error: Primero debe registrarse"
- Algunos comandos requieren estar registrado
- El registro ocurre automáticamente al iniciar

### Usuario no recibe mensajes
- Si está INVISIBLE, está configurado así
- Si el remitente es él mismo, se filtra
- Verificar que esté conectado

### Desconexiones inesperadas
- Verificar conexión de red
- Revisar logs del servidor
- Intentar reconectar

## Ejemplos de Uso

### Ejemplo 1: Chat Simple
```
1. Dos usuarios conectan
2. Alice: Opción 1 → "Hola Bob"
3. Bob recibe: [CHAT GENERAL] Alice: Hola Bob
4. Bob: Opción 1 → "Hola Alice"
5. Alice recibe: [CHAT GENERAL] Bob: Hola Alice
```

### Ejemplo 2: Mensaje Directo
```
1. Opción 2
2. Destinatario: Charlie
3. Mensaje: Saludo privado
4. Charlie recibe: [DM de Alice]: Saludo privado
```

### Ejemplo 3: Cambiar Disponibilidad
```
1. Alice: Opción 5 → Estado 1 (NO MOLESTAR)
2. Otros usuarios ven que Alice está "NO MOLESTAR"
3. Alice sigue recibiendo DMs
4. Otros todavía pueden ver a Alice en lista
```

## Limitaciones Conocidas

- No hay historial de mensajes
- Los nombres de usuario no pueden contener cambios
- No hay autenticación de contraseñas
- Los mensajes no se almacenan
- Máximo de conexiones limitado por sistema operativo

## Características Futuras

- [ ] Base de datos persistente
- [ ] Autenticación de usuarios
- [ ] Cifrado de mensajes
- [ ] Salas de chat personalizadas
- [ ] Historial de mensajes
- [ ] Archivos compartidos
- [ ] Audio/Video
