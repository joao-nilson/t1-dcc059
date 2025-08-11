import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv('qualidade_vs_iteracoes.csv')

plt.figure(figsize=(10,6))
plt.plot(data['Iterações'], data['QualidadeGuloso'], 'o-', label='Guloso')
plt.plot(data['Iterações'], data['QualidadeGRASP'], 's-', label='GRASP')
plt.plot(data['Iterações'], data['QualidadeReativo'], 'd-', label='Reativo')

plt.xlabel('Iterações')
plt.ylabel('Qualidade da solução (1/set size)')
plt.title('Qualidade da solução vs Iterações')
plt.grid(True)
plt.legend()
plt.savefig('qualidade_vs_iteracoes.png', dpi=300)
plt.close()
