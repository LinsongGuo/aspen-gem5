import matplotlib.pyplot as plt
from matplotlib import ticker
import pandas as pd 
import math 

# color = ['#1F77B4', '#FF7F0E', '#38A538', '#D62728']
color = ['#56B4E9', '#CC7AA8', '#E69600', '#1F77B4', '#FF7F0E', '#38A538', '#D62728']
T = [100000, 10000, 5000, 1000, 500, 100, 50, 20, 10, 7, 5] # , 3, 1]
Tname= ['baseline', '10ms', '5ms', '1ms', '500us', '100us', '50us', '20us', '10us', '7us', '5us']# , '3us', '1us']
S = 3

def get_exe(work_spec):
    df = pd.read_csv('{}/{}.csv'.format(work_spec, work_spec))
    return df['execution_time']

def merge(merged, new):
    for i in range(len(merged)):
        merged[i] += new[0]
    return merged


def plot_slowdown(work_specs, title):
    plt.figure(figsize=(8, 5))
    
    maxy = 0
    for i in range(len(work_specs)):
        work_spec = work_specs[i]
        exe = get_exe(work_spec)
        slowdown = [exe[j]/exe[0]-1 for j in range(len(T))]
        # maxy = max(maxy, max(slowdown))
        plt.plot(T[S:], slowdown[S:], linewidth=1, marker='o', markersize=4, c=color[i], markerfacecolor='none', label=work_spec)
    
    plt.axhline(y=0.1, linestyle='dashed', linewidth=1, c='grey')
    yticks = []
    maxy = 0.31
    for t in range(0, math.ceil(maxy/0.05)):
        yticks.append(t*0.05)
        
    plt.xscale('log')
    plt.legend()
    plt.xlim(max(T[S:]) * 2, min(T[S:]) / 2)
    # plt.ylim(bottom=0, top=0.25)
    plt.gca().yaxis.set_major_formatter(ticker.PercentFormatter(xmax=1))
    plt.xticks(T[S:], Tname[S:], fontsize=8)
    plt.yticks(yticks)
    plt.xlabel('Timeslice')
    plt.ylabel('Slowdown Compared with Baseline(no preemptions)')
        
    # Show the plot
    plt.tight_layout()
    plt.savefig('slowdown/{}.pdf'.format(title))
    plt.show()

# work_specs, title = ['mcf', 'base64', 'matmul_int', 'mcf+base64', 'mcf+matmul_int', 'base64+matmul_int', 'mcf+base64+matmul_int'], 'all'
# work_specs, title = ['mcf', 'mcf+mcf', 'mcf+mcf+mcf', 'mcf+mcf+mcf+mcf'], 'mcf'
# work_specs, title = ['matmul_int', 'matmul_int+matmul_int', 'matmul_int+matmul_int+matmul_int', 'matmul_int+matmul_int+matmul_int+matmul_int'], 'matmul_int'
work_specs, title = ['base64', 'base64+base64', 'base64+base64+base64', 'base64+base64+base64+base64'], 'base64'

plot_slowdown(work_specs, title)
