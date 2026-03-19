@echo off
setlocal enabledelayedexpansion

echo.
echo ========================================
echo Simple Chat Protocol - Windows Build
echo ========================================
echo.

if not exist "build" (
    echo [*] Creando directorio build...
    mkdir build
)

cd build

echo [*] Ejecutando CMake...
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..

if errorlevel 1 (
    echo [ERROR] CMake fallo
    goto :error
)

echo [*] Compilando...
cmake --build . --config Release

if errorlevel 1 (
    echo [ERROR] Compilacion fallo
    goto :error
)

echo.
echo ========================================
echo [OK] Compilacion completada
echo ========================================
echo.
echo Ejecutables en: build/
echo.
echo   Servidor: chat_server.exe ^<puerto^>
echo   Cliente:  chat_client.exe ^<usuario^> ^<IP_servidor^> ^<puerto^>
echo.
echo Ejemplo:
echo   Terminal 1: chat_server.exe 8080
echo   Terminal 2: chat_client.exe juan 192.168.1.10 8080
echo.

cd ..
exit /b 0

:error
echo.
echo [X] Error durante la compilacion
echo.
cd ..
exit /b 1
