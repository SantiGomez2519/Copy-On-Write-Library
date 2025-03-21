#include "Versioning.h"
#include <iostream>
#include <cassert>
#include <fstream>

void test_create() {
    Versioning v;
    v.create("test_create.txt");

    std::ifstream file("storage/test_create.txt");
    if (!file.good()) {
        std::cerr << "❌ ERROR: El archivo no se creó correctamente en storage/.\n";
    } else {
        std::cout << "✅ Archivo detectado en storage/.\n";
    }

    assert(file.good());  // Verifica que el archivo existe
    std::cout << "✅ test_create PASSED\n";
}

void test_open() {
    Versioning v;
    v.create("test_open.txt");
    v.open("test_open.txt");

    std::string fullPath = "storage/test_open.txt";
    std::ifstream file(fullPath);

    if (!file.good()) {
        std::cerr << "❌ ERROR: No se encontró el archivo en " << fullPath << std::endl;
    } else {
        std::cout << "✅ Archivo detectado en " << fullPath << std::endl;
    }

    assert(file.good());  // Verifica que el archivo existe
    std::cout << "✅ test_open PASSED\n";
}

void test_write() {
    Versioning v;
    v.create("test_write.txt");
    v.open("test_write.txt");
    v.write("Hola, mundo!");
    v.close();

    std::ifstream file("storage/test_write.txt");
    std::string line;
    std::getline(file, line);

    std::cout << "📖 Contenido leído: '" << line << "'" << std::endl;

    assert(line == "Hola, mundo!");  // Verifica que se escribió correctamente
    std::cout << "✅ test_write PASSED\n";
}

void test_read() {
    Versioning v;
    v.create("test_read.txt");
    v.open("test_read.txt");
    v.write("Texto de prueba.");
    v.close();

    v.open("test_read.txt");
    v.read();
    v.close();
    
    std::cout << "✅ test_read PASSED\n";
}

void test_close() {
    Versioning v;
    v.create("test_close.txt");
    v.open("test_close.txt");
    v.close();
    
    std::cout << "✅ test_close PASSED\n";
}

int main() {
    test_create();
    test_open();
    test_write();
    test_read();
    test_close();
    std::cout << "🎉 TODAS LAS PRUEBAS PASARON 🎉\n";
    return 0;
}

