#include "DominacaoPerfeita.h"
#include <algorithm>
#include <unordered_set>
#include <limits>
#include <numeric>
#include <chrono>
#include <fstream>
#include <functional>
#include "Grafo.h"
#include <unordered_map>
#include <random>
#include <queue>
#include <climits>
#include <iostream>
#include <numeric>
#include <cmath>

using namespace std;


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
    vector<char> ids;// = ordena_ids(G->get_ids_vertices());
    for (No* no : G->get_lista_adj()) {
        ids.push_back(no->get_id());
    }
    sort(ids.begin(), ids.end());

    unordered_set<char> D;                 // solução parcial
    unordered_map<char,int> dom;           // dominadores para V\D
    unordered_map<char,int> deg; // = grau(G); // graus

    // Inicializa dom para não-D (ainda vazio; vamos manter coerente a cada passo)
    for (char v : ids) if (!deg.count(v)) deg[v]=0; // robustez
    // for (char v : ids) {
    //     deg[v] = static_cast<int>(G->get_vizinhanca(v).size());
    //     if (deg[v] == 0) D.insert(v);
    //     else dom[v] = 0;
    // }

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
        for (auto &kv : dom) if (kv.second==0){
            cout << "[DEBUG] Ainda nao dominado: " << kv.first << "\n";
        return true;
        }
        return false;
    };

    // Laço principal
    while (existe_fora_nao_dominado()) {

        // ==============================
        // PASSO 0 — PROMOÇÃO (ANTI-DEADLOCK)
        // Se existe algum vértice w fora de D com dom[w] == 1 e ele é factível,
        // promovemos w imediatamente para D. Isso quebra bloqueios do tipo C–(M)–(F,J).
        // ==============================
        {
            
            char w_prom = 0;
            bool achou = false;
            for (char w : ids) if (D.count(w) == 0) {
                int domw = 0;
                auto it = dom.find(w);
                if (it != dom.end()) domw = it->second;

                if (domw == 1 && factivel(w) && ganho(w) > 0) {
                    w_prom = w;
                    achou = true;
                    break;
                }
            }

            if (achou) {
                // Inclui promovido e atualiza dom
                                         cout << "[DEBUG] Promovido: " << w_prom 
                                          << " (ganho=" << ganho(w_prom) << ")\n";
                D.insert(w_prom);
                dom.erase(w_prom);
                for (char z : vizinhos(G, w_prom)) {
                    if (D.count(z) == 0) dom[z] = 1; // passa a ser dominado por 1
                }
                // volta ao topo do while para reavaliar; isso evita montar cand/RCL à toa
                continue;
            }
        }

        // ==============================
        // PASSO 1 — CANDIDATOS VIÁVEIS (como antes)
        // ==============================
        vector<pair<char,int>> cand;
        for (char u : ids) if (D.count(u) == 0) {
            if (factivel(u)) {
                int g = ganho(u);
                if (g > 0) cand.push_back({u, g});
            }
        }
        if (cand.empty()) {
            // Não há como dominar quem falta sem criar conflito
            return PDSResultado{ {}, false };
        }

        // ==============================
        // PASSO 2 — ESCOLHA (guloso puro ou RCL se alpha >= 0)
        // ==============================
        char escolhido;
        if (alpha < 0.0) {
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
            if (RCL.empty()) {
                std::sort(cand.begin(), cand.end(), [](auto&a, auto&b){return a.second>b.second;});
                escolhido = cand.front().first;
            } else {
                std::uniform_int_distribution<int> uni(0, (int)RCL.size()-1);
                escolhido = RCL[uni(rng)];
            }
        }

        // ==============================
        // PASSO 3 — INSERE ESCOLHIDO E ATUALIZA dom
        // ==============================
        D.insert(escolhido);
        dom.erase(escolhido);
        for (char w : vizinhos(G, escolhido)) {
            if (D.count(w) == 0) dom[w] = 1;
        }
    }

    // Converte para vetor ordenado
    vector<char> Dout(D.begin(), D.end());
    std::sort(Dout.begin(), Dout.end());
    bool ok = verificaPDS(G, Dout);
    return PDSResultado{ ok ? Dout : vector<char>{}, ok };
}

