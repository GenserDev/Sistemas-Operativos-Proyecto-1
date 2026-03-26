# Instrucciones de compilación

## Requisitos previos

| Herramienta | Uso |
|---------------|-----|
| **CMake** | 3.10 o superior |
| **Compilador C++** | Con soporte **C++17** (por ejemplo `g++`, `clang++`) |
| **Protocol Buffers** | `protoc` y biblioteca de desarrollo (`libprotobuf`) |

No hace falta invocar `protoc` a mano: **CMake** genera los `.pb.cc` / `.pb.h` al compilar.

## Instalación de dependencias (según el sistema)

### Linux (Debian/Ubuntu y similares)

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake protobuf-compiler libprotobuf-dev
```

### macOS (Homebrew)

```bash
brew install cmake protobuf
```

### Windows

Instala herramientas equivalentes, por ejemplo:

- **CMake** y un entorno con compilador C++ (Visual Studio Build Tools, o MinGW), y  
- **Protobuf** (compilador + librerías), según tu entorno (vcpkg, Chocolatey, instalador oficial, etc.).

Luego usa el script `build.bat` desde la raíz del proyecto (ver abajo).

## Compilación recomendada

### Linux y macOS

Desde la **raíz del repositorio**:

```bash
chmod +x build.sh
./build.sh
```

El script crea `build/`, ejecuta CMake con generadores tipo Unix Makefiles y compila con `make -j`.

### Windows

Desde la raíz del proyecto:

```cmd
build.bat
```

(Tras tener CMake, C++ y Protobuf disponibles en el PATH o configurados como indique tu instalación.)

## Salida

Los ejecutables quedan en el directorio **`build/`**:

| Ejecutable | Descripción |
|------------|-------------|
| `chat_server` | Servidor de chat (`chat_server.exe` en Windows) |
| `chat_client` | Cliente de chat (`chat_client.exe` en Windows) |

## Ejecución

### Servidor

```bash
./build/chat_server <puerto>
```

Ejemplo:

```bash
./build/chat_server 8080
```

### Cliente

```bash
./build/chat_client <nombre_usuario> <IP_servidor> <puerto>
```

Ejemplos:

```bash
./build/chat_client alice 127.0.0.1 8080
./build/chat_client bob 192.168.1.10 8080
```

## Compilación manual (opcional)

Si prefieres no usar los scripts:

```bash
mkdir -p build && cd build
cmake -G "Unix Makefiles" ..
cmake --build .
```

En Windows puedes abrir la carpeta `build` en Visual Studio o usar `cmake --build .` según el generador que elijas.

## Solución de problemas

### `protoc: command not found`

Instala el compilador de Protocol Buffers y asegúrate de que esté en el `PATH`.

### CMake: `Protobuf_NOT_FOUND` o errores al enlazar

- Comprueba que existan tanto **`protoc`** como las **bibliotecas de desarrollo** de protobuf.  
- En algunas instalaciones de protobuf reciente hace falta también **Abseil**; CMake del proyecto intenta enlazarlo si `find_package(absl)` lo encuentra.

### El script falla en `cmake` o `make`

Verifica versiones: CMake ≥ 3.10, compilador con C++17, y que `cmake` y el compilador estén en el `PATH`.

## Notas

- El directorio **`build/`** suele ignorarse en Git; se genera al compilar.  
- Los archivos generados por protobuf bajo `build/` no deben editarse a mano.
