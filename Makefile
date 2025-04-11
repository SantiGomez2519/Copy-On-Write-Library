# Directorios
INCLUDE_DIR = include
SRC_DIR = src
BUILD_DIR = build
LIB_NAME = libcow.a

# Archivos fuente
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# Comando de compilación
CXX = g++
CXXFLAGS = -std=c++17 -I$(INCLUDE_DIR) -Wall -Wextra

# Target por defecto
all: $(LIB_NAME) main

# Compilar objetos
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Crear el directorio de build si no existe
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Crear librería estática
$(LIB_NAME): $(OBJS)
	ar rcs $@ $^

# Compilar ejecutable principal
main: test/main.cpp $(LIB_NAME)
	$(CXX) $(CXXFLAGS) test/main.cpp -L. -lcow -o main

# Limpiar archivos generados
clean:
	rm -rf $(BUILD_DIR) *.a main

.PHONY: all clean

