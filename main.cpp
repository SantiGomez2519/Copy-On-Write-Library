#include <iostream>
#include "./include/file_manager.h"
#include "./include/storage.h"

const size_t BLOCK_SIZE = 4096;  // Define BLOCK_SIZE as 4096 bytes.


void show_version_diff(const VersionedStorage::FileMetadata& meta, size_t ver1, size_t ver2) {
    if (!meta.versions.count(ver1) || !meta.versions.count(ver2)) {
    std::cerr << "Versiones no existen\n";
    return;
    }

    auto& v1 = meta.versions.at(ver1)->blocks;
    auto& v2 = meta.versions.at(ver2)->blocks;

    std::cout << "\nDiferencias entre versión " << ver1 << " y " << ver2 << ":\n";

    for (const auto& [offset, block] : v1) {
        if (v2.count(offset)) {
            if (v2.at(offset)->block_id == block->block_id) {
                std::cout << "Bloque @" << offset << ": Mismo (ID " << block->block_id << ")\n";
            } else {
                std::cout << "Bloque @" << offset << ": Diferente (" << block->block_id << " -> " << v2.at(offset)->block_id << ")\n";
            }
        }
    }
}

void test_serialization() {
    std::string test_file = "test_serialization.cow";
    
    // Crear datos de prueba
    VersionedStorage::FileMetadata original(test_file);
    std::string test_data = "Datos de prueba para serialización";
    VersionedStorage::write(test_file, original, test_data.data(), test_data.size());
    
    // Serializar
    if (!VersionedStorage::saveMetadata(original) || 
        !VersionedStorage::saveBlockData(original)) {
        std::cerr << "Error al serializar" << std::endl;
        return;
    }
    
    // Deserializar
    VersionedStorage::FileMetadata loaded(test_file);
    if (!VersionedStorage::loadMetadata(test_file, loaded) ||
        !VersionedStorage::loadBlockData(test_file, loaded)) {
        std::cerr << "Error al deserializar" << std::endl;
        return;
    }
    
    // Verificar
    std::string content;
    if (!VersionedStorage::read(loaded, 1, content)) {
        std::cerr << "Error al leer versión deserializada" << std::endl;
        return;
    }
    
    if (content != test_data) {
        std::cerr << "Error: Datos no coinciden después de serialización" << std::endl;
        std::cerr << "Esperado: " << test_data << std::endl;
        std::cerr << "Obtenido: " << content << std::endl;
        return;
    }
    
    std::cout << "✅ Serialización verificada correctamente" << std::endl;
}

