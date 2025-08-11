#ifndef DOMINACAO_PERFEITA_H
#define DOMINACAO_PERFEITA_H

#include "Grafo.h"
#include <unordered_map>
#include <random>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <chrono>
#include <queue>
#include <fstream>
#include <functional>

struct PDSResultado {
    std::vector<char> D;      // solução
    bool factivel = false;    // solução válida?
    int custo() const { return (int)D.size(); }
};

struct IterationTracker {
    std::vector<int> iterations;
    std::vector<double> qualities; // 1/set_size
    
    void record(int iter, const PDSResultado& result) {
        iterations.push_back(iter);
        qualities.push_back(result.factivel ? 1.0 / result.custo() : 0);

        // if (result.factivel) {
        //     qualities.push_back(1.0 / result.custo());
        // } else {
        //     qualities.push_back(0); // ou marcador invalido
        // }
    }
};

struct EvaluationResult {
    double avg_time;
    double avg_size;
    double success_rate;
};

struct RuntimeData {
    std::vector<int> graph_sizes;
    std::vector<double> greedy_times;
    std::vector<double> grasp_times;
    std::vector<double> reactive_times;
};

struct IterationData {
    std::vector<int> iterations;
    std::vector<double> greedy_qualities;
    std::vector<double> grasp_qualities;
    std::vector<double> reactive_qualities;
};

struct AlphaPerformance {
    std::vector<double> alpha_values;
    std::vector<double> solution_qualities;
    std::vector<double> runtimes;
};

class DominacaoPerfeita {
    friend PDSResultado grasp_with_tracking(Grafo*, int, double, IterationTracker&, unsigned);
    friend PDSResultado reativo_with_tracking(Grafo*, int, const std::vector<double>&, int, IterationTracker&, unsigned);

public:
    // Verifica se D é um conjunto dominante perfeito (PDS)
    static bool verificaPDS(Grafo* G, const std::vector<char>& D);

    // (a) Guloso determinístico (empate por ID)
    static PDSResultado guloso(Grafo* G, IterationTracker* tracker = nullptr);

    // (b) Guloso Randomizado Adaptativo (GRASP)
    // iteracoes: nº de reinícios; alpha in [0,1] (0 = muito guloso; 1 = muito aleatório)
    static PDSResultado grasp(Grafo* G, int iteracoes, double alpha, 
                              IterationTracker* tracker = nullptr ,
                              unsigned seed = std::random_device{}());

    // (c) Guloso Randomizado Adaptativo Reativo
    // alphas: conjunto de alphas candidatos; iteracoes: total; bloco: tamanho do bloco para atualizar probabilidades
    static PDSResultado reativo(Grafo* G,
                                int iteracoes,
                                const std::vector<double>& alphas = {0.0, 0.25, 0.5, 0.75, 1.0},
                                int bloco = 10, IterationTracker* tracker = nullptr ,
                                unsigned seed = std::random_device{}());

                                

    // Constrói uma solução PDS (ou falha) dado um gerador e modo de escolha
    // ‘alpha’ define o corte da RCL (min + alpha*(max-min)); se alpha<0 usa puro guloso
    
    // static PDSResultado construcao(Grafo* G, std::mt19937& rng, double alpha);
    private:
    static PDSResultado construcao(Grafo* G, std::mt19937& rng, double alpha);

    friend PDSResultado grasp_with_tracking(Grafo*, int, double, IterationTracker&, unsigned);
    void run_quality_experiment(Grafo* G);
    void write_iteration_csv(const IterationData& data, const std::string& filename);
    void write_runtime_csv(const RuntimeData& data, const std::string& filename);
    void write_alpha_csv(const AlphaPerformance& data, const std::string& filename);
    void run_reactive_experiment(Grafo* G);
    void run_alpha_sensitivity();
    
    

    // Utilidades
    static std::vector<char> vizinhos(Grafo* G, char u);
    static std::unordered_map<char,int> grau(Grafo* G);
    static bool ehDirecionado(Grafo* G); // usamos apenas vizinhos "de saída" se direcionado
};

#endif
