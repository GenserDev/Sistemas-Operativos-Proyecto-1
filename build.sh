#!/bin/bash

# Script de compilación para Simple Chat Protocol (Linux/macOS)

echo ""
echo "========================================"
echo "Simple Chat Protocol - Build"
echo "========================================"
echo ""

# Verificar si existen las dependencias
if ! command -v cmake &> /dev/null; then
    echo "[ERROR] CMake no está instalado"
    echo "Instala con: sudo apt-get install cmake (Ubuntu) o brew install cmake (macOS)"
    exit 1
fi

if ! command -v protoc &> /dev/null; then
    echo "[ERROR] protoc no está instalado"
    echo "Instala con: sudo apt-get install protobuf-compiler (Ubuntu) o brew install protobuf (macOS)"
    exit 1
fi

# Crear directorio build si no existe
if [ ! -d "build" ]; then
    echo "[*] Creando directorio build..."
    mkdir build
fi

cd build || exit 1

echo "[*] Ejecutando CMake..."
cmake -G "Unix Makefiles" ..

if [ $? -ne 0 ]; then
    echo "[ERROR] CMake falló"
    cd ..
    exit 1
fi

echo "[*] Compilando..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "[ERROR] Compilación falló"
    cd ..
    exit 1
fi

echo ""
echo "========================================"
echo "[✓] Compilación completada con éxito"
echo "========================================"
echo ""
echo "Ejecutables generados en: build/"
echo ""
echo "Para ejecutar:"
echo "  Servidor: ./nombe_del_servidor [puerto]"
echo "  Cliente:  ./nombe_del_cliente [host] [puerto]"
echo ""
echo "Ejemplo:"
echo "  Terminal 1: ./nombe_del_servidor 8080"
echo "  Terminal 2: ./nombe_del_cliente 127.0.0.1 8080"
echo ""

cd ..
exit 0
