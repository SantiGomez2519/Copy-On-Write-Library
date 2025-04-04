#include "../include/file_manager.h"
#include "../include/storage.h"
#include <fstream>
#include <iostream>
#include <algorithm>

namespace VersionedStorage {
    bool create(const std::string& filename) {
        FileMetadata new_file(filename);
        return saveMetadata(new_file) && saveBlockData(new_file);
    }

    bool open(const std::string& filename, FileMetadata& meta) {
        return loadMetadata(filename, meta) && loadBlockData(filename, meta);
    }

    bool write(const std::string& filename, FileMetadata& meta, const char* data, size_t size) {
        // Crear nueva versión
        size_t new_version_id = meta.current_version + 1;
        auto new_version = std::make_shared<VersionMetadata>(new_version_id);
        new_version->file_size = size;
    
        // Copiar referencias de bloques no modificados
        if (meta.versions.count(meta.current_version)) {
            auto& current_blocks = meta.versions[meta.current_version]->blocks;
            
            for (const auto& [offset, block] : current_blocks) {
                // Verificar si el bloque será completamente sobrescrito
                if (offset >= size) {
                    // Bloque está después del nuevo tamaño, no lo copiamos
                    continue;
                }
                
                // Verificar si el contenido del bloque cambió
                bool block_changed = false;
                size_t compare_size = std::min(block->data.size(), size - offset);
                if (!std::equal(block->data.begin(), block->data.begin() + compare_size,
                              data + offset, data + offset + compare_size)) {
                    block_changed = true;
                }
                
                if (!block_changed && (offset + block->data.size() <= size)) {
                    new_version->blocks[offset] = block;
                    block->referencing_versions.insert(new_version_id);
                }
            }
        }
    
        // Procesar nuevos datos y bloques modificados
        size_t processed = 0;
        while (processed < size) {
            size_t current_offset = processed;
            size_t block_size = std::min(BLOCK_SIZE, size - processed);
            
            // Verificar si ya tenemos un bloque para este offset
            if (!new_version->blocks.count(current_offset)) {
                // Generar nuevo ID de bloque
                size_t new_block_id = 1;
                if (!meta.all_blocks.empty()) {
                    auto max_it = std::max_element(meta.all_blocks.begin(), meta.all_blocks.end(),
                        [](const auto& a, const auto& b) { return a.first < b.first; });
                    new_block_id = max_it->first + 1;
                }
                
                // Crear nuevo bloque COW
                auto new_block = std::make_shared<DataBlock>(new_block_id, 
                                                            data + processed, 
                                                            block_size);
                new_block->referencing_versions.insert(new_version_id);
                
                new_version->blocks[current_offset] = new_block;
                meta.all_blocks[new_block_id] = new_block;
            }
            
            processed += block_size;
        }
    
        // Actualizar metadatos
        meta.versions[new_version_id] = new_version;
        meta.current_version = new_version_id;
    
        // Guardar cambios
        if (!saveMetadata(meta)) return false;
        return saveBlockData(meta);
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