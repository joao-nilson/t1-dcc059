#include "DominacaoPerfeitaExperimentos.h"
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

void DominacaoPerfeitaExperimentos::run_experiments(
    const std::vector<std::string>& filenames, 
    const std::string& output_csv) {
    
    std::ofstream out(output_csv);
    out << "Grafo,Vertices,Arestas,Densidade,Direcao,PesoAresta,PesoVertice,"
        << "TamanhoGuloso,TamanhoGRASP,TamanhoReativo,TempoGuloso,TempoGRASP,TempoReativo\n";
    
    for (const auto& filename : filenames) {
        Grafo* G = new Grafo();
        G->carregaArquivo(filename);
        
        ExperimentResult res = run_single_experiment(G, filename);
        write_result(out, res);
        
        delete G;
    }
}

ExperimentResult DominacaoPerfeitaExperimentos::run_single_experiment(Grafo* G, const std::string& filename) {
    
    ExperimentResult result;
    parse_filename(filename, result);
    
    // pega propriedades basicas
    result.vertices = G->get_ordem();
    result.edges = G->get_num_arestas();

    // roda e mede tempo dos algoritmos
    auto greedy = run_algorithm(G, 0); // 0 = Guloso
    auto grasp = run_algorithm(G, 1);  // 1 = GRASP
    auto reactive = run_algorithm(G, 2); // 2 = Reativo
    
    result.greedy_size = greedy.first;
    result.greedy_time = greedy.second;
    result.grasp_size = grasp.first;
    result.grasp_time = grasp.second;
    result.reactive_size = reactive.first;
    result.reactive_time = reactive.second;
    
    return result;
}

std::pair<int, double> DominacaoPerfeitaExperimentos::run_algorithm(
    Grafo* G, int type) {
    
    auto start = std::chrono::high_resolution_clock::now();
    PDSResultado result;
    
    switch(type) {
        case 0: // Guloso
            result = DominacaoPerfeita::guloso(G);
            break;
        case 1: // GRASP
            result = DominacaoPerfeita::grasp(G, 100, 0.5);
            break;
        case 2: // Reativo
            result = DominacaoPerfeita::reativo(G, 100);
            break;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    
    return {result.factivel ? result.custo() : -1, elapsed};
}



void DominacaoPerfeitaExperimentos::parse_filename(
    const std::string& full_path, ExperimentResult& res) 
{
    fs::path p(full_path);
    std::string filename = p.filename().string();
    res.filename = filename;

    
    // Formato: g_<order>_<density>_<directed>_<edge_weight>_<vertex_weight>_<#>.txt
    std::string stripped = filename.substr(2); // Remove 'g_'
    std::replace(stripped.begin(), stripped.end(), '_', ' ');
    std::istringstream iss(stripped);
    
    int order, directed, edge_weight, vertex_weight;
    double density;
    iss >> order >> density >> directed >> edge_weight >> vertex_weight;
    
    res.density = density;
    res.directed = (directed == 1);
    res.edge_weighted = (edge_weight == 1);
    res.vertex_weighted = (vertex_weight == 1);
}

void DominacaoPerfeitaExperimentos::write_result(std::ofstream& out, const ExperimentResult& res) {
    out << res.filename << ","
        << res.vertices << ","
        << res.edges << ","
        << res.density << ","
        << (res.directed ? 1 : 0) << ","
        << (res.edge_weighted ? 1 : 0) << ","
        << (res.vertex_weighted ? 1 : 0) << ","
        << res.greedy_size << ","
        << res.grasp_size << ","
        << res.reactive_size << ","
        << res.greedy_time << ","
        << res.grasp_time << ","
        << res.reactive_time << "\n";
}

void DominacaoPerfeitaExperimentos::write_iteration_csv(const IterationData& data, const std::string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error opening file: " << filename << "\n";
        return;
    }
    
    out << "Iteracar,QualidadeGuloso,QualidadeGRASP,QualidadeReactivo\n";
    for (size_t i = 0; i < data.iterations.size(); i++) {
        out << data.iterations[i] << ","
            << data.greedy_qualities[i] << ","
            << data.grasp_qualities[i] << ","
            << data.reactive_qualities[i] << "\n";
    }
}



// int main() {
//     std::vector<std::string> all_graph_files;
    
//     // Process instancias_t1 directory
//     const std::string t1_dir = "instancias_t1";
//     for (const auto& entry : fs::directory_iterator(t1_dir)) {
//         if (entry.is_regular_file() && entry.path().extension() == ".txt") {
//             all_graph_files.push_back(entry.path().string());
//         }
//     }
    
//     // Process instancias_t2 directory
//     const std::string t2_dir = "instancias_t2";
//     for (const auto& entry : fs::directory_iterator(t2_dir)) {
//         if (entry.is_regular_file() && entry.path().extension() == ".txt") {
//             all_graph_files.push_back(entry.path().string());
//         }
//     }
    
//     // Run experiments on all collected files
//     DominacaoPerfeitaExperimentos::run_experiments(all_graph_files);
    
//     return 0;
// }
