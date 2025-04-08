#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>  // Asegura que std::vector esté disponible
#include <fstream> // Necesario para leer/escribir archivos
#include "file_manager.h"

// Espacio de nombres para funciones de almacenamiento de metadatos
namespace VersionedStorage {
    // Guarda los metadatos de un archivo versionado en disco
    bool saveMetadata(const FileMetadata& metadata);

    // Carga los metadatos de un archivo versionado desde disco
    bool loadMetadata(const std::string& filename, FileMetadata& metadata);

    

    void garbageCollector(FileMetadata& metadata);
}

#endif // STORAGE_H

