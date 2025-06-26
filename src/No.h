#ifndef NO_H
#define NO_H

#include <string>
#include <vector>
#include "Aresta.h"


using namespace std;
class No {
    public:
        No(char id, int peso) : id(id), peso(peso){} //vertice ponderado
        No(char id) : id(id){} //vertice não ponderado
        ~No();

        // Getters
        char get_id() const {return id;};
        int get_peso() const {return peso;};
        vector<Aresta*> get_arestas() const {return arestas;};
        vector<No*> get_vizinhos() const {return vizinhos;};

        // Setters
        void set_peso(int novo_peso) {peso = novo_peso;};

    private:
        char id;
        int peso;
        vector<Aresta*> arestas;
        vector<No*> vizinhos;
};


#endif //NO_H
