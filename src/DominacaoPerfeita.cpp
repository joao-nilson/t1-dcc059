#include "DominacaoPerfeita.h"
#include <algorithm>
#include <unordered_set>
#include <limits>
#include <numeric>

using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::pair;

static vector<char> ordena_ids(const vector<char>& ids) {
    auto v = ids;
    std::sort(v.begin(), v.end());
    return v;
}

bool DominacaoPerfeita::ehDirecionado(Grafo* G) {
    return G->get_direcionado();
}

vector<char> DominacaoPerfeita::vizinhos(Grafo* G, char u) {
    vector<char> vs;
    for (Aresta* a : G->get_vizinhanca(u)) {
        vs.push_back(a->id_no_alvo);
    }
    return vs;
}

unordered_map<char,int> DominacaoPerfeita::grau(Grafo* G) {
    unordered_map<char,int> g;
    for (No* no : G->get_lista_adj()) {
        g[no->get_id()] = (int)G->get_vizinhanca(no->get_id()).size();
    }
    return g;
}

bool DominacaoPerfeita::verificaPDS(Grafo* G, const vector<char>& Dvec) {
    unordered_set<char> D(Dvec.begin(), Dvec.end());
    // Conta dominadores (apenas para V\D)
    unordered_map<char,int> dom;
    for (No* no : G->get_lista_adj()) {
        char v = no->get_id();
        if (D.count(v)==0) dom[v] = 0;
    }

    for (char u : D) {
        for (Aresta* a : G->get_vizinhanca(u)) {
            char v = a->id_no_alvo;
            if (D.count(v)==0) dom[v] += 1;
        }
    }
    // todo v fora de D deve ter exatamente 1 dominador
    for (auto& kv : dom) {
        if (kv.second != 1) return false;
    }
    // vértices isolados fora de D falham; dentro de D pode
    return true;
}

// Construtora genérica (gulosa / GRASP): devolve solução ou falha
PDSResultado DominacaoPerfeita::construcao(Grafo* G, std::mt19937& rng, double alpha) {
    vector<char> ids = ordena_ids(G->get_ids_vertices());
    unordered_set<char> D;                 // solução parcial
    unordered_map<char,int> dom;           // dominadores para V\D
    unordered_map<char,int> deg = grau(G); // graus

    // Inicializa dom para não-D (ainda vazio; vamos manter coerente a cada passo)
    for (char v : ids) if (!deg.count(v)) deg[v]=0; // robustez

    auto esta_fora = [&](char v){ return D.count(v)==0; };
    auto factivel = [&](char u)->bool {
        // u só pode “tocar” vértices ainda não dominados
        for (char w : vizinhos(G,u)) {
            if (esta_fora(w)) {
                // se já está dominado por 1, adicionar u criará conflito
                // precisamos saber dom[w]; se ainda não inicializamos, considere 0
                auto it = dom.find(w);
                int val = (it==dom.end()? 0 : it->second);
                if (val >= 1) return false;
            }
        }
        return true;
    };

    auto ganho = [&](char u)->int {
        int g = 0;
        for (char w : vizinhos(G,u)) {
            if (esta_fora(w)) {
                auto it = dom.find(w);
                int val = (it==dom.end()? 0 : it->second);
                if (val == 0) g++;
            }
        }
        return g;
    };

    // Regra: vértices isolados só podem estar em D — adicione todos
    for (char v : ids) {
        if (deg[v]==0) D.insert(v);
    }

    // Recalcula dom após inserir isolados (não altera dom porque isolados não dominam ninguém)
    for (char v : ids) if (D.count(v)==0) dom[v]=0;

    auto existe_fora_nao_dominado = [&](){
        for (auto &kv : dom) if (kv.second==0) return true;
        return false;
    };

    // Laço principal
    while (existe_fora_nao_dominado()) {
        // Constrói conjunto de candidatos viáveis com scores
        vector<pair<char,int>> cand;
        for (char u : ids) if (D.count(u)==0) {
            if (factivel(u)) {
                int g = ganho(u);
                if (g>0) cand.push_back({u,g});
            }
        }
        if (cand.empty()) {
            // Não há como dominar quem falta sem criar conflito
            return PDSResultado{ {}, false };
        }

        // Escolha do candidato: guloso puro (alpha<0) ou RCL (alpha in [0,1])
        char escolhido;
        if (alpha < 0.0) {
            // guloso determinístico: maior ganho, desempate por id
            std::sort(cand.begin(), cand.end(),
                [](auto& A, auto& B){
                    if (A.second != B.second) return A.second > B.second;
                    return A.first < B.first;
                });
            escolhido = cand.front().first;
        } else {
            int gmin = std::numeric_limits<int>::max();
            int gmax = std::numeric_limits<int>::min();
            for (auto& p : cand) { gmin = std::min(gmin,p.second); gmax = std::max(gmax,p.second); }
            double corte = gmin + alpha * (gmax - gmin);
            vector<char> RCL;
            for (auto& p : cand) if (p.second >= (int)std::ceil(corte)) RCL.push_back(p.first);
            if (RCL.empty()) { // fallback
                std::sort(cand.begin(), cand.end(), [](auto&a, auto&b){return a.second>b.second;});
                escolhido = cand.front().first;
            } else {
                std::uniform_int_distribution<int> uni(0, (int)RCL.size()-1);
                escolhido = RCL[uni(rng)];
            }
        }

        // Inclui escolhido em D e atualiza dom
        D.insert(escolhido);
        for (char w : vizinhos(G,escolhido)) {
            if (D.count(w)==0) dom[w] = 1; // passa a ser dominado por 1
        }
        // Observação: se algum w fora de D já tinha dom[w]==1, factível() teria bloqueado escolhido
    }

    // Converte para vetor ordenado
    vector<char> Dout(D.begin(), D.end());
    std::sort(Dout.begin(), Dout.end());
    bool ok = verificaPDS(G, Dout);
    return PDSResultado{ ok ? Dout : vector<char>{}, ok };
}