void test_cow_blocks() {
    std::cout << "\n=== INICIANDO PRUEBA COW ===\n";
    
    std::string filename = "test_cow.cow";
    
    // 1. Preparación - crear archivo
    VersionedStorage::FileMetadata meta(filename);
    if (!VersionedStorage::create(filename)) {
        std::cerr << "Error al crear archivo" << std::endl;
        return;
    }

    // 2. Escribir datos iniciales (3 bloques llenos de 'A')
    std::string initial_data(BLOCK_SIZE * 3, 'A');
    if (!VersionedStorage::write(filename, meta, initial_data.data(), initial_data.size())) {
        std::cerr << "Error en escritura inicial" << std::endl;
        return;
    }

    std::cout << "\n=== DESPUÉS DE ESCRITURA INICIAL ===\n";
    VersionedStorage::inspectMetadata(filename);

    // 3. Modificar solo el segundo bloque (cambiar 1 byte)
    std::string modified_data = initial_data;
    size_t modify_offset = BLOCK_SIZE + 10; // Dentro del segundo bloque
    modified_data[modify_offset] = 'B';     // Cambiar un byte

    if (!VersionedStorage::write(filename, meta, modified_data.data(), modified_data.size())) {
        std::cerr << "Error en escritura modificada" << std::endl;
        return;
    }

    std::cout << "\n=== DESPUÉS DE MODIFICACIÓN (COW) ===\n";
    VersionedStorage::inspectMetadata(filename);

    // 4. Comprobaciones específicas de COW
    std::cout << "\n=== COMPROBACIONES COW ===\n";
    
    // Verificar número de versiones (debería ser 2: 1 y 2)
    if (meta.versions.size() != 3) { // Versión 0 (vacía) + 1 + 2
        std::cerr << "Error: Deberían existir 3 versiones (0, 1, 2), encontradas: " 
                  << meta.versions.size() << std::endl;
    } else {
        std::cout << "✅ Número correcto de versiones (3 incluyendo versión 0 vacía)" << std::endl;
    }

    // Obtener referencias a las versiones
    if (!meta.versions.count(1) || !meta.versions.count(2)) {
        std::cerr << "Error: No se encontraron las versiones 1 o 2" << std::endl;
        return;
    }
    
    auto& v1 = meta.versions[1];
    auto& v2 = meta.versions[2];

    // Verificar que el bloque modificado es diferente
    size_t modified_block_offset = BLOCK_SIZE; // Offset del segundo bloque
    if (!v1->blocks.count(modified_block_offset) || !v2->blocks.count(modified_block_offset)) {
        std::cerr << "Error: No se encontró el bloque modificado en una de las versiones" << std::endl;
    } 
    else if (v1->blocks.at(modified_block_offset)->block_id == v2->blocks.at(modified_block_offset)->block_id) {
        std::cerr << "Error: El bloque modificado debería tener un ID diferente" << std::endl;
    } else {
        std::cout << "✅ Bloque modificado tiene nuevo ID (COW funcionó)" << std::endl;
    }

    // Verificar que los bloques no modificados son iguales
    bool shared_blocks_ok = true;
    for (const auto& [offset, block] : v1->blocks) {
        if (offset != modified_block_offset) { // Bloques no modificados
            if (!v2->blocks.count(offset) || 
                v2->blocks.at(offset)->block_id != block->block_id) {
                shared_blocks_ok = false;
                std::cerr << "Error: Bloque no modificado @" << offset 
                          << " debería ser compartido" << std::endl;
            }
        }
    }
    if (shared_blocks_ok) {
        std::cout << "✅ Bloques no modificados son compartidos correctamente" << std::endl;
    }

    // Verificar referencias
    if (v2->blocks.count(modified_block_offset)) {
        auto& modified_block = v2->blocks.at(modified_block_offset);
        if (modified_block->referencing_versions.size() != 1 || 
            *modified_block->referencing_versions.begin() != 2) {
            std::cerr << "Error: Referencias del bloque modificado incorrectas" << std::endl;
        } else {
            std::cout << "✅ Referencias del bloque modificado correctas" << std::endl;
        }
    }

    // Mostrar diferencias entre versiones
    std::cout << "\n=== RESUMEN DE CAMBIOS COW ===\n";
    std::cout << "Versión 1: " << v1->blocks.size() << " bloques\n";
    std::cout << "Versión 2: " << v2->blocks.size() << " bloques\n";
    
    for (const auto& [offset, block] : v1->blocks) {
        if (v2->blocks.count(offset)) {
            if (v2->blocks.at(offset)->block_id == block->block_id) {
                std::cout << "Bloque @" << offset << ": COMPARTIDO (ID " << block->block_id << ")\n";
            } else {
                std::cout << "Bloque @" << offset << ": COPIADO ("
                          << block->block_id << " -> " << v2->blocks.at(offset)->block_id << ")\n";
            }
        }
    }

    std::cout << "\n=== PRUEBA COW COMPLETADA ===\n";
    show_version_diff(meta, 1, 2);
}


int main() {
    test_serialization();
    test_cow_blocks();
    // std::string filename = "testfile.cow";
    
    // // Crear metadatos
    // VersionedStorage::FileMetadata meta(filename);
    
    // // Crear archivo
    // if (!VersionedStorage::create(filename)) {
    //     std::cerr << "Error al crear archivo" << std::endl;
    //     return 1;
    // }
    
    // // Datos de prueba
    // std::vector<std::string> test_data = {
    //     "Primera version del archivo.",
    //     "Segunda version con cambios.",
    //     "Tercera version con mas cambios."
    // };
    
    // // Escribir versiones
    // for (const auto& data : test_data) {
    //     if (!VersionedStorage::write(filename, meta, data.data(), data.size())) {
    //         std::cerr << "Error al escribir datos" << std::endl;
    //     }
    // }
    
    // // Leer versiones
    // for (size_t i = 0; i <= meta.current_version; ++i) {
    //     std::string content;
    //     if (VersionedStorage::read(meta, i, content)) {
    //         std::cout << "Version " << i << ":\n" << content << "\n---\n";
    //     } else {
    //         std::cerr << "Error al leer version " << i << std::endl;
    //     }
    // }
    
    return 0;
}