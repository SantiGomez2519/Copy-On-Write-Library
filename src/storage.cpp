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

    void mostrarEstadoDataFile(const std::string& filename) {
        std::string data_file = filename + ".data";
        std::string meta_file = filename + ".meta";
    
        std::ifstream meta_in(meta_file, std::ios::binary);
        if (!meta_in) {
            std::cerr << "No se pudo abrir el archivo .meta." << std::endl;
            return;
        }
    
        // Leer nombre
        size_t name_length;
        meta_in.read(reinterpret_cast<char*>(&name_length), sizeof(size_t));
        std::string name(name_length, '\0');
        meta_in.read(&name[0], name_length);
    
        // Leer total de versiones
        size_t total_versions;
        meta_in.read(reinterpret_cast<char*>(&total_versions), sizeof(size_t));
    
        std::cout << "\n📦 Estado del archivo de datos (" << data_file << "):\n";
        std::cout << "Archivo: " << name << "\nTotal de versiones: " << total_versions << std::endl;
    
        for (size_t i = 0; i < total_versions; ++i) {
            Version v;
            meta_in.read(reinterpret_cast<char*>(&v), sizeof(Version));
            std::cout << " - Version " << i << ": offset=" << v.offset << ", tamano=" << v.size << " bytes\n";
        }
    
        // Tamaño del archivo .data
        std::ifstream data_in(data_file, std::ios::binary | std::ios::ate);
        if (data_in) {
            size_t filesize = data_in.tellg();
            std::cout << "Tamano total del archivo .data: " << filesize << " bytes\n";
            data_in.close();
        } else {
            std::cerr << "No se pudo abrir el archivo .data para leer su tamano." << std::endl;
        }
    
        meta_in.close();
    }

    void garbageCollector(FileMetadata& metadata) {
        std::string data_file = metadata.filename + ".data";
        std::string temp_file = metadata.filename + ".temp";
    
        // Abrimos el archivo .data original en modo lectura binaria
        std::ifstream original(data_file, std::ios::binary);
        if (!original) {
            std::cerr << " Error al abrir el archivo .data para GC.\n";
            return;
        }
    
        // Abrimos archivo temporal para reescribir las versiones que se conservarán
        std::ofstream temp(temp_file, std::ios::binary | std::ios::trunc);
        if (!temp) {
            std::cerr << " Error al crear archivo temporal para GC.\n";
            original.close();
            return;
        }
    
        // Eliminar la versión más antigua (la primera en el vector)
        if (metadata.versions.empty()) {
            std::cerr << " No hay versiones para limpiar.\n";
            original.close();
            temp.close();
            return;
        }
    
        metadata.versions.erase(metadata.versions.begin());
        metadata.total_versions--;
    
        // Vector con las versiones actualizadas (con nuevos offsets)
        std::vector<Version> nuevas_versions;
    
        // Copiar las versiones restantes al archivo temporal con nuevos offsets
        for (const auto& version : metadata.versions) {
            std::vector<char> buffer(version.size);
    
            // Leer contenido desde el archivo original usando offset original
            original.seekg(version.offset, std::ios::beg);
            original.read(buffer.data(), version.size);
    
            // Nuevo offset es la posición actual del archivo temporal
            size_t nuevo_offset = temp.tellp();
    
            // Escribir el contenido en el archivo temporal
            temp.write(buffer.data(), version.size);
    
            // Crear nueva versión con el nuevo offset
            Version nueva_version;
            nueva_version.version_id = nuevas_versions.size();  // Reasignar IDs si lo necesitas
            nueva_version.offset = nuevo_offset;
            nueva_version.size = version.size;
    
            nuevas_versions.push_back(nueva_version);
        }
    
        // Cerrar archivos
        original.close();
        temp.close();
    
        // Reemplazar el archivo original con el temporal
        std::remove(data_file.c_str());
        std::rename(temp_file.c_str(), data_file.c_str());
    
        // Actualizar metadatos y sobrescribir el .meta
        metadata.versions = nuevas_versions;
        saveMetadata(metadata);
    
        std::cout << " Garbage Collector completado. Ahora hay " << metadata.total_versions << " versiones activas.\n";
    }
    
    
    

}

