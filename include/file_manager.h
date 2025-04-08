#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>

// Estructura para representar una versión del archivo
struct Version {
    size_t version_id;
    size_t offset;
    size_t size;

    // Constructor por defecto
    Version() : version_id(0), offset(0), size(0) {}

    // Constructor para inicializar correctamente
    Version(size_t id, size_t off, size_t sz) : version_id(id), offset(off), size(sz) {}
};

// Estructura para representar un bloque de datos modificados
struct DataBlock {
    size_t version_id;
    size_t offset;
    size_t size;

    // Constructor por defecto
    DataBlock() : version_id(0), offset(0), size(0) {}

    // Constructor para inicializar correctamente
    DataBlock(size_t ver, size_t off, size_t sz) : version_id(ver), offset(off), size(sz) {}
};

// Estructura para manejar el archivo y sus versiones
struct FileMetadata {
    std::string filename;   // Nombre del archivo base
    size_t total_versions;  // Número total de versiones
    std::vector<Version> versions;  // Historial de versiones
};

// Funciones para manejar archivos versionados
namespace VersionedStorage {
    bool create(const std::string& filename);
    bool open(const std::string& filename);
    bool write(const std::string& filename);
    bool read(const std::string& filename, size_t version_id, std::string& output);
    bool close(const std::string& filename);

    //Nueva función para mostrar el estado del archivo .data
    void mostrarEstadoDataFile(const std::string& filename);

    // Leer la última versión del archivo
    bool readLatestVersion(const std::string& filename, std::string& output);

    void showMemoryUsage(const std::string& filename);


}

#endif // FILE_MANAGER_H