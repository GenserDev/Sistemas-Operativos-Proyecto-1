#!/bin/bash

echo ""
echo "========================================"
echo "Simple Chat Protocol - Build (Linux/macOS)"
echo "========================================"
echo ""

# Detectar sistema operativo
OS="$(uname -s)"
case "$OS" in
    Linux*)     PLATFORM="Linux";;
    Darwin*)    PLATFORM="macOS";;
    *)          PLATFORM="Unknown";;
esac

echo "[*] Plataforma detectada: $PLATFORM"
echo ""

# Verificar dependencias
if ! command -v cmake &> /dev/null; then
    echo "[ERROR] CMake no esta instalado"
    if [ "$PLATFORM" = "macOS" ]; then
        echo "Instala con: brew install cmake"
    else
        echo "Instala con: sudo apt-get install cmake"
    fi
    exit 1
fi

if ! command -v protoc &> /dev/null; then
    echo "[ERROR] protoc no esta instalado"
    if [ "$PLATFORM" = "macOS" ]; then
        echo "Instala con: brew install protobuf"
    else
        echo "Instala con: sudo apt-get install protobuf-compiler libprotobuf-dev"
    fi
    exit 1
fi

# Detectar numero de cores
if [ "$PLATFORM" = "macOS" ]; then
    NUM_CORES=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
else
    NUM_CORES=$(nproc 2>/dev/null || echo 4)
fi

# Crear directorio build
mkdir -p build
cd build || exit 1

echo "[*] Ejecutando CMake..."
cmake -G "Unix Makefiles" ..

if [ $? -ne 0 ]; then
    echo "[ERROR] CMake fallo"
    cd ..
    exit 1
fi

echo "[*] Compilando con $NUM_CORES cores..."
make -j"$NUM_CORES"

if [ $? -ne 0 ]; then
    echo "[ERROR] Compilacion fallo"
    cd ..
    exit 1
fi

echo ""
echo "========================================"
echo "[OK] Compilacion completada ($PLATFORM)"
echo "========================================"
echo ""
echo "Ejecutables en: build/"
echo ""
echo "  Servidor: ./build/chat_server <puerto>"
echo "  Cliente:  ./build/chat_client <usuario> <IP_servidor> <puerto>"
echo ""
echo "Ejemplo:"
echo "  Terminal 1: ./build/chat_server 8080"
echo "  Terminal 2: ./build/chat_client juan 192.168.1.10 8080"
echo ""

cd ..
exit 0
