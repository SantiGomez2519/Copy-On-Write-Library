#include <iostream>
#include <fstream>
#include <vector>
#include "./include/file_manager.h"

int main() {
    std::string filename = "testfile";

    std::cout << "Intentando crear el archivo: " << filename << std::endl;
    if (!VersionedStorage::create(filename)) {
        std::cerr << "Error: No se pudo crear el archivo." << std::endl;
        return 1;
    }

    std::cout << "Intentando abrir el archivo: " << filename << std::endl;
    if (!VersionedStorage::open(filename)) {
        std::cerr << "Error: No se pudo abrir el archivo." << std::endl;
        return 1;
    }
    std::cout << "Archivo abierto exitosamente: " << filename << std::endl;

    std::vector<std::string> test_data = {
        "Primera version del archivo.",
        "Segunda version con cambios.",
        "Tercera version con mas cambios.",
        "Cuarta version con otro texto.",
        "Quinta version, final de la prueba."
    };

    for (size_t i = 0; i < test_data.size(); i++) {
        std::ofstream file(filename, std::ios::binary);
        file << test_data[i];
        file.close();

        std::cout << "Escribiendo version " << i << "..." << std::endl;
        if (!VersionedStorage::write(filename)) {
            std::cerr << "Error: No se pudo escribir la version " << i << "." << std::endl;
        }
    }

    std::cout << "Leyendo versiones en orden inverso..." << std::endl;
    for (int i = test_data.size() - 1; i >= 0; i--) {
        std::string read_data;
        if (VersionedStorage::read(filename, i, read_data)) {
            std::cout << "Version " << i << ": " << read_data << std::endl;
        } else {
            std::cerr << "Error: No se pudo leer la version " << i << "." << std::endl;
        }
    }

    std::cout << "Intentando leer una version invalida (99)..." << std::endl;
    std::string read_data;
    if (!VersionedStorage::read(filename, 99, read_data)) {
        std::cout << "Correcto: Se detecto que la version 99 no existe." << std::endl;
    } else {
        std::cerr << "Error: Se leyo incorrectamente una version inexistente." << std::endl;
    }

    std::cout << "Cerrando el archivo." << std::endl;
    if (!VersionedStorage::close(filename)) {
        std::cerr << "Error: No se pudo cerrar el archivo." << std::endl;
        return 1;
    }
    std::cout << "Archivo cerrado correctamente." << std::endl;
    VersionedStorage::mostrarEstadoDataFile(filename);
    return 0;
}