#include "Gerenciador.h"
#include <fstream>

// Impressao e escrita - A,B,C,D - Início

static int contador_ordem_arquivo = 0;

void imprimir_vetor_char(const vector<char> &vetor)
{
    for (size_t i = 0; i < vetor.size(); ++i)
    {
        cout << vetor[i];
        if (i != vetor.size() - 1)
        {
            cout << ",";
        }
    }
    cout << endl;
}

void salvar_vetor_char_em_arquivo(const vector<char> &vetor, const string &nome_arquivo)
{
    int *ponteiroContador = &contador_ordem_arquivo;
    (*ponteiroContador)++;
    string caminho_completo = "../output/" + to_string(contador_ordem_arquivo) + nome_arquivo;

    ofstream arquivo(caminho_completo);

    if (!arquivo.is_open())
    {
        cout << "Erro ao abrir o arquivo para escrita." << endl;
        return;
    }

    for (size_t i = 0; i < vetor.size(); ++i)
    {
        arquivo << vetor[i];
        if (i != vetor.size() - 1)
        {
            arquivo << ",";
        }
    }
    arquivo << endl;
    arquivo.close();
}

// Impressao e escrita - A,B,C,D - Fim

// Impressao e escrita E,F - Início

void imprimir_lista_adjacencias(Grafo *grafo)
{
    vector<No*> lista = grafo->get_lista_adj();

    for (No* no : lista)
    {
        cout << no->get_id() << ":";
        vector<Aresta*> arestas = grafo->get_vizinhanca(no->get_id());

        bool primeiro = true;
        for (Aresta* a : arestas)
        {
            cout << (primeiro ? " " : " -> ") << a->id_no_alvo;
            primeiro = false;
        }
        cout << endl;
    }
}

void salvar_lista_adjacencias_em_arquivo(const string &nome_arquivo, Grafo *grafo)
{
    
    string caminho_completo = "../output/" + to_string(contador_ordem_arquivo) + nome_arquivo;
    ofstream arquivo(caminho_completo);

    if (!arquivo.is_open())
    {
        cerr << "Erro ao abrir o arquivo " << nome_arquivo << endl;
        return;
    }

    vector<No*> lista = grafo->get_lista_adj();

    for (No* no : lista)
    {
        arquivo << no->get_id() << ":";
        vector<Aresta*> arestas = grafo->get_vizinhanca(no->get_id());

        bool primeiro = true;
        for (Aresta* a : arestas)
        {
            arquivo << (primeiro ? " " : " -> ") << a->id_no_alvo;
            primeiro = false;
        }
        arquivo << endl;
    }

    arquivo.close();
}
// Impressao e escrita E,F - Fim

