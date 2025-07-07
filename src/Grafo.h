//
// Created by Rafael on 28/05/2025.
//

#ifndef GRAFO_H
#define GRAFO_H

#include "No.h"
#include <iostream>
#include <set>
#include <vector>
#include <unordered_map>

using namespace std;
class Grafo {
public:
    Grafo();                      // Construtor padrão
    Grafo(const string& grafo);   // Construtor que carrega o grafo a partir do arquivo
    
    ~Grafo();

    void carregaArquivo(const string& grafo);

    void aux_dfs(No* no, set<char>& visitado);
    vector<char> fecho_transitivo_direto(char id_no); // a
    vector<char> fecho_transitivo_indireto(char id_no); // b
    vector<char> caminho_minimo_dijkstra(char id_no_a, char id_no_b); // c
    vector<char> caminho_minimo_floyd(char id_no, char id_no_b); // d

    Grafo* arvore_geradora_minima_prim(vector<char> ids_nos); // e
    Grafo* arvore_geradora_minima_kruskal(vector<char> ids_nos); // f

    Grafo* arvore_caminhamento_profundidade(char id_no); // g
    int raio(); // h 1
    int diametro(); // h 2
    vector<char> centro(); // h 3
    vector<char> periferia(); // h 4
    vector<char> vertices_de_articulacao(); // i

    void adiciona_no(No* no) { lista_adj.push_back(no); };
    void imprimeInfo() const;

    //getters
    int get_ordem(){return ordem;};
    bool get_direcionado(){return in_direcionado;};
    bool get_ponderado_aresta(){return in_ponderado_aresta;};
    bool get_ponderado_vertice()const{return in_ponderado_vertice;};
    vector<No*> get_lista_adj(){return lista_adj;}
    vector<Aresta*> get_vizinhanca(char id);
    vector<char> get_ids_vertices();
    No* getBuscaNo(char id);

    //setters
    void set_ordem(int new_ordem){ordem = new_ordem;};
    void set_direcionado(bool direcionado){in_direcionado = direcionado;};
    void set_ponderado_aresta(bool pon_aresta){in_ponderado_aresta = pon_aresta;};
    void set_ponderado_vertice(bool pon_vertice){in_ponderado_vertice = pon_vertice;};
    void set_lista_adj(vector<No*> new_list){lista_adj = new_list;};


    
private:
    int ordem;
    bool in_direcionado;
    bool in_ponderado_aresta;
    bool in_ponderado_vertice;
    vector<No*> lista_adj;
    void aux_dfs_reverso(char id_atual, set<char>& visitado, const unordered_map<char, vector<No*>>& lista_adjacencia_inversa);

};



#endif //GRAFO_H
