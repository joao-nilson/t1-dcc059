#include "DominacaoPerfeita.h"
#include <algorithm>
#include <unordered_set>
#include <limits>
#include <numeric>
#include <cmath>

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
    return true;
}

// Construtora genérica (gulosa / GRASP): devolve solução ou falha
PDSResultado DominacaoPerfeita::construcao(Grafo* G, std::mt19937& rng, double alpha) {
    // --- ids ordenados, graus, estruturas base ---
    vector<char> ids = ordena_ids(G->get_ids_vertices());
    unordered_set<char> D;                 // solução parcial
    unordered_map<char,int> dom;           // dominadores para V\D
    unordered_map<char,int> deg = grau(G); // graus

    auto esta_fora = [&](char v){ return D.count(v)==0; };

    // ganho(u): quantos vizinhos fora de D com dom==0 passarão a ser dominados
    auto ganho = [&](char u)->int {
        int g = 0;
        for (char w : vizinhos(G,u)) {
            if (esta_fora(w)) {
                int val = 0;
                auto it = dom.find(w);
                if (it != dom.end()) val = it->second;
                if (val == 0) g++;
            }
        }
        return g;
    };

    // Inserção com reparo: aceita conflitos e promove em cascata vizinhos com dom==1
    auto inserir_com_reparo = [&](char u) {
        if (!esta_fora(u)) return;

        // 1) Insere u em D
        D.insert(u);
        dom.erase(u);

        // 2) Primeiro, marque vizinhos de u: se já tinham dom==1, entram na fila de promoção
        std::vector<char> fila;
        for (char w : vizinhos(G, u)) {
            if (esta_fora(w)) {
                int val = 0;
                if (auto it = dom.find(w); it != dom.end()) val = it->second;
                if (val == 1) fila.push_back(w); // conflito: teria 2 dominadores fora de D
                dom[w] = 1; // manter como "1 dominador" (não somamos)
            }
        }

        // 3) Processa promoções em cascata
        while (!fila.empty()) {
            char w = fila.back();
            fila.pop_back();
            if (!esta_fora(w)) continue;

            // promove w
            D.insert(w);
            dom.erase(w);

            // ao promover w, seus vizinhos fora de D ficam com dom=1;
            // se algum vizinho z já estava com dom==1, ele também precisa ser promovido (entra na fila)
            for (char z : vizinhos(G, w)) {
                if (esta_fora(z)) {
                    int valz = 0;
                    if (auto itz = dom.find(z); itz != dom.end()) valz = itz->second;
                    if (valz == 1) fila.push_back(z); // conflito em cascata
                    dom[z] = 1;
                }
            }
        }
    };

    // --- Regra de isolados: entram direto em D ---
    for (char v : ids) {
        if (!deg.count(v)) deg[v] = 0; // robustez
        if (deg[v] == 0) {
            inserir_com_reparo(v);
        }
    }
    // Inicializa dom apenas para quem não está em D
    for (char v : ids) if (esta_fora(v) && !dom.count(v)) dom[v] = 0;

    auto existe_fora_nao_dominado = [&](){
        for (auto &kv : dom) if (kv.second == 0) return true;
        return false;
    };

    // =========================
    // Laço principal de construção
    // =========================
    while (existe_fora_nao_dominado()) {

        // ===== PASSO 0 — PROMOÇÃO (anti-deadlock “normal”) =====
        // Se existe w fora de D com dom[w]==1 e ganho(w)>0, promove
        {
            char w_prom = 0;
            for (char w : ids) if (esta_fora(w)) {
                int domw = 0;
                auto it = dom.find(w);
                if (it != dom.end()) domw = it->second;
                if (domw == 1 && ganho(w) > 0) {
                    w_prom = w;
                    break;
                }
            }
            if (w_prom != 0) {
                inserir_com_reparo(w_prom);
                continue; // volta ao topo do while
            }
        }

        // ===== PASSO 1 — CANDIDATOS (ganho>0) =====
        vector<pair<char,int>> cand;
        for (char u : ids) if (esta_fora(u)) {
            int g = ganho(u);
            if (g > 0) cand.push_back({u, g});
        }

        // ===== PASSO 2 — ESCOLHA (guloso puro ou RCL) =====
        if (!cand.empty()) {
            char escolhido;
            if (alpha < 0.0) {
                // Guloso puro: maior ganho; desempate por id
                std::sort(cand.begin(), cand.end(),
                    [](auto& A, auto& B){
                        if (A.second != B.second) return A.second > B.second;
                        return A.first < B.first;
                    });
                escolhido = cand.front().first;
            } else {
                // GRASP: RCL por alpha
                int gmin = std::numeric_limits<int>::max();
                int gmax = std::numeric_limits<int>::min();
                for (auto& p : cand) { gmin = std::min(gmin,p.second); gmax = std::max(gmax,p.second); }
                double corte = gmin + alpha * (gmax - gmin);
                vector<char> RCL;
                for (auto& p : cand) if (p.second >= (int)std::ceil(corte)) RCL.push_back(p.first);
                if (RCL.empty()) {
                    std::sort(cand.begin(), cand.end(), [](auto&a, auto&b){return a.second>b.second;});
                    escolhido = cand.front().first;
                } else {
                    std::uniform_int_distribution<int> uni(0, (int)RCL.size()-1);
                    escolhido = RCL[uni(rng)];
                }
            }

            // PASSO 3 — insere escolhido COM REPARO
            inserir_com_reparo(escolhido);
            continue; // próximo ciclo
        }

        // ===== Fallback A — cand vazio, mas ainda há vértice não dominado =====
        {
            char alvo = 0;
            for (auto &kv : dom) {
                if (kv.second == 0) { alvo = kv.first; break; }
            }
            if (alvo != 0) {
                // Escolhe um vizinho de 'alvo' (pode ter ganho 0); reparo garante perfeição
                char melhor = 0;
                int melhorGanho = -1;
                for (Aresta* a : G->get_vizinhanca(alvo)) {
                    char u = a->id_no_alvo;
                    if (!esta_fora(u)) continue; // já em D
                    int g = ganho(u);             // pode ser 0
                    if (g > melhorGanho) { melhor = u; melhorGanho = g; }
                }
                if (melhor != 0) {
                    inserir_com_reparo(melhor);
                    continue; // volta ao topo
                }
                // Se não achou vizinho para cobrir 'alvo', cai no Fallback B
            }
        }

        // ===== Fallback B — promover alguém com dom==1 mesmo com ganho==0 =====
        {
            char w_prom = 0;
            for (char w : ids) if (esta_fora(w)) {
                int domw = 0; auto it = dom.find(w);
                if (it != dom.end()) domw = it->second;
                if (domw == 1) { // sem exigir ganho>0 aqui; reparo cuida dos conflitos
                    w_prom = w; break;
                }
            }
            if (w_prom != 0) {
                inserir_com_reparo(w_prom);
                continue;
            }
        }

        // Se chegou aqui, realmente não há como avançar (muito improvável com reparo)
        return PDSResultado{ {}, false };
    }

    // ---------- Reconstrói vetor e valida ----------
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
