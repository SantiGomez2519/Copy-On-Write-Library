#include "./include/Versioning.h"

int main() {
    Versioning v;

    v.create("mi_archivo.txt");  // Crear un archivo
    v.open("mi_archivo.txt");    // Abrir el archivo
    v.write("Hola, Soy Santi haciendo pruebas!");  // Escribir en el archivo
    v.read();                    // Leer el contenido
    v.close();                   // Cerrar el archivo

    return 0;
}