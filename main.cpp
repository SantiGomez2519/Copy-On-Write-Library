#include "include/file_manager.h"
#include <iostream>

int main() {
    std::string filename = "testfile";

    // 1️⃣ Crear archivo
    if (!VersionedStorage::create(filename)) {
        std::cerr << "❌ Error al crear el archivo.\n";
        return 1;
    }

    // 2️⃣ Abrir archivo
    if (!VersionedStorage::open(filename)) {
        std::cerr << "❌ Error al abrir el archivo.\n";
        return 1;
    }

    // 3️⃣ Escribir datos en el archivo
    std::string data1 = "Primera versión del archivo.";
    if (!VersionedStorage::write(filename, data1)) {
        std::cerr << "❌ Error al escribir en el archivo.\n";
        return 1;
    }

    std::string data2 = "Segunda versión con más contenido.";
    if (!VersionedStorage::write(filename, data2)) {
        std::cerr << "❌ Error al escribir en el archivo.\n";
        return 1;
    }

    // 4️⃣ Leer las versiones
    std::string read_output;
    if (VersionedStorage::read(filename, 0, read_output)) {
        std::cout << "✅ Versión 0 leída: " << read_output << std::endl;
    } else {
        std::cerr << "❌ Error al leer la versión 0.\n";
    }

    if (VersionedStorage::read(filename, 1, read_output)) {
        std::cout << "✅ Versión 1 leída: " << read_output << std::endl;
    } else {
        std::cerr << "❌ Error al leer la versión 1.\n";
    }

    // 5️⃣ Cerrar archivo
    if (!VersionedStorage::close(filename)) {
        std::cerr << "❌ Error al cerrar el archivo.\n";
        return 1;
    }

    std::cout << "✅ Prueba completada con éxito.\n";
    return 0;
}

