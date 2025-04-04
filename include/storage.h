#ifndef STORAGE_H
#define STORAGE_H

#include "file_manager.h"

namespace VersionedStorage {
    bool saveMetadata(const FileMetadata& meta);
    bool loadMetadata(const std::string& filename, FileMetadata& meta);
    bool saveBlockData(const FileMetadata& meta);
    bool loadBlockData(const std::string& filename, FileMetadata& meta);
    void displayHexDump(const std::string& filename);
    void inspectMetadata(const std::string& filename);
}

#endif // STORAGE_H