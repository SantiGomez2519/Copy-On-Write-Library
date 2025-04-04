// file_manager.h
#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <set>

namespace VersionedStorage {
    // Tamaño de bloque fijo
    constexpr size_t BLOCK_SIZE = 4096;

    struct DataBlock {
        size_t block_id;
        std::vector<char> data;
        std::set<size_t> referencing_versions;
        
        DataBlock(size_t id, const char* block_data, size_t size) 
            : block_id(id), data(block_data, block_data + size) {}
    };

    struct VersionMetadata {
        size_t version_id;
        std::unordered_map<size_t, std::shared_ptr<DataBlock>> blocks;
        size_t file_size;
        
        VersionMetadata(size_t id) : version_id(id), file_size(0) {}
    };

    struct FileMetadata {
        std::string filename;
        size_t current_version;
        std::unordered_map<size_t, std::shared_ptr<VersionMetadata>> versions;
        std::unordered_map<size_t, std::shared_ptr<DataBlock>> all_blocks;
        
        FileMetadata(const std::string& name) : filename(name), current_version(0) {
            versions[0] = std::make_shared<VersionMetadata>(0);
        }
    };

    // Declaraciones de funciones
    bool create(const std::string& filename);
    bool open(const std::string& filename, FileMetadata& meta);
    bool write(const std::string& filename, FileMetadata& meta, const char* data, size_t size);
    bool read(const FileMetadata& meta, size_t version_id, std::string& output);
    bool close(FileMetadata& meta);
    bool snapshot(const FileMetadata& meta, size_t version_to_clone);
}

#endif // FILE_MANAGER_H