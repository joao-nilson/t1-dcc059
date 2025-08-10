#ifndef DOMINACAO_PERFEITA_H
#define DOMINACAO_PERFEITA_H

#include "Grafo.h"
#include <unordered_map>
#include <random>

struct PDSResultado {
    std::vector<char> D;      // solução
    bool factivel = false;    // solução válida?
    int custo() const { return (int)D.size(); }
};

class DominacaoPerfeita {
public:
    // Verifica se D é um conjunto dominante perfeito (PDS)
    static bool verificaPDS(Grafo* G, const std::vector<char>& D);

    // (a) Guloso determinístico (empate por ID)
    static PDSResultado guloso(Grafo* G);

    // (b) Guloso Randomizado Adaptativo (GRASP)
    // iteracoes: nº de reinícios; alpha in [0,1] (0 = muito guloso; 1 = muito aleatório)
    static PDSResultado grasp(Grafo* G, int iteracoes, double alpha, unsigned seed = std::random_device{}());

    // (c) Guloso Randomizado Adaptativo Reativo
    // alphas: conjunto de alphas candidatos; iteracoes: total; bloco: tamanho do bloco para atualizar probabilidades
    static PDSResultado reativo(Grafo* G,
                                int iteracoes,
                                const std::vector<double>& alphas = {0.0, 0.25, 0.5, 0.75, 1.0},
                                int bloco = 10,
                                unsigned seed = std::random_device{}());

private:
    // Constrói uma solução PDS (ou falha) dado um gerador e modo de escolha
    // ‘alpha’ define o corte da RCL (min + alpha*(max-min)); se alpha<0 usa puro guloso
    static PDSResultado construcao(Grafo* G, std::mt19937& rng, double alpha);

    // Utilidades
    static std::vector<char> vizinhos(Grafo* G, char u);
    static std::unordered_map<char,int> grau(Grafo* G);
    static bool ehDirecionado(Grafo* G); // usamos apenas vizinhos "de saída" se direcionado
};

#endif
