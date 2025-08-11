#include <iostream>
#include <iterator>
#include "Gerenciador.h"
#include "Grafo.h"

#include <filesystem>
#include "DominacaoPerfeitaExperimentos.h"
namespace fs = std::filesystem;

int main() {
    std::vector<std::string> all_graph_files;
    
    const std::string directories[] = {
        "instancias_t1",
        "instancias_t2"
    };
    
    for (const auto& dir : directories) {
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                all_graph_files.push_back(entry.path().string());
            }
        }
    }
    
    DominacaoPerfeitaExperimentos::run_experiments(all_graph_files);
    return 0;
}

// using namespace std;
// int main(int argc, char *argv[])
// {
//     if (argc < 2){
//         cerr << "Erro: Nenhum grafo especificado.\nUso: ./execGrupoX nome_do_arquivo.txt" << endl;
//         return 1;
//     }
//     if (argc > 2){
//         cerr << "Erro: Mais de um grafo ao mesmo tempo" << endl;
//         return 1;
//     }
    
//     string nome_arquivo = argv[1];
    
//     // Cria o grafo e já carrega o arquivo
//     Grafo* grafo = new Grafo(nome_arquivo);

//     grafo->imprimeInfo();

//     // Chama os comandos passando o grafo já carregado
//     Gerenciador::comandos(grafo);

//     delete grafo;
//     return 0;
// }
