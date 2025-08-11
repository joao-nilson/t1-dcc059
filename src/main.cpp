#include <filesystem>
#include <vector>
#include <iostream>
#include <map>
#include "DominacaoPerfeitaExperimentos.h"

namespace fs = std::filesystem;

int main() {
    std::map<std::string, std::vector<std::string>> grouped_files;
    
    // Diretórios com as instâncias
    const std::string t1_path = "instancias_t1";
    const std::string t2_path = "instancias_t2";
    
    // Função para adicionar arquivos de um diretório
    auto add_files_from_dir = [&](const std::string& path, const std::string& group) {
        if (!fs::exists(path)) {
            std::cerr << "Aviso: Diretório não encontrado - " << path << std::endl;
            return;
        }
        
        try {
            for (const auto& entry : fs::directory_iterator(path)) {
                if (entry.path().extension() == ".txt") {
                    grouped_files[group].push_back(entry.path().string());
                    std::cout << "Adicionado: " << entry.path().filename().string() 
                              << " (Grupo: " << group << ")" << std::endl;
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Erro ao acessar " << path << ": " << e.what() << std::endl;
        }
    };
    
    // Coletar arquivos dos diretórios
    std::cout << "Coletando arquivos de instâncias..." << std::endl;
    add_files_from_dir(t1_path, "t1");
    add_files_from_dir(t2_path, "t2");
    
    if (grouped_files.empty()) {
        std::cerr << "Erro: Nenhum arquivo de instância encontrado!" << std::endl;
        std::cerr << "Certifique-se que os diretórios existem e contêm arquivos .txt:" << std::endl;
        std::cerr << " - " << fs::absolute(t1_path) << std::endl;
        std::cerr << " - " << fs::absolute(t2_path) << std::endl;
        return 1;
    }
    
    // Preparar lista de arquivos mantendo a informação do grupo
    std::vector<std::string> filenames;
    std::vector<std::string> groups;
    
    for (const auto& [group, files] : grouped_files) {
        for (const auto& file : files) {
            filenames.push_back(file);
            groups.push_back(group);
        }
    }
    
    std::cout << "\nIniciando experimentos com " << filenames.size() << " arquivos..." << std::endl;
    DominacaoPerfeitaExperimentos::run_experiments(filenames, groups);
    
    std::cout << "\nExperimentos concluídos com sucesso!" << std::endl;
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