PDSResultado DominacaoPerfeita::guloso(Grafo* G, IterationTracker* tracker) 
{
    // vector<char> ids;
    // for (No* no : G->get_lista_adj()) {
    //     ids.push_back(no->get_id());
    // }
    // sort(ids.begin(), ids.end());
    
    // unordered_set<char> D;
    // unordered_map<char, int> dom;
    // unordered_map<char, int> deg;
    
    // for (char v : ids) {
    //     deg[v] = static_cast<int>(G->get_vizinhanca(v).size());
    //     if (deg[v] == 0) D.insert(v);
    //     else dom[v] = 0;
    // }
    //if (tracker) tracker->record(it + 1, best);
    // auto factivel = [&](char u) {
    //     for (Aresta* a : G->get_vizinhanca(u)) {
    //         char w = a->id_no_alvo;
    //         if (D.count(w)) continue;
    //         if (dom[w] >= 1) return false;
    //     }
    //     return true;
    // };

    // auto ganho = [&](char u) {
    //     int g = 0;
    //     for (Aresta* a : G->get_vizinhanca(u)) {
    //         char w = a->id_no_alvo;
    //         if (D.count(w)) continue;
    //         if (dom[w] == 0) g++;
    //     }
    //     return g;
    // };

    // while (true) {
    //     // Promoção anti-deadlock
    //     char w_prom = 0;
    //     for (char w : ids) {
    //         if (D.count(w) || dom[w] != 1) continue;
    //         if (factivel(w) && ganho(w) > 0) {
    //             w_prom = w;
    //             break;
    //         }
    //     }
        
    //     if (w_prom) {
    //         D.insert(w_prom);
    //         dom.erase(w_prom);
    //         for (Aresta* a : G->get_vizinhanca(w_prom)) {
    //             char z = a->id_no_alvo;
    //             if (!D.count(z)) dom[z] = 1;
    //         }
    //         continue;
    //     }

    //     // Lista de candidatos
    //     vector<pair<char, int>> cand;
    //     for (char u : ids) {
    //         if (D.count(u)) continue;
    //         if (!factivel(u)) continue;
    //         int g = ganho(u);
    //         if (g > 0) cand.push_back({u, g});
    //     }

    //     if (cand.empty()) {
    //         break;
    //     }

    //     // Escolha gulosa
    //     sort(cand.begin(), cand.end(), [](auto& a, auto& b) {
    //         return (a.second != b.second) ? (a.second > b.second) : (a.first < b.first);
    //     });
    //     char escolhido = cand[0].first;

    //     // Atualização
    //     D.insert(escolhido);
    //     dom.erase(escolhido);
    //     for (Aresta* a : G->get_vizinhanca(escolhido)) {
    //         char w = a->id_no_alvo;
    //         if (!D.count(w)) dom[w] = 1;
    //     }
    // }

    // vector<char> Dout(D.begin(), D.end());
    // sort(Dout.begin(), Dout.end());
    // bool ok = verificaPDS(G, Dout);
    // return {Dout, ok};


    //                         cout << "[DEBUG] Guloso PDS iniciado\n";
    std::mt19937 rng(123); // determinístico
    return construcao(G, rng, -1.0); // alpha<0 => guloso puro
}

PDSResultado DominacaoPerfeita::grasp(Grafo* G, int iteracoes, double alpha, 
    IterationTracker* tracker, unsigned seed) 
{
    // std::mt19937 rng(seed);
    // PDSResultado best; //best.factivel=false;
    
    // for (int it=0; it<iteracoes; ++it) 
    // {
    //     auto res = construcao(G, rng, alpha);
    //     if (res.factivel && (!best.factivel || res.custo() < best.custo())) 
    //     {
    //         best = res;
    //     }
    //     if (tracker) tracker->record(it + 1, best);
    // }
    // return best;
    std::mt19937 rng(seed);
    PDSResultado best; best.factivel=false;
    for (int it=0; it<iteracoes; ++it) {
        auto res = construcao(G, rng, alpha);
        if (res.factivel && (!best.factivel || res.custo() < best.custo())) {
            best = res;
        }
        if (tracker) tracker->record(it + 1, best);
    }
    return best;
}