void Gerenciador::comandos(Grafo *grafo)
{
    cout << "Digite uma das opcoes abaixo e pressione enter:" << endl
         << endl;
    cout << "(a) Fecho transitivo direto de um no;" << endl;
    cout << "(b) Fecho transitivo indireto de um no;" << endl;
    cout << "(c) Caminho minimo (Djikstra);" << endl;
    cout << "(d) Caminho minimo (Floyd);" << endl;
    cout << "(e) Arvore Geradora Minima (Algoritmo de Prim);" << endl;
    cout << "(f) Arvore Geradora Minima (Algoritmo de Kruskal);" << endl;
    cout << "(g) Arvore de caminhamento em profundidade;" << endl;
    cout << "(h) Raio, diametro, centro e periferia do grafo;" << endl;
    cout << "(0) Sair;" << endl
         << endl;

    string entrada;
    cout << "Escolha: " << endl;
    cin >> entrada;

    if (entrada.size() != 1)
    {
        cout << endl;
        cout << endl;
        cout << "Por favor, digite apenas uma letra." << endl;
        cout << endl;
        comandos(grafo); // volta para o menu
        return;
    }

    char resp = entrada[0];

    switch (resp)
    {
    case 'a':
    {

        char id_no = get_id_entrada();
        vector<char> fecho_transitivo_direto = grafo->fecho_transitivo_direto(id_no);

        cout << "Fecho transitivo direto de " << id_no << ": ";
        imprimir_vetor_char(fecho_transitivo_direto);
        cout << endl;

        // Impressão no arquivo
        if (pergunta_imprimir_arquivo("fecho_trans_dir.txt"))
        {
            salvar_vetor_char_em_arquivo(fecho_transitivo_direto, "fecho_trans_dir.txt");
            cout << "Fecho salvo no arquivo fecho_trans_dir.txt" << endl
                 << endl;
        }

        break;
    }

    case 'b':
    {
        char id_no = get_id_entrada();
        vector<char> fecho_transitivo_indireto = grafo->fecho_transitivo_indireto(id_no);

        cout << "Fecho transitivo indireto de " << id_no << ": ";
        imprimir_vetor_char(fecho_transitivo_indireto);
        cout << endl;

        // Impressão no arquivo
        if (pergunta_imprimir_arquivo("fecho_trans_indir.txt"))
        {
            salvar_vetor_char_em_arquivo(fecho_transitivo_indireto, "fecho_trans_indir.txt");
            cout << "Fecho salvo no arquivo fecho_trans_indir.txt" << endl
                 << endl;
        }

        break;
    }

    case 'c':
    {

        char id_no_1 = get_id_entrada();
        char id_no_2 = get_id_entrada();
        vector<char> caminho_minimo_dijkstra = grafo->caminho_minimo_dijkstra(id_no_1, id_no_2);

        cout << "Caminho mínimo (Dijkstra): ";
        imprimir_vetor_char(caminho_minimo_dijkstra);
        cout << endl;

        // Impressão no arquivo
        if (pergunta_imprimir_arquivo("caminho_minimo_dijkstra.txt"))
        {
            salvar_vetor_char_em_arquivo(caminho_minimo_dijkstra, "caminho_minimo_dijkstra.txt");
        }

        break;
    }

    case 'd':
    {

        char id_no_1 = get_id_entrada();
        char id_no_2 = get_id_entrada();
        vector<char> caminho_minimo_floyd = grafo->caminho_minimo_floyd(id_no_1, id_no_2);

        cout << "Caminho mínimo (Floyd): ";

        imprimir_vetor_char(caminho_minimo_floyd);

        cout << endl;

        // Impressão no arquivo
        if (pergunta_imprimir_arquivo("caminho_minimo_floyd.txt"))
        {
            salvar_vetor_char_em_arquivo(caminho_minimo_floyd, "caminho_minimo_floyd.txt");
        }

        break;
    }

    case 'e':
    {
        int tam;
        cout << "Digite o tamanho do subconjunto: ";
        cin >> tam;

        if (tam > 0 && tam <= grafo->get_ordem())
        {
            vector<char> ids = get_conjunto_ids(grafo, tam);
            Grafo *agm = grafo->arvore_geradora_minima_prim(ids);

            cout << "Árvore Geradora Mínima (Prim):\n";
            imprimir_lista_adjacencias(agm);

            if (pergunta_imprimir_arquivo("agm_prim.txt"))
            {
                salvar_lista_adjacencias_em_arquivo("agm_prim.txt", agm);
            }

            delete agm;
        }
        else
        {
            cout << "Valor inválido\n";
        }

        break;
    }

    case 'f':
    {

        int tam;
        cout << "Digite o tamanho do subconjunto: ";
        cin >> tam;

        if (tam > 0 && tam <= grafo->get_ordem())
        {

            vector<char> ids = get_conjunto_ids(grafo, tam);
            Grafo *arvore_geradora_minima_kruskal = grafo->arvore_geradora_minima_kruskal(ids);
            cout << "Metodo de impressao em tela nao implementado" << endl
                 << endl;

            if (pergunta_imprimir_arquivo("agm_kruskal.txt"))
            {
                cout << "Metodo de impressao em arquivo nao implementado" << endl;
            }

            delete arvore_geradora_minima_kruskal;
        }
        else
        {
            cout << "Valor invalido" << endl;
        }

        break;
    }

    case 'g':
    {
        char id_no = get_id_entrada();
        Grafo *arvore_caminhamento_profundidade = grafo->arvore_caminhamento_profundidade(id_no);

        if (arvore_caminhamento_profundidade != nullptr)
        {
            cout << endl << "Arvore de caminhamento em profundidade resultante:" << endl;
            imprimir_lista_adjacencias(arvore_caminhamento_profundidade);
            cout << endl;

            if (pergunta_imprimir_arquivo("arvore_caminhamento_profundidade.txt"))
            {
                salvar_lista_adjacencias_em_arquivo("arvore_caminhamento_profundidade.txt", arvore_caminhamento_profundidade);
                cout << "Arvore salva no arquivo arvore_caminhamento_profundidade.txt" << endl << endl;
            }

            delete arvore_caminhamento_profundidade;
        }
        
        break;
    }

    case 'h':
    {
        vector<char> articulacao = grafo->vertices_de_articulacao();
        cout << "Metodo de impressao em tela nao implementado" << endl
             << endl;

        if (pergunta_imprimir_arquivo("arvore_caminhamento_profundidade.txt"))
        {
            cout << "Metodo de impressao em arquivo nao implementado" << endl;
        }

        break;
    }
    case 'i':
    {

        vector<char> articulacao = grafo->vertices_de_articulacao();
        cout << "Metodo de impressao em tela nao implementado" << endl
             << endl;

        if (pergunta_imprimir_arquivo("arvore_caminhamento_profundidade.txt"))
        {
            cout << "Metodo de impressao em arquivo nao implementado" << endl;
        }

        break;
    }

    case '0':
    {
        exit(0);
    }
    default:
    {
        cout << "Opção inválida" << endl;
    }
    }

    comandos(grafo);
}

char Gerenciador::get_id_entrada()
{
    cout << "Digite o id de um no: ";
    char id;
    cin >> id;
    cout << endl;

    return id;
}

vector<char> Gerenciador::get_conjunto_ids(Grafo *grafo, int tam)
{
    vector<char> ids = {};
    while ((int)ids.size() < tam)
    {
        char id_no = get_id_entrada();
        bool existe = false;
        for (No *no : grafo->get_lista_adj())
        {
            if (no->get_id() == id_no)
            {
                existe = true;
                break;
            }
        }

        if (!existe)
        {
            cout << "Vertice nao existe" << endl;
        }
        else
        {
            bool repetido = find(ids.begin(), ids.end(), id_no) != ids.end();
            if (repetido)
            {
                cout << "Valor repetido" << endl;
            }
            else
            {
                ids.push_back(id_no);
            }
        }
    }

    return ids;
}

bool Gerenciador::pergunta_imprimir_arquivo(string nome_arquivo)
{

    cout << "Imprimir em arquivo externo? (" << nome_arquivo << ")" << endl;
    cout << "(1) Sim;" << endl;
    cout << "(2) Nao." << endl;
    int resp;
    cin >> resp;
    cout << endl;

    switch (resp)
    {
    case 1:
        return true;
    case 2:
        return false;
    default:
        cout << "Resposta invalida" << endl;
        return pergunta_imprimir_arquivo(nome_arquivo);
    }
}