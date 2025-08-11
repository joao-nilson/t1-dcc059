#include "Grafo.h"
#include "No.h"
#include <cstdlib>
#include <functional>
#include <set>
#include <fstream>
#include <sstream>
#include <stack>
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
    string caminho = grafo;
    cout << "Tentando abrir arquivo em: " << caminho << endl;

    //caminho antigo das instancias
    // string caminho = "../instancias/" + grafo;
    // cout << "Tentando abrir arquivo em: " << caminho << endl;

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
    if (ids_nos.empty()) return nullptr;

    // Cria novo grafo para a AGM
    Grafo* agm = new Grafo();
    agm->set_direcionado(false);
    agm->set_ponderado_aresta(true);
    agm->set_ponderado_vertice(false);

    // Adiciona nós ao novo grafo
    for (char id : ids_nos) {
        agm->adiciona_no(new No(id));
    }

    // Coleta e filtra arestas
    vector<tuple<int, char, char>> arestas;
    for (No* no : lista_adj) {
        char u = no->get_id();
        if (find(ids_nos.begin(), ids_nos.end(), u) == ids_nos.end()) continue;
        
        for (Aresta* a : no->get_arestas()) {
            char v = a->id_no_alvo;
            if (find(ids_nos.begin(), ids_nos.end(), v) == ids_nos.end()) continue;
            
            // Evita duplicação (grafo não direcionado)
            if (u < v) {
                arestas.emplace_back(a->peso, u, v);
            }
        }
    }

    // Ordena arestas (por peso, depois por vértices para determinismo)
    sort(arestas.begin(), arestas.end(), 
        [](const auto& a, const auto& b) {
            if (get<0>(a) != get<0>(b)) 
                return get<0>(a) < get<0>(b);
            if (get<1>(a) != get<1>(b)) 
                return get<1>(a) < get<1>(b);
            return get<2>(a) < get<2>(b);
        });

    // Implementação Union-Find otimizada
    class UnionFind {
        unordered_map<char, char> parent;
    public:
        UnionFind(const vector<char>& nodes) {
            for (char id : nodes) {
                parent[id] = id;
            }
        }
        
        char find(char u) {
            if (parent[u] != u) {
                parent[u] = find(parent[u]); // Path compression
            }
            return parent[u];
        }
        
        bool unite(char u, char v) {
            char rootU = find(u);
            char rootV = find(v);
            if (rootU == rootV) return false;
            parent[rootV] = rootU;
            return true;
        }
    };

    UnionFind uf(ids_nos);
    int edgesAdded = 0;
    const int neededEdges = ids_nos.size() - 1;

    // Construção da AGM
    for (const auto& [peso, u, v] : arestas) {
        if (edgesAdded == neededEdges) break;
        
        if (uf.unite(u, v)) {
            No* noU = agm->get_no_by_id(u);
            No* noV = agm->get_no_by_id(v);
            if (noU && noV) {
                noU->add_aresta(v, peso);
                noV->add_aresta(u, peso);
                edgesAdded++;
            }
        }
    }

    if (edgesAdded != neededEdges) {
        cerr << "AVISO: Subgrafo não é conexo - AGM incompleta\n";
    }

    agm->set_ordem(ids_nos.size());
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
unordered_map<char, int> Grafo::dijkstra_distancia(char id_no) {
    unordered_map<char, int> dist;
    for (No *no : lista_adj) {
        dist[no->get_id()] = INT_MAX;
    }

    if (dist.find(id_no) == dist.end()) return dist;

    priority_queue<pair<int, char>, vector<pair<int, char>>, greater<>> pq;
    dist[id_no] = 0;
    pq.push({0, id_no});

    while (!pq.empty()) {
        int d = pq.top().first;
        char u = pq.top().second;
        pq.pop();

        if (d != dist[u]) continue;

        No* orig_u = get_no_by_id(u);
        if (!orig_u) continue;

        for (Aresta* a : orig_u->get_arestas()) {
            char v = a->id_no_alvo;
            int weight = a->peso;

            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

// Calcula excentricidades de todos os nós
vector<int> Grafo::computeEccentricities() {
    vector<int> eccs;
    for (No *no : lista_adj) {
        char id = no->get_id();
        unordered_map<char, int> dist_map = dijkstra_distancia(id);
        int ecc = 0;
        for (auto &p : dist_map) {
            if (p.second != INT_MAX && p.second > ecc) {
                ecc = p.second;
            }
        }
        eccs.push_back(ecc);
    }
    return eccs;
}
int Grafo::raio()
{
    if (lista_adj.empty()) return 0;
    vector<int> eccs = computeEccentricities();
    return *min_element(eccs.begin(), eccs.end());
}

int Grafo::diametro()
{
    if (lista_adj.empty()) return 0;
    vector<int> eccs = computeEccentricities();
    return *max_element(eccs.begin(), eccs.end());
}

vector<char> Grafo::centro()
{
    vector<char> center;
    if (lista_adj.empty()) return center;
    vector<int> eccs = computeEccentricities();
    int min_ecc = *min_element(eccs.begin(), eccs.end());
    for (int i = 0; i < lista_adj.size(); i++) {
        if (eccs[i] == min_ecc) {
            center.push_back(lista_adj[i]->get_id());
        }
    }
    return center;

}

vector<char> Grafo::periferia()
{
    vector<char> periphery;
    if (lista_adj.empty()) return periphery;
    vector<int> eccs = computeEccentricities();
    int max_ecc = *max_element(eccs.begin(), eccs.end());
    for (int i = 0; i < lista_adj.size(); i++) {
        if (eccs[i] == max_ecc) {
            periphery.push_back(lista_adj[i]->get_id());
        }
    }
    return periphery;
}

// Função auxiliar para DFS usada no Kosaraju
void Grafo::fillOrder(char v, set<char>& visited, stack<char>& Stack) {
    visited.insert(v);
    No* no = get_no_by_id(v);
    if (no) {
        for (Aresta* a : no->get_arestas()) {
            char neighbor = a->id_no_alvo;
            if (visited.find(neighbor) == visited.end()) {
                fillOrder(neighbor, visited, Stack);
            }
        }
    }
    Stack.push(v);
}
// Função auxiliar para DFS no grafo transposto
void Grafo::DFSUtil(char v, set<char>& visited) {
    visited.insert(v);
    No* no = get_no_by_id(v);
    if (no) {
        for (Aresta* a : no->get_arestas()) {
            char neighbor = a->id_no_alvo;
            if (visited.find(neighbor) == visited.end()) {
                DFSUtil(neighbor, visited);
            }
        }
    }
}
Grafo* Grafo::getTranspose() {
    Grafo* reversed = new Grafo();
    reversed->set_direcionado(true);
    reversed->set_ponderado_aresta(in_ponderado_aresta);
    reversed->set_ponderado_vertice(in_ponderado_vertice);
    reversed->set_ordem(ordem);

    // Adiciona todos os vértices
    for (No* originalNode : lista_adj) {
        No* newNode = new No(originalNode->get_id(), originalNode->get_peso());
        reversed->adiciona_no(newNode);
    }

    // Adiciona arestas invertidas
    for (No* originalNode : lista_adj) {
        char u = originalNode->get_id();
        for (Aresta* a : originalNode->get_arestas()) {
            char v = a->id_no_alvo;
            No* reversedNode = reversed->get_no_by_id(v);
            if (reversedNode) {
                reversedNode->add_aresta(u, a->peso);
            }
        }
    }

    return reversed;
}
// Conta o número de componentes fortemente conectados (Kosaraju)
int Grafo::countSCC() {
    stack<char> Stack;
    set<char> visited;

    // Primeira DFS para preencher a pilha
    for (No* no : lista_adj) {
        char id = no->get_id();
        if (visited.find(id) == visited.end()) {
            fillOrder(id, visited, Stack);
        }
    }

    // Cria grafo transposto
    Grafo* reversed = getTranspose();
    
    // Reseta visited para segunda DFS
    visited.clear();
    int count = 0;

    // Segunda DFS no grafo transposto
    while (!Stack.empty()) {
        char id = Stack.top();
        Stack.pop();

        if (visited.find(id) == visited.end()) {
            reversed->DFSUtil(id, visited);
            count++;
        }
    }

    delete reversed;
    return count;
}

// Remove um vértice e todas as arestas associadas
void Grafo::remove_vertex(char v) {
    auto it_node = lista_adj.begin();
    while (it_node != lista_adj.end()) {
        if ((*it_node)->get_id() == v) {
            delete *it_node;
            it_node = lista_adj.erase(it_node);
            break;
        } else {
            ++it_node;
        }
    }
    
    // Remove incident edges
    for (No* no : lista_adj) {
        // Get reference to edges vector
        vector<Aresta*>& arestas = no->get_arestas(); 
        auto it_edge = arestas.begin();
        while (it_edge != arestas.end()) {
            if ((*it_edge)->id_no_alvo == v) {
                delete *it_edge;
                it_edge = arestas.erase(it_edge);
            } else {
                ++it_edge;
            }
        }
    }
    ordem--;
}

// Função principal para grafos direcionados
vector<char> Grafo::directed_articulation_points() {
    vector<char> articulation_points;
    int originalSCC = countSCC();

    // Cria cópia do grafo para manipulação
    Grafo* tempGraph = new Grafo();
    *tempGraph = *this;  // Assume que o operador de cópia está implementado

    for (No* no : lista_adj) {
        char id = no->get_id();
        
        // Remove o vértice e conta SCCs
        tempGraph->remove_vertex(id);
        int newSCC = tempGraph->countSCC();
        
        // Restaura o grafo original
        *tempGraph = *this;
        
        // Verifica se o número de SCCs aumentou
        if (newSCC > originalSCC) {
            articulation_points.push_back(id);
        }
    }

    delete tempGraph;
    return articulation_points;
}
vector<char> Grafo::vertices_de_articulacao()
{
    // Verifica se o grafo é direcionado
    if (in_direcionado) {
        return directed_articulation_points();
    } else {

        // Mapeamentos para armazenar estados dos vértices
        unordered_map<char, int> disc;    // Tempo de descoberta
        unordered_map<char, int> low;     // Valor low (menor tempo alcançável)
        unordered_map<char, char> parent; // Pai na árvore DFS
        unordered_map<char, bool> visited;// Marcador de visitação
        unordered_map<char, bool> isAP;   // Indica se é ponto de articulação

        for (No* no : lista_adj) {
            char id = no->get_id();
            disc[id] = -1;
            low[id] = -1;
            parent[id] = '\0'; // '\0' indica ausência de pai
            visited[id] = false;
            isAP[id] = false;
        }

        int time = 0; // Contador global de tempo

        // Função DFS recursiva para encontrar pontos de articulação
        function<void(char)> dfs = [&](char u) {
            visited[u] = true;
            disc[u] = low[u] = ++time;
            int children = 0; // Contador de filhos na árvore DFS

            // Encontra o nó correspondente na lista de adjacência
            No* no_u = nullptr;
            for (No* no : lista_adj) {
                if (no->get_id() == u) {
                    no_u = no;
                    break;
                }
            }
            if (!no_u) return;

            // Explora todos os vizinhos do nó atual
            for (Aresta* a : no_u->get_arestas()) {
                char v = a->id_no_alvo;

                if (!visited[v]) {
                    children++;
                    parent[v] = u;
                    dfs(v);

                    // Atualiza o valor low de u após retornar da DFS
                    low[u] = min(low[u], low[v]);

                    // Verifica se u é ponto de articulação (não raiz)
                    if (parent[u] != '\0' && low[v] >= disc[u]) {
                        isAP[u] = true;
                    }
                } 
                // Atualiza low para arestas de retorno (v já visitado e não é pai de u)
                else if (v != parent[u]) {
                    low[u] = min(low[u], disc[v]);
                }
            }

            // Verifica se u (raiz) é ponto de articulação
            if (parent[u] == '\0' && children > 1) {
                isAP[u] = true;
            }
        };

        // Executa DFS para cada componente conexo não visitado
        for (No* no : lista_adj) {
            char id = no->get_id();
            if (!visited[id]) {
                parent[id] = '\0'; // Configura como raiz do componente
                dfs(id);
            }
        }

        // Coleta todos os vértices de articulação identificados
        vector<char> articulacoes;
        for (No* no : lista_adj) {
            if (isAP[no->get_id()]) {
                articulacoes.push_back(no->get_id());
            }
        }

        return articulacoes;
    }
}
