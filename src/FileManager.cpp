#include "../include/FileManager.h"
#include <fstream>

// Copiar un archivo dentro de 'storage/'
bool FileManager::copyFile(const std::string& source, const std::string& destination) {
    std::ifstream src("storage/" + source, std::ios::binary);
    std::ofstream dest("storage/" + destination, std::ios::binary);
    return src && dest && (dest << src.rdbuf());
}

