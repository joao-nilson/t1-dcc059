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
#include <unordered_map>
#include <map>
#include <functional>

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
void Grafo::aux_dfs_reverso(char id_atual, set<char> &visitado, const unordered_map<char, vector<No *>> &lista_adjacencia_inversa)
{
    if (visitado.find(id_atual) != visitado.end())
        return;

    visitado.insert(id_atual);

    auto it = lista_adjacencia_inversa.find(id_atual);
    if (it != lista_adjacencia_inversa.end())
    {
        for (No *no_origem : it->second)
        {
            aux_dfs_reverso(no_origem->get_id(), visitado, lista_adjacencia_inversa);
        }
    }
}

vector<char> Grafo::fecho_transitivo_indireto(char id_no)
{
    // Constrói a lista de adjacência inversa
    unordered_map<char, vector<No *>> lista_adjacencia_inversa;
    for (No *no : lista_adj)
    {
        for (Aresta *aresta : no->get_arestas())
        {
            lista_adjacencia_inversa[aresta->id_no_alvo].push_back(no);
        }
    }

    set<char> visitado;
    aux_dfs_reverso(id_no, visitado, lista_adjacencia_inversa);

    visitado.erase(id_no); // não inclui o próprio nó

    return vector<char>(visitado.begin(), visitado.end());
}

/// DIJKSTRA - INÍCIO

vector<char> Grafo::caminho_minimo_dijkstra(char id_no_a, char id_no_b)
{
    vector<char> ids = this->get_ids_vertices();
    int n = ids.size();

    // Mapeia o ID do vértice para seu índice na matriz
    auto indice_de = [&](char id)
    {
        for (int i = 0; i < n; i++)
        {
            if (ids[i] == id)
                return i;
        }
        return -1;
    };

    int origem = indice_de(id_no_a);
    int destino = indice_de(id_no_b);

    if (origem == -1 || destino == -1)
    {
        cout << "Origem ou destino não encontrados!\n";
        return {};
    }

    vector<int> dist(n, INT_MAX);
    vector<int> visitado(n, 0);
    vector<int> anterior(n, -1);

    dist[origem] = 0;

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> fila;
    fila.push({0, origem});

    while (!fila.empty())
    {
        int atual = fila.top().second;
        fila.pop();

        if (visitado[atual])
            continue;
        visitado[atual] = 1;

        vector<Aresta *> vizinhos = this->get_vizinhanca(ids[atual]);

        for (Aresta *a : vizinhos)
        {
            int viz = indice_de(a->id_no_alvo);
            if (viz == -1)
                continue;

            int peso = this->get_ponderado_aresta() ? a->peso : 1;

            if (dist[atual] + peso < dist[viz])
            {
                dist[viz] = dist[atual] + peso;
                anterior[viz] = atual;
                fila.push({dist[viz], viz});
            }
        }
    }

    // Reconstrução do caminho
    vector<char> caminho;
    for (int v = destino; v != -1; v = anterior[v])
    {
        caminho.push_back(ids[v]);
        if (v == origem)
            break;
    }

    if (caminho.back() != id_no_a)
    {
        cout << "Caminho não encontrado.\n";
        return {};
    }

    // Inverte caminho
    reverse(caminho.begin(), caminho.end());

    cout << endl;

    return caminho;
}

/// DIJKSTRA - FIM

/// AUXILIARES CAMINHO MINIMO - INICIO

vector<Aresta *> Grafo::get_vizinhanca(char id)
{
    for (No *no : lista_adj)
    {
        if (no != nullptr && no->get_id() == id)
        {
            return no->get_arestas();
        }
    }
    return {};
}

vector<char> Grafo::get_ids_vertices()
{
    vector<char> ids;
    for (No *no : lista_adj)
    {
        if (no != nullptr)
        {
            ids.push_back(no->get_id());
        }
    }
    return ids;
}

/// AUXILIARES CAMINHO MINIMO - INICIO

/// FLOYD - INICIO

vector<char> Grafo::caminho_minimo_floyd(char id_no, char id_no_b)
{
    vector<char> ids = this->get_ids_vertices();
    int n = ids.size();

    unordered_map<char, int> id_para_indice;
    for (int i = 0; i < n; ++i)
        id_para_indice[ids[i]] = i;

    if (id_para_indice.find(id_no) == id_para_indice.end() ||
        id_para_indice.find(id_no_b) == id_para_indice.end())
    {
        cout << "Origem ou destino não encontrados!\n";
        return {};
    }

    vector<vector<int>> dist(n, vector<int>(n, INT_MAX));
    vector<vector<int>> pred(n, vector<int>(n, -1));

    // Inicializa a matriz de distâncias
    for (int i = 0; i < n; ++i)
        dist[i][i] = 0;

    for (No *no : lista_adj)
    {
        int u = id_para_indice[no->get_id()];
        for (Aresta *a : no->get_arestas())
        {
            int v = id_para_indice[a->id_no_alvo];
            int peso = get_ponderado_aresta() ? a->peso : 1;
            dist[u][v] = peso;
            pred[u][v] = u;

            // Se o grafo não for direcionado, adiciona o inverso
            if (!get_direcionado())
            {
                dist[v][u] = peso;
                pred[v][u] = v;
            }
        }
    }

    // Algoritmo de Floyd-Warshall
    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (dist[i][k] != INT_MAX && dist[k][j] != INT_MAX &&
                    dist[i][k] + dist[k][j] < dist[i][j])
                {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    pred[i][j] = pred[k][j];
                }

    // Reconstrói o caminho
    int origem = id_para_indice[id_no];
    int destino = id_para_indice[id_no_b];

    if (dist[origem][destino] == INT_MAX)
    {
        cout << "Caminho não encontrado.\n";
        return {};
    }

    vector<char> caminho;
    for (int at = destino; at != origem; at = pred[origem][at])
    {
        if (at == -1)
        {
            cout << "Caminho não encontrado.\n";
            return {};
        }
        caminho.push_back(ids[at]);
    }
    caminho.push_back(ids[origem]);
    reverse(caminho.begin(), caminho.end());

    return caminho;
}

