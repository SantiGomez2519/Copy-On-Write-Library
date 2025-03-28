#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>

// 📌 Estructura para representar una versión del archivo
struct Version {
    size_t version_id;      // Identificador de la versión
    size_t offset;          // Ubicación en el archivo de datos
    size_t size;            // Tamaño de los datos almacenados
};

// 📌 Estructura para manejar el archivo y sus versiones
struct FileMetadata {
    std::string filename;   // Nombre del archivo base
    size_t total_versions;  // Número total de versiones
    std::vector<Version> versions;  // Historial de versiones
};

// 📌 Funciones para manejar archivos versionados
namespace VersionedStorage {
    bool create(const std::string& filename);
    bool open(const std::string& filename);
    bool write(const std::string& filename, const std::string& data);
    bool read(const std::string& filename, size_t version_id, std::string& output);
    bool close(const std::string& filename);
}

#endif // FILE_MANAGER_H

