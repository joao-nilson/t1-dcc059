Nomeação das instâncias:
g_<ordem do grafo>_<densidade de arestas>_<in_direcionado>_<in_orientado_aresta>_<#>.txt
g_5_0.40_0_0_0.txt


Formado das instâncias:

<direcionado> <ponderado_aresta> <ponderado_vertice>
<#_vertices>
<lista_vertices>
<lista_arestas>

-----------------------------------------

<direcionado> --> 1 se direcionado, 0 se não direcionado;
<ponderado_aresta> --> 1 se ponderado nas arestas, 0 se não ponderado nas arestas;
<ponderado_vertices> --> 1 se ponderado nos vértices> 0 se não ponderado nos vértices>
<#_vertices> --> Ordem so grafo
<lista_vertices>
    <id> - se não ponderado
    <id peso> - se ponderado
<lista_arestas>
    <id_a id_b> - se não ponderado
    <id_a id_b peso> - se ponderado

-----------------------------------------

Ex. 1:
0 1 1 
5
a 1
b 2
c 3
d 4
e 5
a b 5
b e 4
e c 3
d e 2
a e 1

Ex. 2:
0 0 0 
5
a
b
c
d
e
a b
b e
e c
d e
a e