/// FLOYD - FIM

/// AGM - PRIM - INICIO

Grafo *Grafo::arvore_geradora_minima_prim(vector<char> ids_nos)
{
    if (ids_nos.empty())
        return nullptr;

    Grafo *agm = new Grafo();
    agm->set_direcionado(false);
    agm->set_ponderado_aresta(true);
    agm->set_ponderado_vertice(false);

    set<char> visitado;
    map<char, No *> mapa_nos;

    // Adiciona todos os nós ao novo grafo AGM
    for (char id : ids_nos)
    {
        No *no = new No(id);
        mapa_nos[id] = no;
        agm->adiciona_no(no);
    }

    // Estrutura de min-heap: (peso, origem, destino)
    using ParAresta = tuple<float, char, char>;
    priority_queue<ParAresta, vector<ParAresta>, greater<ParAresta>> heap;

    char atual = ids_nos[0];
    visitado.insert(atual);

    for (Aresta *a : get_vizinhanca(atual))
    {
        if (find(ids_nos.begin(), ids_nos.end(), a->id_no_alvo) != ids_nos.end())
        {
            heap.emplace(a->peso, atual, a->id_no_alvo);
        }
    }

    cout << "Criando AGM..." << endl;

    while (!heap.empty() && visitado.size() < ids_nos.size())
    {
        auto tupla = heap.top();
        auto peso = std::get<0>(tupla);
        auto origem = std::get<1>(tupla);
        auto destino = std::get<2>(tupla);

        heap.pop();

        if (visitado.count(destino))
            continue;
        visitado.insert(destino);

        mapa_nos[origem]->add_aresta(destino, peso);
        mapa_nos[destino]->add_aresta(origem, peso); // grafo não direcionado

        cout << "Adicionando aresta " << origem << " - " << destino << " (" << peso << ")" << endl;

        for (Aresta *a : get_vizinhanca(destino))
        {
            if (!visitado.count(a->id_no_alvo) &&
                find(ids_nos.begin(), ids_nos.end(), a->id_no_alvo) != ids_nos.end())
            {
                heap.emplace(a->peso, destino, a->id_no_alvo);
            }
        }
    }

    agm->set_ordem(agm->get_lista_adj().size());
    return agm;
}

// Classe auxiliar UnionFind para o algoritmo de Kruskal
// Implementa a estrutura de conjunto disjunto (disjoint set union - DSU),
// usada para detectar ciclos e unir componentes em grafos

class UnionFind {
public:
    // Construtor: cria n conjuntos disjuntos, cada elemento é seu próprio pai
    UnionFind(int n) {
        parent.resize(n);
        rank.resize(n, 0);
        for (int i = 0; i < n; ++i) {
            parent[i] = i;
        }
    }

    // Encontra o representante (pai) do conjunto onde o elemento x está,
    // com compressão de caminho para otimizar futuras consultas
    int find(int x) {
        if (parent[x] != x) {
            parent[x] = find(parent[x]);
        }
        return parent[x];
    }

    // Une os conjuntos que contêm x e y.
    // Retorna true se a união foi feita (estavam em conjuntos diferentes),
    // ou false se já estavam no mesmo conjunto (união não feita para evitar ciclo)
    bool unite(int x, int y) {
        int rootX = find(x);
        int rootY = find(y);

        if (rootX == rootY) {
            return false; // já estão unidos, não une para evitar ciclo
        }

        // Une o conjunto de menor rank ao de maior rank para balancear a árvore
        if (rank[rootX] < rank[rootY]) {
            parent[rootX] = rootY;
        } else if (rank[rootY] < rank[rootX]) {
            parent[rootY] = rootX;
        } else {
            parent[rootY] = rootX;
            rank[rootX]++;
        }
        return true;
    }

private:
    std::vector<int> parent; // parent[i] é o pai do elemento i
    std::vector<int> rank;   // rank[i] é a altura da árvore cujo i é raiz
};

