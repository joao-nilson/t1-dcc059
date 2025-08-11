# Configurações do compilador
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O3
LDFLAGS := 

# Nome do executável final
TARGET := experiments

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

# Gera lista de objetos
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

# Regra padrão
all: $(TARGET)

# Regra para o executável final
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

# Regra genérica para compilar arquivos .cpp
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Inclui dependências de cabeçalhos
-include $(DEPS)

# Limpeza
clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)

.PHONY: all clean