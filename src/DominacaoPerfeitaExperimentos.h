#ifndef DOMINACAO_PERFEITA_EXPERIMENTOS_H
#define DOMINACAO_PERFEITA_EXPERIMENTOS_H

#include "DominacaoPerfeita.h"
#include <string>
#include <vector>
#include <fstream>

//enum class AlgorithmType { GREEDY, GRASP, REACTIVE };


struct ExperimentResult {
    std::string filename;
    std::string group;
    int vertices;
    int edges;
    double density;
    bool directed;
    bool edge_weighted;
    bool vertex_weighted;
    int greedy_size;
    int grasp_size;
    int reactive_size;
    double greedy_time;
    double grasp_time;
    double reactive_time;
};

class DominacaoPerfeitaExperimentos {
public:
    static void run_experiments(const std::vector<std::string>& filenames, 
                                const std::vector<std::string>& groups,
                                const std::string& output_csv = "results.csv");
    static ExperimentResult run_single_experiment(Grafo* G, const std::string& filename, const std::string& group);
    static void parse_filename(const std::string& full_path, ExperimentResult& res);
    static std::pair<int, double> run_algorithm(Grafo* G, int type);
    static void write_result(std::ofstream& out, const ExperimentResult& res);
    void write_iteration_csv(const IterationData& data, const std::string& filename);
};


#endif // DOMINACAO_PERFEITA_EXPERIMENTOS_H