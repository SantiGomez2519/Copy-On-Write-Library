#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>

// Estructura para representar una versión del archivo
struct Version {
    size_t version_id;
    size_t offset;
    size_t size;
    size_t user_id;   // Nuevo campo: autor del cambio

    // Constructor por defecto
    Version() : version_id(0), offset(0), size(0), user_id(0) {}

    Version(size_t id, size_t off, size_t sz, size_t user)
        : version_id(id), offset(off), size(sz), user_id(user) {}


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
    bool write(const std::string& filename, size_t user_id); // ← Actualizado
    bool read(const std::string& filename, size_t version_id, std::string& output);
    bool close(const std::string& filename);

    //Nueva función para mostrar el estado del archivo con contenido .data
    void showFileStatusWithContent(const std::string& filename);
    //igual que la anterior pero sin contenido
    void showFileStatus(const std::string& filename);

    // Leer la última versión del archivo
    bool readLatestVersion(const std::string& filename, std::string& output);

    void showMemoryUsage(const std::string& filename);

    // Copia un archivo desde una ruta fuente a una ruta destino
    bool copyFile(const std::string& src, const std::string& dst);


}

#endif // FILE_MANAGER_H