import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from matplotlib.ticker import ScalarFormatter

color = ['#1F77B4', '#D62728', '#38A538','#FF7F0E'] # blue, red, green, orange
  
def plot(file, title):
    df = pd.read_csv(file)
    native = df['native'].to_list()[0]
    threads = df['uths'].to_list()
    caladan = df['caladan'].to_list()
    caladan_simd = df['caladan-simdreg'].to_list()
    caladan_simd_custom = df['caladan-simdreg-custom'].to_list()
    
    caladan = [x - native for x in caladan]
    caladan_simd = [x - native for x in caladan_simd]
    caladan_simd_custom = [x - native for x in caladan_simd_custom]
    
    plt.figure(figsize=(6, 3))
    plt.plot(threads, caladan, linewidth=0.8, marker='o', markersize=3, c=color[0], markerfacecolor='none', label='Caladan')
    plt.plot(threads, caladan_simd, linewidth=0.8, marker='o', markersize=3, c=color[1], markerfacecolor='none', label='Caladan w/ simdreg')
    plt.plot(threads, caladan_simd_custom, linewidth=0.8, marker='o', markersize=3, c=color[2], markerfacecolor='none', label='Caladan w/ custom simdreg')
    
    plt.legend(fontsize=10)
    # plt.ylim(bottom=0.2, top=0.35)
    plt.ylim(bottom=0, top=0.5)
    plt.xticks([1, 8, 16, 24, 32, 40, 48, 56, 64], fontsize=10)
    plt.yticks(fontsize=10)
    plt.xlabel('Number of user threads', fontsize=12)
    plt.ylabel('Yield overhead (us)', fontsize=12)
    plt.title('workload: {}'.format(title))
    
    plt.tight_layout()
    plt.savefig('{}.pdf'.format(title))
    plt.show()
    
plot('chase-1KB.csv', 'chase-1KB')
# plot('chase-8KB.csv', 'chase-8KB')

# plot('sum-1KB.csv', 'sum-1KB')
# plot('sum-8KB.csv', 'sum-KB')
# plot('sum-16KB.csv', 'sum-16KB')
