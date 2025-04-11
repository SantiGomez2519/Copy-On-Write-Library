#include "../include/file_manager.h"
#include "../include/storage.h"
#include <fstream>
#include <iostream>

namespace VersionedStorage {

    bool create(const std::string& filename) {
        std::string metadata_file = filename + ".meta";

        std::ofstream meta_out(metadata_file, std::ios::binary | std::ios::trunc);
        if (!meta_out) {
            std::cerr << "Error al crear el archivo de metadatos.\n";
            return false;
        }

        FileMetadata metadata;
        metadata.filename = filename;
        metadata.total_versions = 0;

        size_t name_length = metadata.filename.size();
        meta_out.write(reinterpret_cast<const char*>(&name_length), sizeof(size_t));
        meta_out.write(metadata.filename.c_str(), name_length);
        meta_out.write(reinterpret_cast<const char*>(&metadata.total_versions), sizeof(size_t));

        meta_out.close();
        std::cout << "Archivo creado exitosamente: " << filename << std::endl;
        return true;
    }

    bool open(const std::string& filename) {
        FileMetadata metadata;
        if (!loadMetadata(filename, metadata)) {
            std::cerr << "No se pudo abrir el archivo: " << filename << std::endl;
            return false;
        }

        std::cout << "Archivo abierto exitosamente: " << filename << std::endl;
        std::cout << "Total de versiones: " << metadata.total_versions << std::endl;
        return true;
    }

