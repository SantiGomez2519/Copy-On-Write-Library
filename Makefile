# Nombre del compilador
CC = g++

# Opciones de compilación
CFLAGS = -Iinclude -Wall

# Archivos fuente y objetos
SRC = src/Versioning.cpp src/FileManager.cpp
OBJ = Versioning.o FileManager.o

# Nombre de la biblioteca
LIBRARY = libversioning.a

# Archivos de prueba
TEST_SRC = tests/test_functions.cpp
TEST_EXE = test_functions

# Carpeta de almacenamiento
STORAGE_DIR = storage
VERSIONS_DIR = versions

# Comando por defecto: Compilar todo
all: $(LIBRARY) $(TEST_EXE)

# Compilar los archivos fuente en objetos
%.o: src/%.cpp include/%.h
	$(CC) -c $< -o $@ $(CFLAGS)

# Crear la biblioteca estática
$(LIBRARY): $(OBJ)
	ar rcs $(LIBRARY) $(OBJ)

# Compilar el ejecutable de pruebas
$(TEST_EXE): $(TEST_SRC) $(LIBRARY)
	$(CC) $(TEST_SRC) -L. -lversioning $(CFLAGS) -o $(TEST_EXE)

# Crear los directorios si no existen
init:
	mkdir -p $(STORAGE_DIR)
	mkdir -p $(VERSIONS_DIR)

# Limpiar archivos compilados y ejecutables
clean:
	rm -f *.o $(LIBRARY) $(TEST_EXE)

# Eliminar todo, incluyendo archivos de almacenamiento y versiones
clean-all: clean
	rm -rf $(STORAGE_DIR) $(VERSIONS_DIR)

