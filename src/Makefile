# Configurações do compilador
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O3
LDFLAGS := 

# Nome do executável final
TARGET := experiments

# Diretórios
SRC_DIR := .
BUILD_DIR := build

# Lista de arquivos fonte
SRCS := \
    DominacaoPerfeita.cpp \
    DominacaoPerfeitaExperimentos.cpp \
    Grafo.cpp \
    No.cpp \
    main.cpp

# Lista de cabeçalhos importantes
HEADERS := \
    Aresta.h \
    No.h \
    Grafo.h \
    DominacaoPerfeita.h \
    DominacaoPerfeitaExperimentos.h

# Gera lista de objetos no diretório build
OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# Regra padrão
all: $(BUILD_DIR) $(TARGET)

# Cria o diretório build se não existir
$(BUILD_DIR):
	@mkdir -p $@

# Regra para o executável final
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

# Regra genérica para compilar arquivos .cpp no diretório build
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Inclui dependências de cabeçalhos
-include $(DEPS)

# Limpeza
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean