#!/bin/bash

echo ""
echo "========================================"
echo "Simple Chat Protocol - Build (Linux/macOS)"
echo "========================================"
echo ""

# Verificar dependencias
if ! command -v cmake &> /dev/null; then
    echo "[ERROR] CMake no esta instalado"
    echo "Instala con: sudo apt-get install cmake"
    exit 1
fi

if ! command -v protoc &> /dev/null; then
    echo "[ERROR] protoc no esta instalado"
    echo "Instala con: sudo apt-get install protobuf-compiler libprotobuf-dev"
    exit 1
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

echo "[*] Compilando..."
make -j$(nproc 2>/dev/null || echo 4)

if [ $? -ne 0 ]; then
    echo "[ERROR] Compilacion fallo"
    cd ..
    exit 1
fi

echo ""
echo "========================================"
echo "[OK] Compilacion completada"
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
