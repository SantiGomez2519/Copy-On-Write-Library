#include "file_manager.h"

int main() {
    std::string base = "image_test";
    std::string extension = ".jpg";
    std::string filename = base + extension;

    // Crear y abrir el archivo versionado
    VersionedStorage::create(filename);
    VersionedStorage::open(filename);

    // Primera version: guardar img1.jpg
    VersionedStorage::copyFile("img1.jpg", filename);
    VersionedStorage::write(filename, 0);

    // Segunda version: guardar img2.jpg (sobrescribir)
    VersionedStorage::copyFile("img2.jpg", filename);
    VersionedStorage::write(filename, 1);
    VersionedStorage::copyFile("img2.jpg", filename);
    VersionedStorage::write(filename, 1);
    VersionedStorage::copyFile("img2.jpg", filename);
    VersionedStorage::write(filename, 1);
    VersionedStorage::copyFile("img2.jpg", filename);
    VersionedStorage::write(filename, 1);
    VersionedStorage::copyFile("img2.jpg", filename);
    VersionedStorage::write(filename, 1);
    

    // Leer versión 0 y guardar como output_0.jpg
    std::string out0;
    if (VersionedStorage::read(filename, 0, out0)) {
        std::ofstream out("output_0.jpg", std::ios::binary);
        out.write(out0.c_str(), out0.size());
        std::cout << "Version 0 guardada como output_0.jpg\n";
    }

    // Leer versión 1 y guardar como output_1.jpg
    std::string out1;
    if (VersionedStorage::read(filename, 1, out1)) {
        std::ofstream out("output_1.jpg", std::ios::binary);
        out.write(out1.c_str(), out1.size());
        std::cout << "Version 1 guardada como output_1.jpg\n";
    }

    std::string latest;
    // VersionedStorage::readLatestVersion(filename, latest);
    VersionedStorage::showFileStatus(filename);
    VersionedStorage::showMemoryUsage(filename);

    VersionedStorage::close(filename);
    return 0;
}
