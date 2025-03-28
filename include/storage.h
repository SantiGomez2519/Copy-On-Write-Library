#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include "file_manager.h"

// 📌 Espacio de nombres para funciones de almacenamiento
namespace VersionedStorage {
    bool saveMetadata(const FileMetadata& metadata);
    bool loadMetadata(const std::string& filename, FileMetadata& metadata);
}

#endif // STORAGE_H

