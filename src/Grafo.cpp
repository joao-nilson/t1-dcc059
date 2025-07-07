#include "Grafo.h"
#include "No.h"
#include <cstdlib>
#include <set>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <climits>
#include <algorithm>

using namespace std;

// Construtor padrão do grafo
Grafo::Grafo()
{
    ordem = 0;
    in_direcionado = false;
    in_ponderado_aresta = false;
    in_ponderado_vertice = false;
    lista_adj = vector<No *>();
}

// Construtor que recebe o nome do arquivo e carrega o grafo
Grafo::Grafo(const string &nome_arquivo) : Grafo()
{
    carregaArquivo(nome_arquivo);
}

// Carrega o grafo a partir de um arquivo
void Grafo::carregaArquivo(const string &grafo)
{
    string caminho = "../instancias/" + grafo;
    cout << "Tentando abrir arquivo em: " << caminho << endl;

    ifstream arquivo(caminho);
    string line;

    if (!arquivo.is_open())
    {
        cerr << "Erro ao abrir arquivo: " << grafo << endl;
        exit(1);
    }

    // Lê as configurações iniciais do grafo
    getline(arquivo, line);
    if (line.length() < 5)
    {
        cout << "Erro: formato inválido da linha de configuração." << endl;
        exit(1);
    }
    set_direcionado(line.at(0) == '1');
    set_ponderado_aresta(line.at(2) == '1');
    set_ponderado_vertice(line.at(4) == '1');

    // Lê a ordem do grafo (número de vértices)
    getline(arquivo, line);
    ordem = stoi(line);

    // Lê os vértices
    for (int i = 0; i < ordem; ++i)
    {
        getline(arquivo, line);
        if (!line.empty())
        {
            if (in_ponderado_vertice)
            {
                istringstream iss(line);
                char id;
                int peso;
                iss >> id >> peso;
                No *no = new No(id, peso);
                lista_adj.push_back(no);
            }
            else
            {
                char id = line.at(0);
                No *no = new No(id);
                lista_adj.push_back(no);
            }
        }
    }

    // Lê as arestas
    while (getline(arquivo, line))
    {
        if (line.empty())
            continue;
        istringstream iss(line);
        char origem_id, destino_id;
        int peso = 1;

        if (in_ponderado_aresta)
        {
            iss >> origem_id >> destino_id >> peso;
        }
        else
        {
            iss >> origem_id >> destino_id;
        }

        No *origem = nullptr;
        No *destino = nullptr;

        for (No *no : lista_adj)
        {
            if (no->get_id() == origem_id)
                origem = no;
            if (no->get_id() == destino_id)
                destino = no;
        }

        if (origem && destino)
        {
            origem->add_vizinho(destino);
            origem->add_aresta(destino_id, peso);

            if (!in_direcionado)
            {
                destino->add_vizinho(origem);
                destino->add_aresta(origem_id, peso);
            }
        }
    }

    arquivo.close();
}

// Imprime informações gerais do grafo
void Grafo::imprimeInfo() const
{
    cout << "----- Informações do Grafo -----" << endl;
    cout << "Ordem: " << ordem << endl;
    cout << "Direcionado: " << (in_direcionado ? "Sim" : "Não") << endl;
    cout << "Ponderado nas Arestas: " << (in_ponderado_aresta ? "Sim" : "Não") << endl;
    cout << "Ponderado nos Vértices: " << (in_ponderado_vertice ? "Sim" : "Não") << endl;

    cout << "\nVértices:" << endl;
    for (const auto &no : lista_adj)
    {
        cout << "- " << no->get_id();
        if (in_ponderado_vertice)
        {
            cout << " (peso: " << no->get_peso() << ")";
        }
        cout << endl;
    }
    cout << "---------------------------------" << endl;
}

// Função auxiliar de DFS (busca em profundidade)
void Grafo::aux_dfs(No *no, set<char> &visitado)
{
    if (visitado.find(no->get_id()) == visitado.end())
    {
        visitado.insert(no->get_id());
        for (No *vizinho : no->get_vizinhos())
        {
            aux_dfs(vizinho, visitado);
        }
    }
}

// Destrutor do grafo
Grafo::~Grafo()
{
    for (No *no : lista_adj)
        delete no;
}

