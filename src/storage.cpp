#include "../include/storage.h"
#include <fstream>
#include <sstream>
#include <cstring> // Para memcpy
#include <iomanip> // Para std::hex
#include <iostream>

namespace VersionedStorage {

    // Constantes para el formato de archivo
    const uint32_t MAGIC_NUMBER = 0x4D574F43; // "COWM" en little-endian
    const uint8_t FORMAT_VERSION = 0x01;

    // Funciones auxiliares para escritura binaria
    template<typename T>
    void write_binary(std::ostream& out, const T& value) {
        out.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    template<typename T>
    void read_binary(std::istream& in, T& value) {
        in.read(reinterpret_cast<char*>(&value), sizeof(T));
    }

    bool saveMetadata(const FileMetadata& meta) {
        std::ofstream meta_file(meta.filename + ".meta", std::ios::binary);
        if (!meta_file) return false;
        
        // Escribir header
        write_binary(meta_file, MAGIC_NUMBER);
        write_binary(meta_file, FORMAT_VERSION);
        
        // Escribir metadatos básicos
        size_t filename_length = meta.filename.size();
        write_binary(meta_file, filename_length);
        meta_file.write(meta.filename.c_str(), filename_length);
        write_binary(meta_file, meta.current_version);
        
        // Escribir versiones
        write_binary(meta_file, meta.versions.size());
        for (const auto& [ver_id, ver_meta] : meta.versions) {
            write_binary(meta_file, ver_id);
            write_binary(meta_file, ver_meta->file_size);
            
            // Escribir bloques de esta versión
            write_binary(meta_file, ver_meta->blocks.size());
            for (const auto& [offset, block] : ver_meta->blocks) {
                write_binary(meta_file, offset);
                write_binary(meta_file, block->block_id);
            }
        }
        
        // Escribir metadatos de bloques
        write_binary(meta_file, meta.all_blocks.size());
        for (const auto& [block_id, block] : meta.all_blocks) {
            write_binary(meta_file, block_id);
            write_binary(meta_file, block->data.size());
            
            // Escribir versiones que referencian este bloque
            write_binary(meta_file, block->referencing_versions.size());
            for (const auto& ver_id : block->referencing_versions) {
                write_binary(meta_file, ver_id);
            }
        }
        
        return meta_file.good();
    }

    bool loadMetadata(const std::string& filename, FileMetadata& meta) {
        std::ifstream meta_file(filename + ".meta", std::ios::binary);
        if (!meta_file) return false;
        
        // Leer y verificar header
        uint32_t magic;
        read_binary(meta_file, magic);
        if (magic != MAGIC_NUMBER) return false;
        
        uint8_t version;
        read_binary(meta_file, version);
        if (version != FORMAT_VERSION) return false;
        
        // Leer metadatos básicos
        size_t filename_length;
        read_binary(meta_file, filename_length);
        
        std::vector<char> filename_buf(filename_length);
        meta_file.read(filename_buf.data(), filename_length);
        meta.filename.assign(filename_buf.begin(), filename_buf.end());
        
        read_binary(meta_file, meta.current_version);
        
        // Leer versiones
        size_t num_versions;
        read_binary(meta_file, num_versions);
        
        for (size_t i = 0; i < num_versions; ++i) {
            size_t ver_id, file_size, num_blocks;
            read_binary(meta_file, ver_id);
            read_binary(meta_file, file_size);
            
            auto version_meta = std::make_shared<VersionMetadata>(ver_id);
            version_meta->file_size = file_size;
            
            read_binary(meta_file, num_blocks);
            for (size_t j = 0; j < num_blocks; ++j) {
                size_t offset, block_id;
                read_binary(meta_file, offset);
                read_binary(meta_file, block_id);
                
                // El bloque se cargará cuando leamos los metadatos de bloques
                version_meta->blocks[offset] = nullptr; // Placeholder
            }
            
            meta.versions[ver_id] = version_meta;
        }
        
        // Leer metadatos de bloques
        size_t num_blocks;
        read_binary(meta_file, num_blocks);
        
        for (size_t i = 0; i < num_blocks; ++i) {
            size_t block_id, block_size, num_refs;
            read_binary(meta_file, block_id);
            read_binary(meta_file, block_size);
            
            auto block = std::make_shared<DataBlock>(block_id, nullptr, 0);
            block->data.resize(block_size);
            
            read_binary(meta_file, num_refs);
            for (size_t j = 0; j < num_refs; ++j) {
                size_t ver_id;
                read_binary(meta_file, ver_id);
                block->referencing_versions.insert(ver_id);
                
                // Actualizar referencias en versiones
                if (meta.versions.find(ver_id) != meta.versions.end()) {
                    for (auto& [offset, b] : meta.versions[ver_id]->blocks) {
                        if (b == nullptr) { // Nuestro placeholder
                            b = block;
                            break;
                        }
                    }
                }
            }
            
            meta.all_blocks[block_id] = block;
        }
        
        return meta_file.good();
    }

    bool saveBlockData(const FileMetadata& meta) {
        std::ofstream block_file(meta.filename + ".blocks", std::ios::binary);
        if (!block_file) return false;
        
        // Escribir todos los bloques
        for (const auto& [block_id, block] : meta.all_blocks) {
            write_binary(block_file, block_id);
            write_binary(block_file, block->data.size());
            block_file.write(block->data.data(), block->data.size());
        }
        
        return block_file.good();
    }

    bool loadBlockData(const std::string& filename, FileMetadata& meta) {
        std::ifstream block_file(filename + ".blocks", std::ios::binary);
        if (!block_file) return false;
        
        while (block_file.peek() != EOF) {
            size_t block_id, block_size;
            read_binary(block_file, block_id);
            read_binary(block_file, block_size);
            
            if (meta.all_blocks.find(block_id) != meta.all_blocks.end()) {
                auto& block = meta.all_blocks[block_id];
                block->data.resize(block_size);
                block_file.read(block->data.data(), block_size);
            } else {
                // Bloque desconocido, saltar
                block_file.seekg(block_size, std::ios::cur);
            }
        }
        
        return true;
    }

    void displayHexDump(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) return;
    
        const size_t chunk_size = 16;
        char buffer[chunk_size];
        std::vector<char> last_chunk;
        size_t repeated_count = 0;
        size_t offset = 0;
    
        auto print_chunk = [](size_t offset, const char* data, size_t size) {
            std::cout << std::hex << std::setw(6) << std::setfill('0') << offset << "  ";
            for (size_t i = 0; i < size; ++i) {
                std::cout << std::setw(2) << std::setfill('0') 
                          << (static_cast<unsigned int>(data[i]) & 0xFF) << " ";
            }
            std::cout << " |";
            for (size_t i = 0; i < size; ++i) {
                unsigned char c = data[i];
                std::cout << (isprint(c) ? c : '.');
            }
            std::cout << "|\n";
        };
    
        while (file.read(buffer, chunk_size)) {
            if (last_chunk.empty()) {
                last_chunk.assign(buffer, buffer + file.gcount());
            } else if (std::equal(last_chunk.begin(), last_chunk.end(), buffer)) {
                repeated_count++;
                offset += chunk_size;
                continue;
            }
    
            if (repeated_count > 0) {
                std::cout << "..." << repeated_count << " repeated lines...\n";
                repeated_count = 0;
            }
    
            print_chunk(offset, buffer, file.gcount());
            last_chunk.assign(buffer, buffer + file.gcount());
            offset += chunk_size;
        }
    
        if (repeated_count > 0) {
            std::cout << "..." << repeated_count << " repeated lines...\n";
        }
    }
    
