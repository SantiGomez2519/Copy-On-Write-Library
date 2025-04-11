# Proyecto: Biblioteca de Gestión de Archivos Versionados basada en Copy-on-Write (COW)

**Autores:**  
_Juan Esteban Alzate_  
_Santiago Gómez Ospina_  
_Mateo Villada_  

Este documento describe en detalle la solución implementada para el proyecto de sistemas operativos, en el que se desarrolla una biblioteca que utiliza la técnica Copy-on-Write (COW) para gestionar versiones de archivos. A continuación se explica la arquitectura, las funcionalidades implementadas, las estructuras de datos utilizadas, el mecanismo de recolección de basura y las conclusiones derivadas.

---

## Contenidos

- [1. Introducción y Objetivos](#1-introducción-y-objetivos)
- [2. Arquitectura de la Solución](#2-arquitectura-de-la-solución)
- [3. Descripción de las Funcionalidades Implementadas](#3-descripción-de-las-funcionalidades-implementadas)
  - [3.1. Creación y Apertura de Archivos Versionados](#31-creación-y-apertura-de-archivos-versionados)
  - [3.2. Escritura de Cambios y Gestión del Versionado (Copy-on-Write)](#32-escritura-de-cambios-y-gestión-del-versionado-copy-on-write)
  - [3.3. Lectura de Versiones](#33-lectura-de-versiones)
  - [3.4. Visualización y Monitoreo](#34-visualización-y-monitoreo)
  - [3.5. Copia de Archivos](#35-copia-de-archivos)
  - [3.6. Recolección de Basura (Garbage Collector)](#36-recolección-de-basura-garbage-collector)
- [4. Estructuras de Datos](#4-estructuras-de-datos)
- [5. Desarrollo y Pruebas](#5-desarrollo-y-pruebas)
- [6. Conclusiones y Consideraciones Finales](#6-conclusiones-y-consideraciones-finales)

---

## 1. Introducción y Objetivos

El proyecto tiene como objetivo desarrollar una biblioteca que permita mantener un historial de versiones de un archivo sin duplicar todo su contenido en cada modificación. La técnica Copy-on-Write (COW) se utiliza para escribir los cambios únicamente cuando es necesario, optimizando así el uso de memoria y garantizando la integridad de los datos. Entre los principales objetivos se destacan:

- **Aplicar COW** para gestionar versiones de archivos, evitando duplicaciones innecesarias.
- **Mantener un historial** completo de los cambios, facilitando auditorías y recuperación de versiones previas.
- **Optimizar el uso de memoria** mediante técnicas de recolección de basura.
- **Desarrollar una API** con las funciones básicas: `create`, `open`, `write`, `read`, `readLatestVersion`, `close`, y funciones auxiliares de monitoreo y copia de archivos.

---

## 2. Arquitectura de la Solución

La solución se estructura en varios módulos, distribuidos en distintos archivos:

- **file_manager.h / file_manager.cpp:**  
  - Se definen las estructuras principales:  
    - `Version`: Registra el identificador de la versión, offset, tamaño del bloque y el `user_id` del autor del cambio.  
    - `DataBlock`: Destinado a representar bloques de datos modificados (aunque en este caso se integra en el manejo de versiones).  
    - `FileMetadata`: Almacena el nombre del archivo, la cantidad total de versiones y un vector con las instancias de `Version`.

  - Se implementan las funciones principales para manejar archivos versionados:
    - `create`: Crea un archivo de metadatos (archivo `.meta`) e inicializa la estructura `FileMetadata`.
    - `open`: Carga la metadata del archivo y verifica que exista.
    - `write`: Realiza la escritura basada en COW, grabando los cambios en el archivo de datos (`.data`) sin sobrescribir el contenido original y actualizando la metadata.
    - `read`: Permite la lectura de una versión específica, usando el offset y tamaño almacenados en la metadata.
    - `readLatestVersion`: Extrae el contenido de la última versión disponible.
    - Funciones para mostrar el estado del archivo y el uso de memoria.
    - `copyFile`: Copia archivos (por ejemplo, imágenes) para simular cambios que generan nuevas versiones.

- **storage.h / storage.cpp:**  
  - Se encargan de la persistencia de los metadatos en disco a través de:
    - `saveMetadata`: Guarda la metadata del archivo versionado en un archivo `.meta`.
    - `loadMetadata`: Carga la metadata desde disco.
    - `garbageCollector`: Implementa la recolección de basura, eliminando la versión más antigua y reasignando offsets en el archivo de datos para optimizar el uso del espacio.

- **main.cpp:**  
  - Demuestra el uso de la biblioteca.  
  - Se crean archivos versionados a partir de copias de imágenes, se generan múltiples versiones a través de sucesivas operaciones de escritura, y se leen versiones específicas para generar archivos de salida.
  - Se invocan funciones de monitoreo que muestran el estado del archivo y el uso de memoria.

---

## 3. Descripción de las Funcionalidades Implementadas

### 3.1. Creación y Apertura de Archivos Versionados

- **Función `create`:**
  - Crea un archivo de metadatos con extensión `.meta`, en el que se almacena el nombre del archivo original y se inicializa el número de versiones en 0.
  - Se escribe en disco la información inicial utilizando operaciones de escritura binaria.
  
- **Función `open`:**
  - Carga y muestra la metadata existente mediante la función `loadMetadata`.
  - Verifica la existencia del archivo y despliega el número total de versiones registradas.

### 3.2. Escritura de Cambios y Gestión del Versionado (Copy-on-Write)

- **Función `write`:**
  - Carga la metadata actual y abre el archivo original para leer el contenido en uso.
  - Calcula el offset a partir del último bloque escrito y realiza la escritura en el archivo `.data` en modo "append".
  - Crea una nueva entrada en la estructura `Version` que almacena:
    - `version_id`: Identificador de la nueva versión.
    - `offset`: Posición en el archivo de datos donde se escribió el contenido.
    - `size`: Tamaño del bloque de datos escrito.
    - `user_id`: Identificador del usuario que realizó el cambio.
  - Actualiza el vector de versiones y el total de versiones en `FileMetadata`.
  - Guarda la metadata actualizada en el archivo `.meta`.
  - **Ejecución del Garbage Collector:**  
    - Cuando el número total de versiones supera un umbral (en este caso, 5), se invoca el Garbage Collector para eliminar la versión más antigua y optimizar el uso de almacenamiento.

### 3.3. Lectura de Versiones

- **Función `read`:**
  - Permite leer una versión específica validando que el `version_id` solicitado exista.
  - Utiliza el offset y tamaño registrados para obtener el contenido desde el archivo `.data`.
  
- **Función `readLatestVersion`:**
  - Facilita la lectura de la última versión disponible, accediendo al último elemento del vector de versiones.

### 3.4. Visualización y Monitoreo

- **Funciones `showFileStatus` y `showFileStatusWithContent`:**
  - Muestran el estado actual del archivo versionado.
  - Se despliega el nombre del archivo, el total de versiones y, para cada versión, se imprime el `version_id`, offset, tamaño y `user_id`.
  - En `showFileStatusWithContent` se incluye también el contenido almacenado en cada bloque.
  
- **Función `showMemoryUsage`:**
  - Calcula y muestra el uso actual de memoria, tomando en cuenta tanto el archivo de datos (`.data`) como el de metadatos (`.meta`).

### 3.5. Copia de Archivos

- **Función `copyFile`:**
  - Se encarga de copiar un archivo desde una ruta fuente a una ruta destino, permitiendo simular cambios en el contenido del archivo versionado.

### 3.6. Recolección de Basura (Garbage Collector)

- **Función `garbageCollector`:**
  - Abre el archivo original de datos y un archivo temporal para reescribir las versiones que se desean conservar.
  - Elimina la versión más antigua (primer elemento del vector de versiones).
  - Copia las versiones restantes al archivo temporal, reasignando los offsets para reflejar la nueva posición de cada bloque.
  - Reemplaza el archivo original con el archivo temporal y actualiza la metadata en disco.
  - De esta forma se optimiza el uso de almacenamiento y se eliminan datos que ya no son necesarios.

---

## 4. Estructuras de Datos

- **Version:**
  - Representa cada versión del archivo.
  - Contiene:
    - `version_id`: Identificador secuencial de la versión.
    - `offset`: Posición en el archivo de datos donde inicia el bloque correspondiente.
    - `size`: Tamaño del bloque de datos.
    - `user_id`: Identificador del usuario autor del cambio.

- **DataBlock:**
  - Declarada para representar bloques de datos modificados, aunque la funcionalidad se integra en la manipulación de `Version`.

- **FileMetadata:**
  - Almacena la información general del archivo versionado:
    - `filename`: Nombre del archivo base.
    - `total_versions`: Número total de versiones existentes.
    - `versions`: Vector con las diferentes versiones registradas.

---

## 5. Desarrollo y Pruebas

El archivo **main.cpp** demuestra el uso de la biblioteca:

1. **Creación y Apertura:**
   - Se crea el archivo versionado y se abre para confirmar la carga de metadatos.

2. **Generación de Versiones:**
   - Se utilizan copias de imágenes (por ejemplo, "img1.jpg" y "img2.jpg") para simular cambios.
   - Se realizan múltiples operaciones de escritura para generar distintas versiones.
   - Cada operación de `write` registra una nueva versión, asignando los nuevos valores de offset, tamaño y `user_id`.

3. **Lectura y Exportación:**
   - Se leen versiones específicas (por ejemplo, la versión 0 y la versión 1) y se guardan en archivos de salida ("output_0.jpg" y "output_1.jpg").

4. **Monitoreo:**
   - Se invocan funciones para mostrar el estado del archivo versionado y el uso de memoria.
   - Se observa la ejecución del Garbage Collector cuando el número de versiones excede el límite establecido, verificándose el estado del archivo antes y después de la recolección.

---

## 6. Conclusiones y Consideraciones Finales

La implementación de esta biblioteca basada en COW demuestra la viabilidad de mantener un historial de versiones en archivos sin duplicar por completo su contenido, mejorando la eficiencia en el uso de memoria. Entre las conclusiones principales se destacan:

- **Optimización en la escritura:**  
  Se reducen duplicaciones al escribir solo los bloques modificados, lo que es fundamental para la eficiencia en sistemas con gran cantidad de cambios.

- **Gestión del historial:**  
  La estructura de metadatos permite recuperar versiones anteriores, lo que resulta esencial para auditorías, recuperación de datos y colaboración.

- **Recolección de basura:**  
  La función del Garbage Collector optimiza el espacio de almacenamiento al eliminar las versiones más antiguas y reasignar nuevos offsets.

- **Integración de conceptos de Sistemas Operativos:**  
  Se aplican técnicas de virtualización de memoria y manejo de archivos binarios, demostrando el aprovechamiento de conceptos vistos en clase.

# 📂 Sistema de Archivos Versionados en C++

Este proyecto implementa un sistema de control de versiones para archivos de texto plano, inspirado en conceptos básicos de Git. Permite crear, editar, leer y gestionar versiones de archivos de forma sencilla.

---

## 🧠 ¿Cómo funciona?

- Cada archivo versionado tiene dos archivos asociados:
  - `.data`: contiene el contenido de todas las versiones
  - `.meta`: contiene los metadatos como versiones, offsets, tamaños y `user_id`

- Las versiones son gestionadas automáticamente:
  - Cada `write()` guarda una nueva versión con el `user_id` del autor
  - Máximo 5 versiones por archivo (las más antiguas se eliminan automáticamente)

- Solo puedes trabajar con **un archivo abierto a la vez**

---

## ⚙️ Instrucciones de uso

### 1. Crear un nuevo archivo versionado

VersionedStorage::create("mi_archivo");
Crea los archivos mi_archivo.meta y mi_archivo.data

### 2. Abrir un archivo existente

VersionedStorage::open("mi_archivo");
Debes cerrar el archivo actual con close() antes de abrir otro

### 3. Escribir una nueva versión

VersionedStorage::write("mi_archivo", user_id);

Acciones:

  Guarda el contenido actual como nueva versión

  Asocia el user_id como autor

  Ejecuta GC si hay más de 5 versiones

### 4. Leer la última versión

std::string contenido;
VersionedStorage::readLatestVersion("mi_archivo", contenido);

### 5. Leer versión específica

std::string contenido;
VersionedStorage::read("mi_archivo", version_id, contenido);
version_id comienza en 0 (versión más antigua)

### 6. Ver estado del archivo

// Solo metadatos
VersionedStorage::showFileStatus("mi_archivo");

// Con contenido
VersionedStorage::showFileStatusWithContent("mi_archivo");

### 7. Uso de memoria

VersionedStorage::showMemoryUsage("mi_archivo");

### 8. Copiar archivo

VersionedStorage::copyFile("mi_archivo", "mi_copia");

9. Cerrar archivo

VersionedStorage::close("mi_archivo");

🚫 Restricciones
✔️ Solo 1 archivo abierto a la vez
✔️ Máximo 5 versiones por archivo
✔️ Requiere open() antes de write()
✔️ Solo para archivos de texto (.txt)

✅ Requisitos
C++11 o superior

Compilador compatible con STL (g++, clang++, etc.)

Bibliotecas: <filesystem>, <fstream>, <vector>, <map>
  

## Para archivos binarios o que no sean .txt (por ejemplo imágenes o PDFs):
copyFile también puede utilizarse para inicializar archivos versionados con contenido que no sea texto plano, como imágenes. En este caso, se recomienda utilizar una estructura como la siguiente:

  std::string base = "image_test";
  std::string extension = ".jpg";
  std::string filename = base + extension;
  
  // Crear y abrir el archivo versionado
  VersionedStorage::create(filename);
  VersionedStorage::open(filename);
  
  // Primera versión: guardar el contenido de una imagen original
  VersionedStorage::copyFile("img1.jpg", filename); //siendo img1.jpg una imagen dentro de la carpeta donde estemos trabajando
  VersionedStorage::write(filename, 0);

Esto guarda el contenido de img1.jpg dentro del archivo versionado image_test.jpg, lo cual será tratado como una versión binaria dentro del sistema.