// Retorna um novo grafo com a Árvore Geradora Mínima usando o algoritmo de Kruskal
Grafo* Grafo::arvore_geradora_minima_kruskal(vector<char> ids_nos) {
    // Verifica se a lista de nós está vazia
    if (ids_nos.empty())
        return nullptr;

    // Cria um mapa de ID para índice numérico (necessário para o UnionFind)
    unordered_map<char, int> id_para_indice;
    for (int i = 0; i < (int)ids_nos.size(); i++) {
        id_para_indice[ids_nos[i]] = i;
    }

    // Cria um novo grafo para armazenar a AGM
    Grafo* agm = new Grafo();
    agm->set_direcionado(false);
    agm->set_ponderado_aresta(true);
    agm->set_ponderado_vertice(false);

    // Cria os nós no novo grafo e armazena ponteiros para fácil acesso
    unordered_map<char, No*> mapa_nos;
    for (char id : ids_nos) {
        No* no = new No(id);
        mapa_nos[id] = no;
        agm->adiciona_no(no);
    }

    // Cria uma lista de todas as arestas entre os nós do subconjunto
    vector<tuple<int, char, char>> arestas; // (peso, origem, destino)
    for (char origem_id : ids_nos) {
        No* no_origem = nullptr;
        for (No* no : lista_adj) {
            if (no->get_id() == origem_id) {
                no_origem = no;
                break;
            }
        }
        if (!no_origem) continue;

        // Percorre arestas e adiciona somente uma direção (evita duplicação)
        for (Aresta* a : no_origem->get_arestas()) {
            char destino_id = a->id_no_alvo;
            if (id_para_indice.count(destino_id) && origem_id < destino_id) {
                int peso = get_ponderado_aresta() ? a->peso : 1;
                arestas.emplace_back(peso, origem_id, destino_id);
            }
        }
    }

    // Ordena as arestas pelo peso (menor peso primeiro)
    sort(arestas.begin(), arestas.end(), [](auto& a, auto& b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    // Inicializa estrutura UnionFind com número de nós do subconjunto
    UnionFind uf((int)ids_nos.size());

    // Percorre arestas em ordem crescente de peso
    for (size_t i = 0; i < arestas.size(); ++i) {
        auto tupla = arestas[i];
        int peso = std::get<0>(tupla);
        char origem = std::get<1>(tupla);
        char destino = std::get<2>(tupla);

        int idx_orig = id_para_indice[origem];
        int idx_dest = id_para_indice[destino];

        // Se a aresta não forma ciclo (os nós estão em componentes diferentes)
        if (uf.unite(idx_orig, idx_dest)) {
            // Adiciona a aresta ao grafo AGM
            mapa_nos[origem]->add_aresta(destino, peso);
            mapa_nos[destino]->add_aresta(origem, peso); // não direcionado

            cout << "Adicionando aresta " << origem << " - " << destino << " (" << peso << ")" << endl;
        }
    }

    agm->set_ordem((int)ids_nos.size());
    return agm;
}

No* Grafo::getBuscaNo(char id) {
    for (No* no : this->lista_adj) {
        if (no->get_id() == id) {
            return no;
        }
    }
    return nullptr;
}

Grafo* Grafo::arvore_caminhamento_profundidade(char id_no) 
{
 if (getBuscaNo(id_no) == nullptr)
    {
        cout << "Erro: O nó " << id_no << " nao existe no grafo." << endl;
        return nullptr;
    }

    Grafo* arvore_resultante = new Grafo();
    arvore_resultante->in_direcionado = this->in_direcionado;
    arvore_resultante->in_ponderado_aresta = this->in_ponderado_aresta;
    arvore_resultante->in_ponderado_vertice = this->in_ponderado_vertice;

    map<char, int> cor;
    for (No* no : this->lista_adj)
    {
        cor[no->get_id()] = 0;
    }
    map<char, No*> nos_arvore;

    cout << "Busca em profundidade a partir de " << id_no << "." << endl << "Arestas de retorno:" << endl;

    function<void(char)> dfs_visit;
    dfs_visit = [&](char id)
    {
        cor[id] = 1;

        No* u_no_original = this->getBuscaNo(id);
        if (!u_no_original) return;

        if (nos_arvore.find(id) == nos_arvore.end())
        {
            No* no_novo = new No(id);
            arvore_resultante->adiciona_no(no_novo);
            nos_arvore[id] = no_novo;
        }
        No* no_arvore = nos_arvore[id];

        for (Aresta* aresta : u_no_original->get_arestas())
        {
            char v_id = aresta->id_no_alvo;

            if (cor.count(v_id) && cor[v_id] == 0)
            {
                int peso = get_ponderado_aresta() ? aresta->peso : 1;
                no_arvore->add_aresta(v_id, peso);
                dfs_visit(v_id);
            }
            else if (cor.count(v_id) && cor[v_id] == 1)
            {
                cout << "Aresta de retorno: " << id << " -> " << v_id << endl;
            }
        }
        cor[id] = 2;
    };

    dfs_visit(id_no);

    arvore_resultante->set_ordem(arvore_resultante->get_lista_adj().size());

    return arvore_resultante;
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