PDSResultado DominacaoPerfeita::reativo(Grafo* G, int iteracoes,
                                        const vector<double>& alphas, int bloco, 
                                        IterationTracker* tracker,
                                        unsigned seed) 
{
    // std::mt19937 rng(seed);
    // int k = (int)alphas.size();
    // vector<double> prob(k, 1.0/k);
    // vector<double> score(k, 0.0); // qualidade média por alpha
    // vector<int> cont(k, 0);

    // auto escolhe_idx = [&](){
    //     std::discrete_distribution<int> dist(prob.begin(), prob.end());
    //     return dist(rng);
    // };

    // PDSResultado best; //best.factivel=false;

    // for (int it=1; it<=iteracoes; ++it) {
    //     int idx = escolhe_idx();
    //     double alpha = alphas[idx];
    //     auto res = construcao(G, rng, alpha);

    //     if (res.factivel) {
    //         // qualidade = 1 / |D| (menor D é melhor)
    //         double q = 1.0 / std::max(1, res.custo());
    //         score[idx] += q;
    //         cont[idx]  += 1;
    //         if (!best.factivel || res.custo() < best.custo()) best = res;
    //     }

    //     if (tracker) tracker->record(it, best);

    //     // Atualiza probabilidades a cada "bloco" iterações
    //     if (it % bloco == 0) {
    //         vector<double> media(k, 0.0);
    //         double soma = 0.0;

    //         for (int i=0;i<k;i++){
    //             media[i] = (cont[i] ? score[i]/cont[i] : 0.0);
    //             soma += media[i];
    //         }
            
    //         if (soma > 0.0) {
    //             for (int i=0;i<k;i++) prob[i] = media[i] / soma;
    //         } else {
    //             std::fill(prob.begin(), prob.end(), 1.0/k);
    //         }
    //         // “esquecer” um pouco para continuar adaptando
    //         std::fill(score.begin(), score.end(), 0.0);
    //         std::fill(cont.begin(),  cont.end(),  0);
    //     }
    // }
    // return best;
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

        if (tracker) tracker->record(it, best);

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

EvaluationResult evaluate_algorithm(
    Grafo* G, 
    std::function<PDSResultado(Grafo*)> algorithm, 
    int trials = 10) 
{
    EvaluationResult res = {0, 0, 0};
    int successes = 0;
    int total_size = 0;
    
    for (int i = 0; i < trials; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        auto result = algorithm(G);
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> elapsed = end - start;
        res.avg_time += elapsed.count();
        
        if (result.factivel) {
            successes++;
            total_size += result.custo();
        }
    }
    
    res.avg_time /= trials;
    res.success_rate = (double)successes / trials;
    res.avg_size = successes > 0 ? (double)total_size / successes : 0;
    
    return res;
}

void run_evaluations(const std::vector<Grafo*>& test_graphs) {
    std::ofstream out("temp_exec.csv");
    out << "Grafo,Vertices,Arestas,Algoritmo,Tempo,Tamanho,TaxaSucesso\n";
    
    for (Grafo* G : test_graphs) {
        int n = G->get_lista_adj().size();
        int m = 0;
        for (No* no : G->get_lista_adj()) {
            m += G->get_vizinhanca(no->get_id()).size();
        }
        if (G->get_direcionado()) m /= 2;
        
        // Testa guloso deterministico
        auto res_greedy = evaluate_algorithm(G, [](Grafo* g) { 
            return DominacaoPerfeita::guloso(g); 
        });
        out << "G" << n << "," << n << "," << m << ",Guloso,"
            << res_greedy.avg_time << "," << res_greedy.avg_size << "," 
            << res_greedy.success_rate << "\n";
            
        // Testa GRASP com alfas diferentes
        for (double alpha : {0.0, 0.25, 0.5, 0.75, 1.0}) {
            auto res_grasp = evaluate_algorithm(G, [alpha](Grafo* g) { 
                return DominacaoPerfeita::grasp(g, 100, alpha); 
            });
            out << "G" << n << "," << n << "," << m << ",GRASP-alpha" << alpha << ","
                << res_grasp.avg_time << "," << res_grasp.avg_size << "," 
                << res_grasp.success_rate << "\n";
        }
        
        // testa GRASP reativo
        auto res_reativo = evaluate_algorithm(G, [](Grafo* g) { 
            return DominacaoPerfeita::reativo(g, 100); 
        });
        out << "G" << n << "," << n << "," << m << ",Reativo,"
            << res_reativo.avg_time << "," << res_reativo.avg_size << "," 
            << res_reativo.success_rate << "\n";
    }
}

void DominacaoPerfeita::write_runtime_csv(const RuntimeData& data, const std::string& filename) {
    std::ofstream out(filename);
    out << "TamanhoGrafo,TempoGuloso,TempoGRASP,TempoReactive\n";
    
    for (size_t i = 0; i < data.graph_sizes.size(); ++i) {
        out << data.graph_sizes[i] << ","
            << data.greedy_times[i] << ","
            << data.grasp_times[i] << ","
            << data.reactive_times[i] << "\n";
    }
}

void write_iteration_csv(const IterationData& data, const std::string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing\n";
        return;
    }
    out << "Iterações,QualidadeGuloso,QualidadeGRASP,QualidadeReativo\n";
    
    for (size_t i = 0; i < data.iterations.size(); ++i) {
        out << data.iterations[i] << ","
            << data.greedy_qualities[i] << ","
            << data.grasp_qualities[i] << ","
            << data.reactive_qualities[i] << "\n";
    }
}

