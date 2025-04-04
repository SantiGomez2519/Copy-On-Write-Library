#include "../include/storage.h"
#include <fstream>
#include <sstream>

namespace VersionedStorage {

    bool saveMetadata(const FileMetadata& meta) {
        std::ofstream meta_file(meta.filename + ".meta", std::ios::binary);
        if (!meta_file) return false;
        
        // Serialización básica (implementar completa)
        meta_file << meta.filename << "\n";
        meta_file << meta.current_version << "\n";
        
        for (const auto& [ver_id, ver_meta] : meta.versions) {
            meta_file << ver_id << " " << ver_meta->file_size << "\n";
        }
        
        return true;
    }

    bool loadMetadata(const std::string& filename, FileMetadata& meta) {
        std::ifstream meta_file(filename + ".meta", std::ios::binary);
        if (!meta_file) return false;
        
        // Deserialización básica (implementar completa)
        std::string line;
        std::getline(meta_file, line); // filename
        meta.filename = line;
        
        std::getline(meta_file, line); // current_version
        meta.current_version = std::stoul(line);
        
        // Cargar versiones (implementación simplificada)
        while (std::getline(meta_file, line)) {
            size_t ver_id, file_size;
            std::istringstream iss(line);
            iss >> ver_id >> file_size;
            meta.versions[ver_id] = std::make_shared<VersionMetadata>(ver_id);
            meta.versions[ver_id]->file_size = file_size;
        }
        
        return true;
    }
}