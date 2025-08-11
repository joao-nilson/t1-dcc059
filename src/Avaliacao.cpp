#include "Grafo.h"
#include <chrono>
#include <fstream>
#include <functional>
#include "DominacaoPerfeita.h"


struct EvaluationResult {
    double avg_time;
    double avg_size;
    double success_rate;
};

EvaluationResult evaluate_algorithm(
    Grafo* G, 
    std::function<PDSResultado(Grafo*)> algorithm, 
    int trials = 10) 
{
    EvaluationResult res = {0, 0, 0};
    int successes = 0;
    int total_size = 0;
    
    for (int i = 0; i < trials; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        auto result = algorithm(G);
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> elapsed = end - start;
        res.avg_time += elapsed.count();
        
        if (result.factivel) {
            successes++;
            total_size += result.custo();
        }
    }
    
    res.avg_time /= trials;
    res.success_rate = (double)successes / trials;
    res.avg_size = successes > 0 ? (double)total_size / successes : 0;
    
    return res;
}

void run_evaluations(const std::vector<Grafo*>& test_graphs) {
    std::ofstream out("resultados.csv");
    out << "Grafo,Vertices,Arestas,Algoritmo,Tempo,Tamanho,TaxaSucesso\n";
    
    for (Grafo* G : test_graphs) {
        int n = G->get_lista_adj().size();
        int m = 0;
        for (No* no : G->get_lista_adj()) {
            m += G->get_vizinhanca(no->get_id()).size();
        }
        if (G->get_direcionado()) m /= 2;
        
        // Testa guloso deterministico
        auto res_greedy = evaluate_algorithm(G, [](Grafo* g) { 
            return DominacaoPerfeita::guloso(g); 
        });
        out << "G" << n << "," << n << "," << m << ",Guloso,"
            << res_greedy.avg_time << "," << res_greedy.avg_size << "," 
            << res_greedy.success_rate << "\n";
            
        // Testa GRASP com alfas diferentes
        for (double alpha : {0.0, 0.25, 0.5, 0.75, 1.0}) {
            auto res_grasp = evaluate_algorithm(G, [alpha](Grafo* g) { 
                return DominacaoPerfeita::grasp(g, 100, alpha); 
            });
            out << "G" << n << "," << n << "," << m << ",GRASP-alpha" << alpha << ","
                << res_grasp.avg_time << "," << res_grasp.avg_size << "," 
                << res_grasp.success_rate << "\n";
        }
        
        // testa GRASP reativo
        auto res_reativo = evaluate_algorithm(G, [](Grafo* g) { 
            return DominacaoPerfeita::reativo(g, 100); 
        });
        out << "G" << n << "," << n << "," << m << ",Reativo,"
            << res_reativo.avg_time << "," << res_reativo.avg_size << "," 
            << res_reativo.success_rate << "\n";
    }
}