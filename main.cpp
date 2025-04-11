#include <iostream>
#include <fstream>
#include <vector>
#include "./include/file_manager.h"

int main() {
    std::string filename = "testfile";

    // Crear y abrir el archivo
    std::cout << "Intentando crear el archivo: " << filename << "...\n";
    if (!VersionedStorage::create(filename)) {
        std::cerr << "Error: No se pudo crear el archivo.\n";
        return 1;
    }

    std::cout << "Intentando abrir el archivo: " << filename << "...\n";
    if (!VersionedStorage::open(filename)) {
        std::cerr << "Error: No se pudo abrir el archivo.\n";
        return 1;
    }
    std::cout << "Archivo abierto exitosamente: " << filename << "\n\n";
    VersionedStorage::showMemoryUsage(filename);

    // Datos de prueba
    std::vector<std::string> test_data = {
        "Primera version del archivo.",
        "Segunda version con cambios.",
        "Tercera version con mas cambios.",
        "Cuarta version con otro texto.",
        "Quinta version, final de la prueba.",
        "Sexta version, final de la prueba con GC correjido."
    };

    // Escribir datos de prueba
    std::cout << "Escribiendo datos de prueba...\n";
    for (size_t i = 0; i < test_data.size(); i++) {
        std::ofstream file(filename, std::ios::binary);
        file << test_data[i];
        file.close();

        if (!VersionedStorage::write(filename, 1)) {
            std::cerr << "Error: No se pudo escribir la version " << i << ".\n";
        } else {
            std::cout << "Version " << i << " escrita correctamente.\n";
        }
    }
    std::cout << "\n";

    // Leer versiones en orden inverso
    std::cout << "Leyendo versiones en orden inverso...\n";
    for (int i = test_data.size() - 2; i >= 0; i--) {
        std::string read_data;
        if (VersionedStorage::read(filename, i, read_data)) {
            std::cout << "Version " << i << ": " << read_data << "\n";
        } else {
            std::cerr << "Error: No se pudo leer la version " << i << ".\n";
        }
    }
    std::cout << "\n";

    // Intentar leer una versión inválida
    std::cout << "Intentando leer una version invalida (99)...\n";
    std::string read_data;
    if (!VersionedStorage::read(filename, 99, read_data)) {
        std::cout << "Correcto: Se detecto que la version 99 no existe.\n";
    } else {
        std::cerr << "Error: Se leyo incorrectamente una version inexistente.\n";
    }
    std::cout << "\n";

    // Leer la última versión
    std::cout << "Leyendo ultima version...\n";
    std::string latest;
    if (VersionedStorage::readLatestVersion(filename, latest)) {
        std::cout << "Contenido de la ultima version: " << latest << "\n";
    } else {
        std::cerr << "Error: No se pudo leer la ultima version.\n";
    }
    std::cout << "\n";

    // Recuperar una versión específica
    std::cout << "Recuperando una version especifica (2)...\n";
    std::string recovered;
    int version_to_recover = 2;
    if (VersionedStorage::read(filename, version_to_recover, recovered)) {
        std::cout << "Recuperada version " << version_to_recover << ": " << recovered << "\n";
    } else {
        std::cout << "No se pudo recuperar la version " << version_to_recover << "\n";
    }
    std::cout << "\n";

    // Cerrar el archivo
    std::cout << "Cerrando el archivo...\n";
    if (!VersionedStorage::close(filename)) {
        std::cerr << "Error: No se pudo cerrar el archivo.\n";
        return 1;
    }
    std::cout << "Archivo cerrado correctamente.\n\n";

    // Mostrar estado del archivo y uso de memoria
    std::cout << "Mostrando estado del archivo con contenido...\n";
    VersionedStorage::showFileStatusWithContent(filename);
    std::cout << "\n";
    VersionedStorage::showMemoryUsage(filename);
    std::cout << "\n";

    return 0;
}