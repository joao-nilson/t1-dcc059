import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv('performance_alpha.csv')

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10,10))

ax1.plot(data['Alpha'], data['Qualidade'], 'o-')
ax1.set_xlabel('Valor Alpha')
ax1.set_ylabel('Qualidade da Solução (1/set size)')
ax1.set_title('Análise de Parâmetro GRASP')
ax1.grid(True)

ax2.plot(data['Alpha'], data['Tempo'], 's-', color='orange')
ax2.set_xlabel('Valor Alpha')
ax2.set_ylabel('Tempo de Execução (segundos)')
ax2.grid(True)

plt.tight_layout()
plt.savefig('alpha_performance.png', dpi=300)
plt.close()
