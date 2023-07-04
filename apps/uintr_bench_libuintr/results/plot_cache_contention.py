import matplotlib.pyplot as plt
from matplotlib import ticker
import pandas as pd 
import math 

# color = ['#1F77B4', '#FF7F0E', '#38A538', '#D62728']
color = ['#56B4E9', '#CC7AA8', '#E69600', '#1F77B4', '#FF7F0E', '#38A538', '#D62728']
# color = ['#1F77B4', '#FF7F0E', '#38A538', '#D62728']
T = [100000, 10000, 5000, 1000, 500, 100, 50, 20, 10, 7, 5, 3, 1]
Tname= ['baseline', '10ms', '5ms', '1ms', '500us', '100us', '50us', '20us', '10us', '7us', '5us', '3us', '1us']
start = 3
end = 3

uintr_handle_overhead = {'mcf': {'5us': 0.48899, '3us': 0.44707, '1us': 0.45383}, 
                        'base64': {'5us': 0.4154749, '3us': 0.417399, '1us': 0.41270}}

def get_exe(work_spec):
    df = pd.read_csv('{}/{}.csv'.format(work_spec, work_spec))
    return df['execution_time'].to_list()

def get_pre(work_spec):
    df = pd.read_csv('{}/{}.csv'.format(work_spec, work_spec))
    return df['#preemptions'].to_list()

def merge(merged, new):
    for i in range(len(merged)):
        merged[i] += new[i]
    return merged

def plot_slowdown(work_specs, title):
    plt.figure(figsize=(8, 5))
    
    maxy = 0
    for i in range(len(work_specs)):
        work_spec = work_specs[i]
        merged = [0 for j in range(len(T))]
        benches = work_spec.split('+')
        for bench in benches:
            merged = merge(merged, get_exe(bench))  
        exe = get_exe(work_spec)
        slowdown = [(exe[j]-merged[j])/(exe[j] - exe[0]) if j > 0 else 0 for j in range(len(T))]
        if len(benches) > 0:
            maxy = max(maxy, max(slowdown[start:-(end-1)] if end > 1 else slowdown[start:]))
            plt.plot(T[start:-(end-1)] if end > 1 else T[start:], slowdown[start:-(end-1)] if end > 1 else slowdown[start:], linewidth=1, marker='o', markersize=4, c=color[i], markerfacecolor='none', label=work_spec)
            # pre = get_pre(work_spec)
            # cache = slowdown[-end]
            # uintr = pre[-end] * uintr_handle_overhead[benches[0]][Tname[-end]] / 1e6 / (exe[-end] - exe[0])
            # swtch = pre[-end] * 0.06 / 1e6 / (exe[-end] - exe[0])
            # # plt.plot(T[-1], uintr, marker='*', markersize=4, c=color[i])
            # # plt.plot(T[-1], swtch, marker='^', markersize=4, c=color[i])
            # print('cache: {:.3}, uintr: {:.3}, swtch: {:.3}, total: {:.3}'.format(cache, uintr, swtch, cache+uintr+swtch))
            
            # cache_overhead = (exe[-end]-merged[-end]) / pre[-end] * (10**6)
            # uintr_overhead = uintr_handle_overhead[benches[0]][Tname[-end]]
            # swtch_overhead = 0.06
            # total_overhead = (exe[-end]-exe[0]) / pre[-end] * (10**6)
            # # print(exe[-end], pre[-end])
            # print('Overhead cache: {:.3}, uintr: {:.3}, swtch: {:.3}, c+u+s:{:.5}, total: {:.5}'.format(cache_overhead, uintr_overhead, swtch_overhead, cache_overhead+uintr_overhead+swtch_overhead, total_overhead))
            
    # plt.axhline(y=0.1, linestyle='dashed', linewidth=1, c='grey')
    # yticks = []
    # for t in range(0, math.ceil(maxy/0.1)):
    #     yticks.append(t*0.1)
        
    plt.xscale('log')
    plt.legend()
    plt.xlim(max(T[start:-(end-1)] if end > 1 else T[start:]) * 2, min(T[start:-(end-1)] if end > 1 else T[start:]) / 2)
    plt.ylim(bottom=-0.2)
    plt.gca().yaxis.set_major_formatter(ticker.PercentFormatter(xmax=1))
    plt.xticks(T[start:-(end-1)] if end > 1 else T[start:], Tname[start:-(end-1)] if end > 1 else Tname[start:], fontsize=8)
    # plt.yticks(yticks)
    plt.xlabel('Timeslice')
    plt.ylabel('Cache Contention Contribution%')
        
    # Show the plot
    plt.tight_layout()
    plt.savefig('cache_contention/{}.pdf'.format(title))
    plt.show()

# work_specs, title = ['mcf', 'base64', 'matmul_int', 'mcf+base64', 'mcf+matmul_int', 'base64+matmul_int', 'mcf+base64+matmul_int'], 'all'
# work_specs, title = ['mcf', 'mcf+mcf', 'mcf+mcf+mcf', 'mcf+mcf+mcf+mcf'], 'mcf'
# work_specs, title = ['matmul_int', 'matmul_int+matmul_int', 'matmul_int+matmul_int+matmul_int', 'matmul_int+matmul_int+matmul_int+matmul_int'], 'matmul_int'
# work_specs, title = ['base64', 'base64+base64', 'base64+base64+base64', 'base64+base64+base64+base64'], 'base64'
work_specs, title = ['mcf+base64', 'mcf+matmul_int', 'base64+matmul_int', 'mcf+base64+matmul_int'], 'all'

plot_slowdown(work_specs, title)
