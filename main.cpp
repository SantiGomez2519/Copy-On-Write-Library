#include <iostream>
#include "./include/file_manager.h"

int main() {
    std::string filename = "testfile.cow";
    
    // Crear metadatos
    VersionedStorage::FileMetadata meta(filename);
    
    // Crear archivo
    if (!VersionedStorage::create(filename)) {
        std::cerr << "Error al crear archivo" << std::endl;
        return 1;
    }
    
    // Datos de prueba
    std::vector<std::string> test_data = {
        "Primera version del archivo.",
        "Segunda version con cambios.",
        "Tercera version con mas cambios."
    };
    
    // Escribir versiones
    for (const auto& data : test_data) {
        if (!VersionedStorage::write(filename, meta, data.data(), data.size())) {
            std::cerr << "Error al escribir datos" << std::endl;
        }
    }
    
    // Leer versiones
    for (size_t i = 0; i <= meta.current_version; ++i) {
        std::string content;
        if (VersionedStorage::read(meta, i, content)) {
            std::cout << "Version " << i << ":\n" << content << "\n---\n";
        } else {
            std::cerr << "Error al leer version " << i << std::endl;
        }
    }
    
    return 0;
}