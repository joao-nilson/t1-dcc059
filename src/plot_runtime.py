import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

data = pd.read_csv('temp_exec.csv')

plt.figure(figsize=(10,6))
plt.loglog(data['TamanhoGrafo'], data['TempoGuloso'], 'o-', label='Guloso')
plt.loglog(data['TamanhoGrafo'], data['TempoGRASP'], 's-', label='GRASP')
plt.loglog(data['TamanhoGrafo'], data['TempoReactive'], 'd-', label='Reativo')

plt.xlabel('Tamanho Grafo (escala log)')
plt.ylabel('Tempo de Execução (segundos, escala log)')
plt.title('Tempo de Execução vs Tamanho Grafo (log-log)')
plt.grid(True, which="both", ls="-")
plt.legend()
plt.savefig('tempExev_vs_Tam.png', dpi=300)
plt.close()