void DominacaoPerfeita::write_alpha_csv(const AlphaPerformance& data, const std::string& filename) {
    std::ofstream out(filename);
    out << "Alpha,Quality,Time\n";
    
    for (size_t i = 0; i < data.alpha_values.size(); ++i) {
        out << data.alpha_values[i] << ","
            << data.solution_qualities[i] << ","
            << data.runtimes[i] << "\n";
    }
}



PDSResultado grasp_with_tracking(Grafo* G, int iteracoes, double alpha, 
                                IterationTracker& tracker, 
                                unsigned seed = std::random_device{}()) {
    std::mt19937 rng(seed);
    PDSResultado best; best.factivel = false;
    
    for (int it = 0; it < iteracoes; ++it) {
        auto res = DominacaoPerfeita::construcao(G, rng, alpha);
        if (res.factivel && (!best.factivel || res.custo() < best.custo())) {
            best = res;
        }
        tracker.record(it + 1, best);
    }
    return best;
}

PDSResultado reativo_with_tracking(Grafo* G, int iteracoes,
                                 const std::vector<double>& alphas, int bloco,
                                 IterationTracker& tracker,
                                 unsigned seed = std::random_device{}()) {
    std::mt19937 rng(seed);
    int k = (int)alphas.size();
    std::vector<double> prob(k, 1.0/k);
    std::vector<double> score(k, 0.0);
    std::vector<int> cont(k, 0);

    auto escolhe_idx = [&](){
        std::discrete_distribution<int> dist(prob.begin(), prob.end());
        return dist(rng);
    };

    PDSResultado best; 
    best.factivel = false;

    for (int it = 1; it <= iteracoes; ++it) {
        int idx = escolhe_idx();
        double alpha = alphas[idx];
        auto res = DominacaoPerfeita::construcao(G, rng, alpha);
        
        if (res.factivel) {
            double q = 1.0 / std::max(1, res.custo());
            score[idx] += q;
            cont[idx] += 1;
            
            if (!best.factivel || res.custo() < best.custo()) {
                best = res;
            }
        }
        
        // guarda o melhor de cada iteracao
        tracker.record(it, best);
        
        // atualiza probabilidades para cada iteracao de bloco
        if (it % bloco == 0) {
            std::vector<double> media(k, 0.0);
            double soma = 0.0;
            
            for (int i = 0; i < k; i++) {
                media[i] = (cont[i] ? score[i]/cont[i] : 0.0);
                soma += media[i];
            }
            
            if (soma > 0.0) {
                for (int i = 0; i < k; i++) {
                    prob[i] = media[i] / soma;
                }
            } else {
                std::fill(prob.begin(), prob.end(), 1.0/k);
            }
            
            // reseta valores
            std::fill(score.begin(), score.end(), 0.0);
            std::fill(cont.begin(), cont.end(), 0);
        }
    }
    
    return best;
}

