#ifndef STORAGE_H
#define STORAGE_H

#include "file_manager.h"

namespace VersionedStorage {
    bool saveMetadata(const FileMetadata& meta);
    bool loadMetadata(const std::string& filename, FileMetadata& meta);
}

#endif // STORAGE_H