    void inspectMetadata(const std::string& filename) {
        std::string meta_file = filename + ".meta";
        std::string block_file = filename + ".blocks";
        
        // Mostrar hex dump
        displayHexDump(meta_file);
        displayHexDump(block_file);
        
        // Mostrar interpretación estructurada
        FileMetadata meta(filename);
        if (!loadMetadata(filename, meta)) {
            std::cerr << "No se pudo cargar metadata para inspección" << std::endl;
            return;
        }
    
        std::cout << "\nInterpretación estructurada:\n";
        std::cout << "Archivo: " << meta.filename << "\n";
        std::cout << "Versión actual: " << meta.current_version << "\n";
        std::cout << "Versiones almacenadas: " << meta.versions.size() << "\n\n";
    
        for (const auto& [ver_id, ver_meta] : meta.versions) {
            std::cout << "Versión " << ver_id << " (tamaño: " << ver_meta->file_size << " bytes)\n";
            std::cout << "Bloques usados: " << ver_meta->blocks.size() << "\n";
            
            for (const auto& [offset, block] : ver_meta->blocks) {
                std::cout << "  Offset " << offset << " -> Bloque " << block->block_id 
                          << " (" << block->data.size() << " bytes)\n";
            }
            std::cout << std::endl;
        }
    
        std::cout << "\nBloques únicos: " << meta.all_blocks.size() << "\n";
        for (const auto& [block_id, block] : meta.all_blocks) {
            std::cout << "Bloque " << block_id << " (" << block->data.size() << " bytes) "
                      << "Referenciado por " << block->referencing_versions.size() 
                      << " versiones\n";
        }
    }
}