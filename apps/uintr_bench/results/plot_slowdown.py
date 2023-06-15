import matplotlib.pyplot as plt
from matplotlib import ticker
import pandas as pd 

color = ['#1F77B4', '#FF7F0E', '#38A538', '#D62728']
# color = ['#56B4E9', '#CC7AA8', '#E69600', '#1F77B4', '#FF7F0E', '#38A538', '#D62728']
T = [100000, 10000, 1000, 100, 50, 20, 10, 5, 4, 3, 2, 1]
Tname = ['baseline', '10ms', '1ms', '100us', '50us', '20us', '10us', '5us', '4us', '3us', '2us', '1us']

def get_exe(work_spec):
    df = pd.read_csv('{}/{}.csv'.format(work_spec, work_spec))
    return df['execution_time']

def merge(merged, new):
    for i in range(len(merged)):
        merged[i] += new[i]
    return merged


def plot_slowdown(work_specs, title):
    plt.figure(figsize=(8, 5))

    for i in range(len(work_specs)):
        work_spec = work_specs[i]
        merged = [0 for j in range(len(T))]
        benches = work_spec.split('+')
        for bench in benches:
            merged = merge(merged, get_exe(bench))  
        # if 'base64' in work_spec:
        #     merged = merge(merged, get_exe('base64'))
        # if 'matmul' in work_spec:
        #     merged = merge(merged, get_exe('matmul'))
        # if 'linpack' in work_spec:
        #     merged = merge(merged, get_exe('linpack'))
        exe = get_exe(work_spec)
        slowdown = [exe[j]/merged[j]-1 for j in range(len(T))]
        plt.plot(T, slowdown, linewidth=1, marker='o', markersize=4, c=color[i], markerfacecolor='none', label=work_spec)
        
    plt.xscale('log')
    plt.legend()
    plt.xlim(max(T) * 2, min(T) / 2)
    # plt.ylim(bottom=0, top=0.25)
    plt.gca().yaxis.set_major_formatter(ticker.PercentFormatter(xmax=1))
    plt.xticks(T, Tname, fontsize=8)
    plt.xlabel('Timeslice')
    plt.ylabel('Slowdown')
        
    # Show the plot
    plt.tight_layout()
    plt.savefig('slowdown/{}.pdf'.format(title))
    plt.show()

# work_specs, title = ['sum', 'sum+sum', 'sum+sum+sum'], 'sum'
# work_specs, title = ['base64', 'base64+base64', 'base64+base64+base64', 'base64+base64+base64+base64'], 'base64'
# work_specs, title = ['matmul', 'matmul+matmul', 'matmul+matmul+matmul', 'matmul+matmul+matmul+matmul'], 'matmul'
# work_specs, title = ['linpack', 'linpack+linpack', 'linpack+linpack+linpack', 'linpack+linpack+linpack+linpack'], 'linpack'
# work_specs, title = ['base64+matmul', 'base64+linpack', 'matmul+linpack', 'base64+matmul+linpack'], 'all'
# work_specs, title = ['base64+base64', 'matmul+matmul', 'linpack+linpack'], '2combine'
# work_specs, title = ['base64+base64+base64', 'matmul+matmul+matmul', 'linpack+linpack+linpack'], '3combine'    
work_specs, title = ['base64+sum', 'matmul+sum', 'linpack+sum'], 'sum+others'

plot_slowdown(work_specs, title)
