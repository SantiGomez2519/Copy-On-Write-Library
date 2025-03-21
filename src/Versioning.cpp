#include "../include/Versioning.h"
#include "../include/FileManager.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>  // Para crear la carpeta si no existe

// Constructor
Versioning::Versioning() : currentFile(""), versionCounter(0) {
    // Crear la carpeta 'storage/' si no existe
    struct stat info;
    if (stat("storage", &info) != 0) {
        std::cout << "Creando carpeta 'storage/'...\n";
        system("mkdir storage");
    }
}

void Versioning::create(const std::string& fileName) {
    // Verificar si la carpeta 'storage/' existe, si no, crearla
    struct stat info;
    if (stat("storage", &info) != 0) {
        std::cout << "Creando carpeta 'storage/'...\n";
        system("mkdir storage");
    }

    // Crear el archivo dentro de 'storage/'
    std::string fullPath = "storage/" + fileName;
    std::ofstream file(fullPath);
    if (file) {
        std::cout << "Archivo creado: " << fullPath << std::endl;
    } else {
        std::cerr << "Error al crear el archivo." << std::endl;
    }
}

void Versioning::open(const std::string& fileName) {
    std::string fullPath = "storage/" + fileName;
    std::ifstream file(fullPath);

    if (file) {
        currentFile = fullPath;
        std::cout << "Archivo abierto: " << fullPath << std::endl;
    } else {
        std::cerr << "Error: No se pudo abrir el archivo en " << fullPath << std::endl;
    }
}

// Escribir en el archivo dentro de 'storage/'
void Versioning::write(const std::string& data) {
    if (currentFile.empty()) {
        std::cerr << "No hay un archivo abierto." << std::endl;
        return;
    }

    std::ofstream file(currentFile, std::ios::app);
    if (file) {
        file << data << std::endl;
        std::cout << "Datos escritos en " << currentFile << std::endl;
    } else {
        std::cerr << "Error al escribir en el archivo." << std::endl;
    }
}

void Versioning::read() {
    if (currentFile.empty()) {
        std::cerr << "No hay un archivo abierto." << std::endl;
        return;
    }

    std::ifstream file(currentFile);
    if (!file) {
        std::cerr << "Error al leer el archivo." << std::endl;
        return;
    }

    std::string line;
    std::cout << "Contenido de " << currentFile << ":\n";
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }
}

void Versioning::close() {
    if (!currentFile.empty()) {
        std::cout << "Archivo cerrado: " << currentFile << std::endl;
        currentFile.clear();
    } else {
        std::cerr << "No hay archivo abierto." << std::endl;
    }
}

void Versioning::listVersions() {
    std::cout << "Listando versiones...\n";
    for (int i = 1; i <= versionCounter; i++) {
        std::cout << "Versión " << i << ": versions/version_" << i << ".txt\n";
    }
}

void Versioning::checkout(int versionNumber) {
    std::string versionPath = "versions/version_" + std::to_string(versionNumber) + ".txt";
    if (FileManager::copyFile(versionPath, "restored_file.txt")) {
        std::cout << "Versión " << versionNumber << " restaurada como 'restored_file.txt'." << std::endl;
    } else {
        std::cerr << "Error: No se encontró la versión " << versionNumber << "." << std::endl;
    }
}