PDSResultado DominacaoPerfeita::guloso(Grafo* G) {
    std::mt19937 rng(123); // determinístico
    return construcao(G, rng, -1.0); // alpha<0 => guloso puro
}

PDSResultado DominacaoPerfeita::grasp(Grafo* G, int iteracoes, double alpha, unsigned seed) {
    std::mt19937 rng(seed);
    PDSResultado best; best.factivel=false;
    for (int it=0; it<iteracoes; ++it) {
        auto res = construcao(G, rng, alpha);
        if (res.factivel && (!best.factivel || res.custo() < best.custo())) {
            best = res;
        }
    }
    return best;
}

PDSResultado DominacaoPerfeita::reativo(Grafo* G, int iteracoes,
                                        const vector<double>& alphas, int bloco,
                                        unsigned seed) {
    std::mt19937 rng(seed);
    int k = (int)alphas.size();
    vector<double> prob(k, 1.0/k);
    vector<double> score(k, 0.0); // qualidade média por alpha
    vector<int> cont(k, 0);

    auto escolhe_idx = [&](){
        std::discrete_distribution<int> dist(prob.begin(), prob.end());
        return dist(rng);
    };

    PDSResultado best; best.factivel=false;

    for (int it=1; it<=iteracoes; ++it) {
        int idx = escolhe_idx();
        double alpha = alphas[idx];
        auto res = construcao(G, rng, alpha);
        if (res.factivel) {
            // qualidade = 1 / |D| (menor D é melhor)
            double q = 1.0 / std::max(1, res.custo());
            score[idx] += q;
            cont[idx]  += 1;
            if (!best.factivel || res.custo() < best.custo()) best = res;
        }

        // Atualiza probabilidades a cada "bloco" iterações
        if (it % bloco == 0) {
            vector<double> media(k, 0.0);
            double soma = 0.0;
            for (int i=0;i<k;i++){
                media[i] = (cont[i] ? score[i]/cont[i] : 0.0);
                soma += media[i];
            }
            if (soma > 0.0) {
                for (int i=0;i<k;i++) prob[i] = media[i] / soma;
            } else {
                std::fill(prob.begin(), prob.end(), 1.0/k);
            }
            // “esquecer” um pouco para continuar adaptando
            std::fill(score.begin(), score.end(), 0.0);
            std::fill(cont.begin(),  cont.end(),  0);
        }
    }
    return best;
}
