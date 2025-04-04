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

    bool write(const std::string& filename) {
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

        // Escribir datos en el nuevo offset
        data_out.seekp(last_offset);
        data_out.write(current_data.data(), current_data.size());
        data_out.close();

        // Registrar la nueva versión con el offset correcto
        metadata.total_versions++;
        Version new_version;
        new_version.offset = last_offset;
        new_version.size = current_data.size();
        metadata.versions.push_back(new_version);

        // Guardar metadatos actualizados
        std::ofstream meta_out(metadata_file, std::ios::binary | std::ios::trunc);
        if (!meta_out) {
            std::cerr << "Error: No se pudo actualizar el archivo de metadatos.\n";
            return false;
        }

        // Escribir metadatos
        size_t name_length = metadata.filename.size();
        meta_out.write(reinterpret_cast<const char*>(&name_length), sizeof(size_t));
        meta_out.write(metadata.filename.c_str(), name_length);
        meta_out.write(reinterpret_cast<const char*>(&metadata.total_versions), sizeof(size_t));

        for (const auto& version : metadata.versions) {
            meta_out.write(reinterpret_cast<const char*>(&version), sizeof(Version));
        }

        meta_out.close();

        std::cout << "Nueva versión " << metadata.total_versions << " guardada correctamente.\n";
        return true;
    }

    bool read(const std::string& filename, size_t version_id, std::string& output) {
        FileMetadata metadata;
        if (!loadMetadata(filename, metadata)) {
            std::cerr << "No se pudo cargar la metadata para leer el archivo: " << filename << std::endl;
            return false;
        }

        if (version_id >= metadata.total_versions) {
            std::cerr << "Error: Versión no válida.\n";
            return false;
        }

        std::string data_file = filename + ".data";
        std::ifstream data_in(data_file, std::ios::binary);
        if (!data_in) {
            std::cerr << "Error al abrir el archivo de datos para lectura.\n";
            return false;
        }

        Version version = metadata.versions[version_id];
        output.resize(version.size);

        data_in.seekg(version.offset);
        data_in.read(&output[0], version.size);
        data_in.close();

        std::cout << "Lectura exitosa de la versión " << version_id << " del archivo: " << filename << std::endl;
        return true;
    }

    bool close(const std::string& filename) {
        std::cout << "Archivo cerrado: " << filename << std::endl;
        return true;
    }

}
