import matplotlib.pyplot as plt
import pandas as pd 
import math 

# color = ['#1F77B4', '#FF7F0E', '#38A538',  '#D62728'] # blue, orange, green, red
# color = ['#56B4E9', '#CC7AA8', '#E69600', '#1F77B4', '#FF7F0E', '#38A538', '#D62728']
color = ['#56B4E9', '#CC7AA8', 'chocolate', '#1F77B4', '#FF7F0E', '#38A538', '#D62728']
# T = [10000, 1000, 100, 50, 20, 10, 5, 4, 3, 2, 1]
# Tname = ['10ms', '1ms', '100us', '50us', '20us', '10us', '5us', '4us', '3us', '2us', '1us']
T = [100000, 10000, 5000, 1000, 500, 100, 50, 20, 10, 7, 5, 3, 1]
Tname= ['baseline', '10ms', '5ms', '1ms', '500us', '100us', '50us', '20us', '10us', '7us', '5us', '3us', '1us']
S = 3

def get_overhead(work_spec):
    df = pd.read_csv('{}/{}.csv'.format(work_spec, work_spec))
    return df['overhead']

def plot_overhead(work_specs, title):
    plt.figure(figsize=(8, 5))
    maxy = 0
    
    for i in range(len(work_specs)):
        overhead = get_overhead(work_specs[i])
        plt.plot(T[S:], overhead[S:], linewidth=1, marker='o', markersize=4, c=color[i], markerfacecolor='none', label=work_specs[i])
        maxy = max(maxy, max(overhead[S:]))
    
    plt.axhline(y=0.5, linestyle='dashed', linewidth=1, c='grey')
    yticks = [0.5]
    for t in range(1, math.ceil(maxy/10)+1):
        yticks.append(t*10)
    print(maxy, yticks)
    
    plt.xscale('log')
    plt.legend()
    plt.xlim(max(T[S:]) * 2, min(T[S:]) / 2)
    plt.ylim(bottom=0)
    plt.xticks(T[S:], Tname[S:], fontsize=8)
    plt.yticks(yticks)
    plt.xlabel('Timeslice')
    plt.ylabel('Average Preemption Overhead (us)')
        
    # Show the plot
    plt.tight_layout()
    plt.savefig('overhead/{}.pdf'.format(title))
    plt.show()
  
work_specs, title = ['mcf', 'base64', 'matmul_int', 'mcf+base64', 'mcf+matmul_int', 'base64+matmul_int', 'mcf+base64+matmul_int'], 'all'
# work_specs, title = ['mcf', 'mcf+mcf', 'mcf+mcf+mcf', 'mcf+mcf+mcf+mcf'], 'mcf'
# work_specs, title = ['matmul_int', 'matmul_int+matmul_int', 'matmul_int+matmul_int+matmul_int', 'matmul_int+matmul_int+matmul_int+matmul_int'], 'matmul_int'
# work_specs, title = ['base64', 'base64+base64', 'base64+base64+base64', 'base64+base64+base64+base64'], 'base64'

plot_overhead(work_specs, title)
