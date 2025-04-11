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
    
        // Guardar nombre del archivo
        size_t name_length = metadata.filename.size();
        meta_out.write(reinterpret_cast<const char*>(&name_length), sizeof(size_t));
        meta_out.write(metadata.filename.c_str(), name_length);
    
        // Guardar número de versiones
        meta_out.write(reinterpret_cast<const char*>(&metadata.total_versions), sizeof(size_t));
    
        // Guardar cada versión
        for (const auto& version : metadata.versions) {
            meta_out.write(reinterpret_cast<const char*>(&version.version_id), sizeof(size_t));
            meta_out.write(reinterpret_cast<const char*>(&version.offset), sizeof(size_t));
            meta_out.write(reinterpret_cast<const char*>(&version.size), sizeof(size_t));
    
            // Guardar user_id como número
            meta_out.write(reinterpret_cast<const char*>(&version.user_id), sizeof(size_t));
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
    
        // Leer nombre del archivo
        size_t name_length;
        meta_in.read(reinterpret_cast<char*>(&name_length), sizeof(size_t));
    
        if (name_length > 255) {
            std::cerr << "Error: Nombre de archivo corrupto.\n";
            return false;
        }
    
        metadata.filename.resize(name_length);
        meta_in.read(&metadata.filename[0], name_length);
    
        // Leer número de versiones
        meta_in.read(reinterpret_cast<char*>(&metadata.total_versions), sizeof(size_t));
        if (metadata.total_versions > 1000) {
            std::cerr << "Error: Número de versiones sospechoso.\n";
            return false;
        }
    
        // Leer versiones
        metadata.versions.clear();
        for (size_t i = 0; i < metadata.total_versions; ++i) {
            Version v;
            meta_in.read(reinterpret_cast<char*>(&v.version_id), sizeof(size_t));
            meta_in.read(reinterpret_cast<char*>(&v.offset), sizeof(size_t));
            meta_in.read(reinterpret_cast<char*>(&v.size), sizeof(size_t));
    
            // Leer user_id como número
            meta_in.read(reinterpret_cast<char*>(&v.user_id), sizeof(size_t));
    
            metadata.versions.push_back(v);
        }
    
        meta_in.close();
        return true;
    }
    

    void showFileStatusWithContent(const std::string& filename) {
        std::string data_file = filename + ".data";
        std::string meta_file = filename + ".meta";
    
        std::ifstream meta_in(meta_file, std::ios::binary);
        if (!meta_in) {
            std::cerr << "No se pudo abrir el archivo .meta." << std::endl;
            return;
        }
    
        // Leer nombre del archivo
        size_t name_length;
        meta_in.read(reinterpret_cast<char*>(&name_length), sizeof(size_t));
        std::string name(name_length, '\0');
        meta_in.read(&name[0], name_length);
    
        // Leer total de versiones
        size_t total_versions;
        meta_in.read(reinterpret_cast<char*>(&total_versions), sizeof(size_t));
    
        std::cout << "\nEstado del archivo de datos (" << data_file << "):\n";
        std::cout << "Archivo: " << name << "\nTotal de versiones: " << total_versions << std::endl;
    
        for (size_t i = 0; i < total_versions; ++i) {
            Version v;
            meta_in.read(reinterpret_cast<char*>(&v), sizeof(Version));
            std::cout << " - Version " << v.version_id << ": offset=" << v.offset << ", tamano=" << v.size << " bytes\n";
            std::cout << "   Usuario ID: " << v.user_id << "\n";
    
            std::ifstream data_in(data_file, std::ios::binary);
            if (data_in) {
                data_in.seekg(v.offset, std::ios::beg);
                std::string contenido(v.size, '\0');
                data_in.read(&contenido[0], v.size);
                std::cout << "   Contenido: \"" << contenido << "\"\n";
            }
        }
    
        // Tamaño total del archivo .data
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
            nueva_version.user_id = version.user_id;
    
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
    
    bool readLatestVersion(const std::string& filename, std::string& output) {
        FileMetadata metadata;
        if (!loadMetadata(filename, metadata)) {
            std::cerr << "Error: No se pudo cargar el archivo de metadatos.\n";
            return false;
        }
    
        if (metadata.total_versions == 0) {
            std::cerr << "No hay versiones disponibles para leer.\n";
            return false;
        }
    
        const Version& version = metadata.versions.back();
        std::string data_file = filename + ".data";
    
        std::ifstream data_in(data_file, std::ios::binary);
        if (!data_in) {
            std::cerr << "Error: No se pudo abrir el archivo de datos para lectura.\n";
            return false;
        }
    
        std::vector<char> buffer(version.size);
        data_in.seekg(version.offset);
        data_in.read(buffer.data(), version.size);
        data_in.close();
    
        output.assign(buffer.begin(), buffer.end());
    
        std::cout << "Ultima version (" << version.version_id << ") leida correctamente:\n";
        std::cout << "Usuario ID: " << version.user_id << "\nContenido: " << output << "\n";
    
        return true;
    }
    

    void showMemoryUsage(const std::string& filename) {
        std::string data_file = filename + ".data";
        std::string meta_file = filename + ".meta";
    
        std::ifstream data_in(data_file, std::ios::binary | std::ios::ate);
        std::ifstream meta_in(meta_file, std::ios::binary | std::ios::ate);
    
        if (!data_in || !meta_in) {
            std::cerr << "No se pudieron abrir los archivos para leer el uso de memoria.\n";
            return;
        }
    
        size_t data_size = data_in.tellg();
        size_t meta_size = meta_in.tellg();
        size_t total = data_size + meta_size;
    
        std::cout << "\nUso actual de memoria de la biblioteca:\n\n";
    
        std::cout << "+----------------+----------------------+\n";
        std::cout << "| Archivo        | Tamano en bytes      |\n";
        std::cout << "+----------------+----------------------+\n";
    
        std::string data_str = std::to_string(data_size);
        std::string meta_str = std::to_string(meta_size);
        std::string total_str = std::to_string(total);
    
        while (data_str.length() < 20) data_str += ' ';
        while (meta_str.length() < 20) meta_str += ' ';
        while (total_str.length() < 20) total_str += ' ';
    
        std::cout << "| .data          | " << data_str << "|\n";
        std::cout << "| .meta          | " << meta_str << "|\n";
        std::cout << "| TOTAL          | " << total_str << "|\n";
        std::cout << "+----------------+----------------------+\n";
    
        data_in.close();
        meta_in.close();
    }
    
    
      

}
