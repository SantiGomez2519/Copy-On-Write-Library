# 📂 Versioning Library - Sistema de Versionado en C++

![Status](https://img.shields.io/badge/status-in%20development-yellow)  
![Language](https://img.shields.io/badge/language-C++-blue)  
![License](https://img.shields.io/badge/license-MIT-green)

Esta es una **biblioteca en C++** para gestionar versiones de archivos utilizando un sistema de versionado similar a Git. Actualmente, la biblioteca permite **crear, abrir, escribir, leer y cerrar archivos**, almacenándolos en la carpeta `storage/`.

## 🚀 Características Actuales
✅ **Manejo de archivos:** Crear, abrir, leer, escribir y cerrar archivos.  
✅ **Sistema de almacenamiento:** Todos los archivos se guardan en `storage/`.  
✅ **Pruebas unitarias:** Se han implementado tests para validar las funciones básicas.  
✅ **Automatización:** Compilación y gestión del proyecto con `Makefile`.  

## 📌 Cómo Usar la Biblioteca

### 1️⃣ **Compilar la Biblioteca****
Ejecuta el siguiente comando para compilar la biblioteca y los archivos de prueba:
```bash
make
```
### 2️⃣ **Ejecutar Pruebas**  
Después de compilar, puedes probar las funciones con:
```bash
./test_functions
```
### 3️⃣ **Uso en Código**  
Ejemplo de uso en C++:
```cpp
#include "Versioning.h"

int main() {
    Versioning v;

    v.create("mi_archivo.txt");  // Crear un archivo
    v.open("mi_archivo.txt");    // Abrir el archivo
    v.write("Hola, este es un mensaje!");  // Escribir en el archivo
    v.read();                    // Leer el contenido
    v.close();                   // Cerrar el archivo

    return 0;
}
```
## 📅 Próximas Tareas (To-Do)
🔹 **Implementar el sistema de versiones completo** (guardar múltiples versiones de archivos).  
🔹 **Agregar la función `commit()`** para almacenar nuevas versiones.  
🔹 **Desarrollar `listVersions()`** para mostrar el historial de versiones.  
🔹 **Implementar `checkout(version)`** para restaurar versiones anteriores.  
🔹 **Optimizar para guardar solo los cambios entre versiones en lugar de archivos completos.**  