    bool write(const std::string& filename, size_t user_id) {


        std::string metadata_file = filename + ".meta";
        std::string data_file = filename + ".data";
    
        // Cargar metadatos
        FileMetadata metadata;
        if (!loadMetadata(filename, metadata)) {
            std::cerr << "Error: No se pudo cargar el archivo de metadatos.\n";
            return false;
        }
    
        // Abrir el archivo original para leer su contenido
        std::ifstream file_in(filename, std::ios::binary);
        if (!file_in) {
            std::cerr << "Error: No se pudo abrir el archivo original para leer cambios.\n";
            return false;
        }
    
        // Leer el contenido actual del archivo
        std::vector<char> current_data((std::istreambuf_iterator<char>(file_in)), std::istreambuf_iterator<char>());
        file_in.close();
    
        // Obtener el último offset disponible
        size_t last_offset = (metadata.total_versions == 0) ? 0 :
                             metadata.versions.back().offset + metadata.versions.back().size;
    
        
    
        // Guardar el nuevo contenido en el archivo de datos
        std::ofstream data_out(data_file, std::ios::binary | std::ios::app);
        if (!data_out) {
            std::cerr << "Error: No se pudo abrir el archivo de datos para escribir.\n";
            return false;
        }
    
        data_out.seekp(last_offset);
        data_out.write(current_data.data(), current_data.size());
        data_out.close();
    
        // Registrar la nueva versión
        Version new_version;
        new_version.version_id = metadata.total_versions;
        new_version.offset = last_offset;
        new_version.size = current_data.size();
        new_version.user_id = user_id;
    
        metadata.versions.push_back(new_version);
        metadata.total_versions++;
    
        // Guardar metadatos actualizados
        std::ofstream meta_out(metadata_file, std::ios::binary | std::ios::trunc);
        if (!meta_out) {
            std::cerr << "Error: No se pudo actualizar el archivo de metadatos.\n";
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
    
        std::cout << "Nueva version " << new_version.version_id << " guardada correctamente (user_id: " << user_id << ").\n";
    
        if (metadata.total_versions > 5) {
            std::cout << " Ejecutando Garbage Collector...\n";

            std::cout << " \n Estado del archivo .data antes del Garbage Collector:\n";
            VersionedStorage::showFileStatus(filename);

            garbageCollector(metadata);
            std::cout << " Garbage Collector completado.\n";
            std::cout << " \n Estado del archivo .data despues del Garbage Collector:\n";
            VersionedStorage::showFileStatus(filename);
        }

        return true;
    }
    
    
    bool read(const std::string& filename, size_t version_id, std::string& output) {
        FileMetadata metadata;
        if (!loadMetadata(filename, metadata)) {
            std::cerr << "No se pudo cargar la metadata para leer el archivo: " << filename << std::endl;
            return false;
        }
    
        if (version_id >= metadata.total_versions) {
            std::cerr << "Error: Version no valida.\n";
            return false;
        }
    
        Version version = metadata.versions[version_id];
        std::string data_file = filename + ".data";
        std::ifstream data_in(data_file, std::ios::binary);
        if (!data_in) {
            std::cerr << "Error al abrir el archivo de datos para lectura.\n";
            return false;
        }
    
        std::vector<char> buffer(version.size);
        data_in.seekg(version.offset);
        data_in.read(buffer.data(), version.size);
        data_in.close();
    
        output.assign(buffer.begin(), buffer.end());
    
        std::cout << "Version (" << version.version_id << ") leida correctamente del archivo: " << filename << "\n";
        std::cout << "Usuario ID: " << version.user_id << "\n";
        std::cout << "Contenido: " << output << "\n";
    
        return true;
    }
    

    bool close(const std::string& filename) {
        std::cout << "Archivo cerrado: " << filename << std::endl;
        return true;
    }

    void showFileStatus(const std::string& filename) {
        std::string data_file = filename + ".data";
        std::string meta_file = filename + ".meta";
    
        std::ifstream meta_in(meta_file, std::ios::binary);
        if (!meta_in) {
            std::cerr << "No se pudo abrir el archivo .meta.\n";
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
    
        std::cout << "\nEstado del archivo versionado (" << filename << "):\n";
        std::cout << "Nombre: " << name << "\n";
        std::cout << "Total de versiones: " << total_versions << "\n\n";
    
        // Encabezado de tabla
        std::cout << "+-----------+------------+------------+----------+\n";
        std::cout << "| Version   | Offset     | Tamano     | User ID  |\n";
        std::cout << "+-----------+------------+------------+----------+\n";
    
        for (size_t i = 0; i < total_versions; ++i) {
            Version v;
            meta_in.read(reinterpret_cast<char*>(&v), sizeof(Version));
    
            // Formatear manualmente con espacios
            std::string version_str  = std::to_string(v.version_id);
            std::string offset_str   = std::to_string(v.offset);
            std::string size_str     = std::to_string(v.size);
            std::string user_id_str  = std::to_string(v.user_id);
    
            // Rellenar espacios para simular alineacion
            while (version_str.length() < 9) version_str += ' ';
            while (offset_str.length() < 10) offset_str += ' ';
            while (size_str.length() < 10) size_str += ' ';
            while (user_id_str.length() < 8) user_id_str += ' ';
    
            std::cout << "| " << version_str << "| "
                      << offset_str << "| "
                      << size_str << "| "
                      << user_id_str << "|\n";
        }
    
        std::cout << "+-----------+------------+------------+----------+\n";
    
        // Mostrar tamano total del archivo .data
        std::ifstream data_in(data_file, std::ios::binary | std::ios::ate);
        if (data_in) {
            size_t filesize = data_in.tellg();
            std::cout << "\nTamano total del archivo .data: " << filesize << " bytes\n";
        }
    
        meta_in.close();
    }
    
    
    bool copyFile(const std::string& src, const std::string& dst) {
        // Abrimos el archivo fuente en modo binario para lectura
        std::ifstream in(src, std::ios::binary);
        // Abrimos el archivo destino en modo binario para escritura
        std::ofstream out(dst, std::ios::binary);
    
        // Verificamos si ambos archivos se abrieron correctamente
        if (!in || !out) {
            std::cerr << " Error al copiar el archivo de " << src << " a " << dst << std::endl;
            return false;
        }
    
        // Copiamos todo el contenido del archivo fuente al archivo destino
        out << in.rdbuf();
    
        // Ambos archivos se cerrarán automáticamente al salir de la función (RAII)
        std::cout << "Archivo copiado correctamente de " << src << " a " << dst << std::endl;
        return true;
    }


}