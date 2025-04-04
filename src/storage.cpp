#include "../include/storage.h"
#include <fstream>
#include <iostream>

namespace VersionedStorage {

    bool saveMetadata(const FileMetadata& metadata) {
        std::string metadata_file = metadata.filename + ".meta";
        std::ofstream meta_out(metadata_file, std::ios::binary | std::ios::trunc);
        if (!meta_out) {
            std::cerr << "Error al guardar la metadata.\n";
            return false;
        }

        size_t name_length = metadata.filename.size();
        meta_out.write(reinterpret_cast<const char*>(&name_length), sizeof(size_t));
        meta_out.write(metadata.filename.c_str(), name_length);
        meta_out.write(reinterpret_cast<const char*>(&metadata.total_versions), sizeof(size_t));

        for (const auto& version : metadata.versions) {
            meta_out.write(reinterpret_cast<const char*>(&version), sizeof(Version));
        }

        meta_out.close();
        return true;
    }

    bool loadMetadata(const std::string& filename, FileMetadata& metadata) {
        std::string metadata_file = filename + ".meta";
        std::ifstream meta_in(metadata_file, std::ios::binary);
        if (!meta_in) {
            std::cerr << "Error al abrir la metadata.\n";
            return false;
        }

        size_t name_length;
        meta_in.read(reinterpret_cast<char*>(&name_length), sizeof(size_t));

        if (name_length > 255) {
            std::cerr << "Error: Nombre de archivo corrupto.\n";
            return false;
        }

        metadata.filename.resize(name_length);
        meta_in.read(&metadata.filename[0], name_length);
        meta_in.read(reinterpret_cast<char*>(&metadata.total_versions), sizeof(size_t));

        if (metadata.total_versions > 1000) {
            std::cerr << "Error: Número de versiones sospechoso.\n";
            return false;
        }

        metadata.versions.resize(metadata.total_versions);
        for (size_t i = 0; i < metadata.total_versions; ++i) {
            meta_in.read(reinterpret_cast<char*>(&metadata.versions[i]), sizeof(Version));
        }

        meta_in.close();
        return true;
    }
}

