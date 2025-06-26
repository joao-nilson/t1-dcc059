//
// Created by Rafael on 28/05/2025.
//

#ifndef GRAFO_H
#define GRAFO_H

#include "No.h"
#include <iostream>
#include <set>
#include <vector>


using namespace std;
class Grafo {
public:
    Grafo();
    ~Grafo();

    void carregaArquivo(const string& grafo);
    void aux_dfs(No* no, set<char>& visitado);
    vector<char> fecho_transitivo_direto(int id_no); // a
    vector<char> fecho_transitivo_indireto(int id_no); // b
    vector<char> caminho_minimo_dijkstra(int id_no_a, int id_no_b); // c
    vector<char> caminho_minimo_floyd(int id_no, int id_no_b); // d
    Grafo* arvore_geradora_minima_prim(vector<char> ids_nos); // e
    Grafo* arvore_geradora_minima_kruskal(vector<char> ids_nos); // f
    Grafo* arvore_caminhamento_profundidade(int id_no); // g
    int raio(); // h 1
    int diametro(); // h 2
    vector<char> centro(); // h 3
    vector<char> periferia(); // h 4
    vector<char> vertices_de_articulacao(); // i

    int get_ordem(){return ordem;};
    bool get_in_direcionado(){return in_direcionado;};
    bool get_in_ponderado_aresta(){return in_ponderado_aresta;};
    bool get_in_ponderado_vertice(){return in_ponderado_vertice;};
    vector<No*> get_lista_adj(){return lista_adj;}
    
private:
    int ordem;
    bool in_direcionado;
    bool in_ponderado_aresta;
    bool in_ponderado_vertice;
    vector<No*> lista_adj;
};



#endif //GRAFO_H
