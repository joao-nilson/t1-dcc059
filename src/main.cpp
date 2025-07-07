#include <iostream>
#include <iterator>
#include "Gerenciador.h"
#include "Grafo.h"

using namespace std;
int main(int argc, char *argv[])
{
    if (argc < 2){
        cerr << "Erro: Nenhum grafo especificado.\nUso: ./execGrupoX nome_do_arquivo.txt" << endl;
        return 1;
    }
    if (argc > 2){
        cerr << "Erro: Mais de um grafo ao mesmo tempo" << endl;
        return 1;
    }
    
    string nome_arquivo = argv[1];
    
    // Cria o grafo e já carrega o arquivo
    Grafo* grafo = new Grafo(nome_arquivo);

    grafo->imprimeInfo();

    // Chama os comandos passando o grafo já carregado
    Gerenciador::comandos(grafo);

    delete grafo;
    return 0;
}