void DominacaoPerfeita::run_quality_experiment(Grafo* G) {
    const int max_iterations = 100;
    const int step = 5; // A cada 5 iteracoes
    const std::vector<double> alphas = {0.0, 0.25, 0.5, 0.75, 1.0};
    const int block_size = 10;
    
    // reastreia cada algoritmo
    IterationTracker greedy_track, grasp_track, reactive_track;
    
    // Testa guloso
    auto greedy_res = DominacaoPerfeita::guloso(G);
    greedy_track.record(1, greedy_res);
    
    // Testa GRASP
    auto grasp_res = grasp_with_tracking(G, max_iterations, 0.5, grasp_track);
    
    // Test Reactive (would need similar tracking modification)
    auto reactive_res = reativo_with_tracking(G, max_iterations, alphas, 
                                          block_size, reactive_track);
    
    // Prepara dados para tabela
    IterationData data;
    for (int i = 0; i <= max_iterations; i += step) {
        data.iterations.push_back(i);
        
        // pega qualidade mais proxima para cada alg
        auto greedy_it = std::lower_bound(greedy_track.iterations.begin(), 
                                         greedy_track.iterations.end(), i);
        if (greedy_it != greedy_track.iterations.end()) {
            size_t idx = greedy_it - greedy_track.iterations.begin();
            data.greedy_qualities.push_back(greedy_track.qualities[idx]);
        } else {
            data.greedy_qualities.push_back(0); // ou greedy_track.qualities.back()
        }
        
        auto grasp_it = std::lower_bound(grasp_track.iterations.begin(),
                                       grasp_track.iterations.end(), i);
        if (grasp_it != grasp_track.iterations.end()) {
            size_t idx = grasp_it - grasp_track.iterations.begin();
            data.grasp_qualities.push_back(grasp_track.qualities[idx]);
        } else {
            data.grasp_qualities.push_back(0);
        }

        auto reactive_it = std::lower_bound(reactive_track.iterations.begin(),
                                         reactive_track.iterations.end(), i);
        if (reactive_it != reactive_track.iterations.end()) {
            size_t idx = reactive_it - reactive_track.iterations.begin();
            data.reactive_qualities.push_back(reactive_track.qualities[idx]);
        } else {
            data.reactive_qualities.push_back(0);
        }
    }
    
    write_iteration_csv(data, "qualidade_vs_iteracoes.csv");
}

void run_reactive_experiment(Grafo* G) {
    const int max_iterations = 100;
    const std::vector<double> alphas = {0.0, 0.25, 0.5, 0.75, 1.0};
    const int block_size = 10;
    const int step = 5;
    
    IterationTracker reactive_track;

    auto reactive_res = reativo_with_tracking(G, max_iterations, alphas, 
                                           block_size, reactive_track);

    IterationData data;

    int last_iteration = std::min(max_iterations, 
        reactive_track.iterations.empty() ? 0 : reactive_track.iterations.back());

    
    for (int i = 0; i <= max_iterations; i += step) {
        data.iterations.push_back(i);
        
        auto it = std::lower_bound(reactive_track.iterations.begin(),
                                 reactive_track.iterations.end(), i);
        
        if (it != reactive_track.iterations.end()) {
            size_t idx = it - reactive_track.iterations.begin();
            data.reactive_qualities.push_back(reactive_track.qualities[idx]);
        } else if (!reactive_track.iterations.empty()) {
            data.reactive_qualities.push_back(reactive_track.qualities.back());
        } else {
            // nenhum dado disponivel
            data.reactive_qualities.push_back(0.0);
        }
    }
    
    std::ofstream alpha_log("probabilidades_alpha.csv");
    alpha_log << "Iteracao,Alpha0,Alpha25,Alpha50,Alpha75,Alpha100\n";

    write_iteration_csv(data, "qualidade_reativa.csv");

    std::cout << "Resultado Experimento GRASP Reativo:\n";
    std::cout << "--------------------------------\n";
    std::cout << "Melhor tamanho de solução: " << reactive_res.custo() << "\n";
    std::cout << "Solução válida: " << (reactive_res.factivel ? "SIM" : "NÃO") << "\n";
    if (!reactive_track.qualities.empty()) {
        std::cout << "Qualidade Final: " << reactive_track.qualities.back() << "\n";
    }

}




// Todas as implementações das funções da classe DominacaoPerfeita
// (manter todas as funções que estavam aqui originalmente)

// Adicionar esta implementação no final:
void DominacaoPerfeita::write_iteration_csv(const IterationData& data, const std::string& filename) {
    ofstream out(filename);
    if (!out) {
        cerr << "Erro ao abrir arquivo " << filename << " para escrita\n";
        return;
    }
    
    out << "Iteration,GreedyQuality,GRASPQuality,ReactiveQuality\n";
    for (size_t i = 0; i < data.iterations.size(); i++) {
        out << data.iterations[i] << ","
            << data.greedy_qualities[i] << ","
            << data.grasp_qualities[i] << ","
            << data.reactive_qualities[i] << "\n";
    }
}

// void write_iteration_csv(const IterationData& data, const std::string& filename) {
//     std::ofstream out(filename);
//     if (!out.is_open()) {
//         std::cerr << "Error: Could not open file " << filename << " for writing\n";
//         return;
//     }
    
//     // Write header
//     out << "Iteration,Quality\n";
    
//     // Write data rows
//     for (size_t i = 0; i < data.iterations.size() && i < data.reactive_qualities.size(); ++i) {
//         out << data.iterations[i] << "," << data.reactive_qualities[i] << "\n";
//     }
    
//     out.close();
// }
