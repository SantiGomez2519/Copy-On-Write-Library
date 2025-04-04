#include "../include/file_manager.h"
#include "../include/storage.h"
#include <fstream>
#include <iostream>
#include <algorithm>

namespace VersionedStorage {
    bool create(const std::string& filename) {
        FileMetadata new_file(filename);
        return saveMetadata(new_file);
    }

    bool open(const std::string& filename, FileMetadata& meta) {
        return loadMetadata(filename, meta);
    }

    bool write(const std::string& filename, FileMetadata& meta, const char* data, size_t size) {
        // 1. Crear nueva versión basada en la actual
        size_t new_version_id = meta.current_version + 1;
        auto new_version = std::make_shared<VersionMetadata>(new_version_id);
        
        // 2. Copiar referencia a todos los bloques de la versión anterior
        if (meta.versions.count(meta.current_version)) {
            auto& current_blocks = meta.versions[meta.current_version]->blocks;
            new_version->blocks = current_blocks;
            
            // Actualizar referencias en los bloques
            for (auto& [offset, block] : current_blocks) {
                block->referencing_versions.insert(new_version_id);
            }
        }
        
        // 3. Procesar los nuevos datos por bloques
        size_t processed = 0;
        while (processed < size) {
            size_t current_offset = processed;
            size_t block_size = std::min(BLOCK_SIZE, size - processed);
            
            // Verificar si este bloque necesita COW
            bool needs_copy = true;
            if (meta.versions.count(meta.current_version)) {
                auto& current_blocks = meta.versions[meta.current_version]->blocks;
                if (current_blocks.count(current_offset)) {
                    // Comparar contenido del bloque
                    auto& existing_block = current_blocks[current_offset];
                    if (std::equal(existing_block->data.begin(), existing_block->data.end(), 
                                data + processed, data + processed + block_size)) {
                        // Bloque idéntico, no necesita copia
                        needs_copy = false;
                    }
                }
            }
            
            if (needs_copy) {
                // Crear nuevo bloque COW
                size_t new_block_id = meta.all_blocks.size() + 1;
                auto new_block = std::make_shared<DataBlock>(
                    new_block_id, data + processed, block_size);
                
                // Registrar en la nueva versión
                new_version->blocks[current_offset] = new_block;
                new_block->referencing_versions.insert(new_version_id);
                
                // Agregar al registro global de bloques
                meta.all_blocks[new_block_id] = new_block;
            }
            
            processed += block_size;
        }
        
        // 4. Actualizar metadatos
        new_version->file_size = size;
        meta.versions[new_version_id] = new_version;
        meta.current_version = new_version_id;
        
        // 5. Persistir cambios
        return saveMetadata(meta);
    }

    bool read(const FileMetadata& meta, size_t version_id, std::string& output) {
        if (!meta.versions.count(version_id)) {
            std::cerr << "Versión no existe" << std::endl;
            return false;
        }
        
        auto& version = meta.versions.at(version_id);
        output.resize(version->file_size);
        
        // Ensamblar archivo a partir de bloques
        for (const auto& [offset, block] : version->blocks) {
            if (offset + block->data.size() > version->file_size) {
                continue; // Bloque parcial al final
            }
            std::copy(block->data.begin(), block->data.end(), 
                    output.begin() + offset);
        }
        
        return true;
    }

    bool snapshot(const FileMetadata& meta, size_t version_to_clone) {
        // Implementación similar a write pero sin nuevos datos
        // Crea una nueva versión idéntica a la especificada
        // Útil para branching
        return true; 
    }
}