// Retorna o fecho transitivo direto de um nó (DFS)
vector<char> Grafo::fecho_transitivo_direto(char id_no)
{
    No *origem = nullptr;
    for (No *no : lista_adj)
    {
        if (no->get_id() == id_no)
        {
            origem = no;
            break;
        }
    }
    if (origem == nullptr)
    {
        cout << "No com id " << id_no << " nao encontrado." << endl;
        return {};
    }
    set<char> visitado;
    aux_dfs(origem, visitado);
    visitado.erase(id_no);
    return vector<char>(visitado.begin(), visitado.end());
}

// Retorna o fecho transitivo indireto de um nó
vector<char> Grafo::fecho_transitivo_indireto(char id_no)
{
    set<char> fecho;
    for (No *no : lista_adj)
    {
        set<char> visitado;
        aux_dfs(no, visitado);
        if (visitado.find(id_no) != visitado.end())
        {
            fecho.insert(no->get_id());
        }
    }
    fecho.erase(id_no);
    return vector<char>(fecho.begin(), fecho.end());
}


/// DIJKSTRA - INÍCIO

vector<char> Grafo::caminho_minimo_dijkstra(char id_no_a, char id_no_b)
{
    vector<char> ids = this->get_ids_vertices();
    int n = ids.size();

    // Mapeia o ID do vértice para seu índice na matriz
    auto indice_de = [&](char id) {
        for (int i = 0; i < n; i++) {
            if (ids[i] == id) return i;
        }
        return -1;
    };

    int origem = indice_de(id_no_a);
    int destino = indice_de(id_no_b);

    //cout << "ID origem: " << id_no_a << " (" << origem << ") | ID destino: " << id_no_b << " (" << destino << ")\n";

    if (origem == -1 || destino == -1) {
        cout << "Origem ou destino não encontrados!\n";
        return {};
    }

    vector<int> dist(n, INT_MAX);
    vector<int> visitado(n, 0);
    vector<int> anterior(n, -1);

    dist[origem] = 0;

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> fila;
    fila.push({0, origem});

    while (!fila.empty()) {
        int atual = fila.top().second;
        fila.pop();

        if (visitado[atual]) continue;
        visitado[atual] = 1;

        vector<Aresta*> vizinhos = this->get_vizinhanca(ids[atual]);

        for (Aresta* a : vizinhos) {
            int viz = indice_de(a->id_no_alvo);
            if (viz == -1) continue;

            int peso = this->get_ponderado_aresta() ? a->peso : 1;

            if (dist[atual] + peso < dist[viz]) {
                dist[viz] = dist[atual] + peso;
                anterior[viz] = atual;
                fila.push({dist[viz], viz});
            }
        }
    }

    // Reconstrução do caminho
    vector<char> caminho;
    for (int v = destino; v != -1; v = anterior[v]) {
        caminho.push_back(ids[v]);
        if (v == origem) break;
    }

    if (caminho.back() != id_no_a) {
        cout << "Caminho não encontrado.\n";
        return {};
    }

    // Inverte caminho
    reverse(caminho.begin(), caminho.end());

    cout << endl;

    return caminho;
}

vector<Aresta*> Grafo::get_vizinhanca(char id) {
    for (No* no : lista_adj) {
        if (no != nullptr && no->get_id() == id) {
            return no->get_arestas();
        }
    }
    return {};
}

vector<char> Grafo::get_ids_vertices() {
    vector<char> ids;
    for (No* no : lista_adj) {
        if (no != nullptr) {
            ids.push_back(no->get_id());
        }
    }
    return ids;
}


/// DIJKSTRA - FIM

vector<char> Grafo::caminho_minimo_floyd(char id_no, char id_no_b)
{
    cout << "Metodo nao implementado" << endl;
    return {};
}

Grafo *Grafo::arvore_geradora_minima_prim(vector<char> ids_nos)
{
    cout << "Metodo nao implementado" << endl;
    return nullptr;
}

Grafo *Grafo::arvore_geradora_minima_kruskal(vector<char> ids_nos)
{
    cout << "Metodo nao implementado" << endl;
    return nullptr;
}

Grafo *Grafo::arvore_caminhamento_profundidade(char id_no)
{
    cout << "Metodo nao implementado" << endl;
    return nullptr;
}

int Grafo::raio()
{
    cout << "Metodo nao implementado" << endl;
    return 0;
}

int Grafo::diametro()
{
    cout << "Metodo nao implementado" << endl;
    return 0;
}

vector<char> Grafo::centro()
{
    cout << "Metodo nao implementado" << endl;
    return {};
}

vector<char> Grafo::periferia()
{
    cout << "Metodo nao implementado" << endl;
    return {};
}

vector<char> Grafo::vertices_de_articulacao()
{
    cout << "Metodo nao implementado" << endl;
    return {};
}
