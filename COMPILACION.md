# Instrucciones de Compilación

## Requisitos Previos

1. **CMake** 3.10 o superior
2. **Protobuf** compiler y librerías
3. **C++ compiler** (g++, clang, o MSVC)
4. **Git** (para control de versiones)

### Instalación de Requisitos

#### En Windows con vcpkg:
```bash
vcpkg install protobuf:x64-windows
```

#### En Windows con Chocolatey:
```bash
choco install cmake protoc
```

#### En Ubuntu/Debian:
```bash
sudo apt-get install cmake protobuf-compiler libprotobuf-dev
```

#### En macOS:
```bash
brew install cmake protobuf
```

## Pasos de Compilación

### 1. Crear directorio de build
```bash
mkdir build
cd build
```

### 2. Ejecutar CMake
```bash
# Windows (Visual Studio)
cmake -G "Visual Studio 16 2019" -A x64 ..

# O para Ninja
cmake -G Ninja ..

# O para Unix Makefiles (Linux/macOS)
cmake -G "Unix Makefiles" ..
```

### 3. Compilar
```bash
# Windows con Visual Studio
cmake --build . --config Release

# O con make
make

# O con ninja
ninja
```

Los ejecutables estarán en:
- `build/Release/nombe_del_servidor.exe` (Windows)
- `build/Release/nombe_del_cliente.exe` (Windows)
- `build/nombe_del_servidor` (Linux/macOS)
- `build/nombe_del_cliente` (Linux/macOS)

## Ejecución

### Iniciar el Servidor
```bash
./nombe_del_servidor [puerto]

# Ejemplo con puerto 8080
./nombe_del_servidor 8080
```

### Conectar Clientes
```bash
./nombe_del_cliente [host] [puerto]

# Ejemplo local
./nombe_del_cliente 127.0.0.1 8080

# Ejemplo remoto
./nombe_del_cliente 192.168.1.100 8080
```

## Solución de Problemas

### Error: "protoc: command not found"
- Instalar Protobuf compiler en tu sistema
- En Windows, asegurarse que sea accesible en PATH

### Error de CMake: "Protobuf_NOT_FOUND"
- En Windows con vcpkg: `cmake -DCMAKE_TOOLCHAIN_FILE=<vcpkg_path>/scripts/buildsystems/vcpkg.cmake ..`

### Error al compilar: undefined reference to protobuf symbols
- Asegurarse que libprotobuf esté correctamente vinculada
- En Windows con vcpkg: probablemente necesites usar la variable de herramientas vcpkg

## Estructura de Directorios

```
Simple-Chat-Protocol/
├── protos/          # Definiciones de Protocol Buffers
├── src/
│   ├── server/      # Código del servidor
│   ├── client/      # Código del cliente
│   └── utils/       # Utilidades compartidas
├── include/         # Headers del proyecto
├── build/           # Directorio de build (generado)
├── CMakeLists.txt   # Configuración de CMake
└── README.md        # Este archivo
```

## Notas Importantes

- Los archivos `.pb.h` y `.pb.cc` se generan automáticamente por CMake
- No deben añadirse al repositorio (están en .gitignore)
- El puerto por defecto es 8080, puede modificarse en tiempo de ejecución
- Para desarrollo, se recomienda compilar en modo Debug: `cmake -DCMAKE_BUILD_TYPE=Debug ..`
