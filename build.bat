@echo off
REM Script de compilación para Simple Chat Protocol (Windows)

setlocal enabledelayedexpansion

echo.
echo ========================================
echo Simple Chat Protocol - Windows Build
echo ========================================
echo.

REM Verificar si existe el directorio build
if not exist "build" (
    echo [*] Creando directorio build...
    mkdir build
)

cd build

echo [*] Ejecutando CMake...
cmake -G "Visual Studio 16 2019" -A x64 ..

if errorlevel 1 (
    echo [ERROR] CMake falló
    goto :error
)

echo [*] Compilando...
cmake --build . --config Release

if errorlevel 1 (
    echo [ERROR] Compilación falló
    goto :error
)

echo.
echo ========================================
echo [✓] Compilación completada con éxito
echo ========================================
echo.
echo Ejecutables generados en: build/Release/
echo.
echo Para ejecutar:
echo   Servidor: .\Release\nombe_del_servidor.exe [puerto]
echo   Cliente:  .\Release\nombe_del_cliente.exe [host] [puerto]
echo.
echo Ejemplo:
echo   Terminal 1: .\Release\nombe_del_servidor.exe 8080
echo   Terminal 2: .\Release\nombe_del_cliente.exe 127.0.0.1 8080
echo.

cd ..
exit /b 0

:error
echo.
echo [✗] Error durante la compilación
echo.
cd ..
exit /b 1
