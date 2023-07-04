import matplotlib.pyplot as plt
import pandas as pd 
import math
from matplotlib import ticker

uintr_handle_overhead = {'mcf': {'5us': 0.48899, '3us': 0.44707, '1us': 0.45383}, 
                        'base64': {'5us': 0.4154749, '3us': 0.417399, '1us': 0.41270},
                        'sum': {'5us': 0.408204},
                        'matmul_int': {'5us': 0.267505}}

def get_data(work_spec):
    df = pd.read_csv('{}/{}.csv'.format(work_spec, work_spec))
    timeslice = df['timeslice'].to_list()
    exe = df['execution_time'].to_list()
    pre = df['#preemptions'].to_list()
    over = df['overhead'].to_list()
    data = {}
    for i in range(len(timeslice)):
        data[timeslice[i]] = [exe[i], pre[i], over[i]]
    return data
    
def get_merge(work_spec, timeslice):
    merged = 0
    benches = work_spec.split('+')
    for bench in benches:
        tmp = get_data(bench)
        # print(bench, tmp, timeslice)
        merged += tmp[timeslice][0]
    return merged

def get_single_overhead(work_spec, timeslice):
    tmp = get_data(work_spec)
    return tmp[timeslice][2]

def plot(work_specs, names, timeslice):
    # plt.figure(figsize=(8, 5))
    
    uintr_percent = []
    swtch_percent = []
    cache_percent = []
    
    uintr_data = []
    swtch_data = []
    cache_data = []
    total_data = []
    
    for i in range(len(work_specs)):
        work_spec = work_specs[i]
        benches = work_spec.split('+')
        data = get_data(work_spec)
        
        baseline = data['baseline'][0]
        exe = data[timeslice][0]
        pre = data[timeslice][1] 
        overhead = data[timeslice][2]
        single = get_single_overhead(benches[0], timeslice)
        
        # print(baseline, exe, pre, merge)
        # cache = (exe - merge) / (exe - baseline)
        # cache = overhead - get_single_overhead(benches[0], timeslice)
        # uintr = pre * uintr_handle_overhead[benches[0]][timeslice] / 1e6 / (exe - baseline)
        # swtch = pre * 0.06 / 1e6 / (exe - baseline)
        # print('cache: {:.3}, uintr: {:.3}, swtch: {:.3}, total: {:.3}'.format(cache, uintr, swtch, cache+uintr+swtch))
        
        # print(single, overhead)
        cache_overhead = overhead - single
        uintr_overhead = uintr_handle_overhead[benches[0]][timeslice]
        swtch_overhead = 0.06
        total_overhead = (exe - baseline) / pre * (10**6)
        
        cache_percent.append(cache_overhead / total_overhead)
        uintr_percent.append(uintr_overhead / total_overhead)
        swtch_percent.append(swtch_overhead / total_overhead)
        
        cache_data.append(cache_overhead)
        uintr_data.append(uintr_overhead)
        swtch_data.append(swtch_overhead)
        total_data.append(total_overhead)
        
        print('{}: cache: {:.3}, uintr: {:.3}, swtch: {:.3}, c+u+s:{:.5}, total: {:.5}'.format(work_spec, cache_overhead, uintr_overhead, swtch_overhead, cache_overhead+uintr_overhead+swtch_overhead, total_overhead))
    
    percent = {'Cache Contention Overhead Percentage': cache_percent, \
            'Uintr Handling Overhead Percentage (includes pipeline flushing)': uintr_percent, \
            'Caladan Switch Overhead Percentage': swtch_percent}
    df = pd.DataFrame(percent)
    colors = ["#6A51A2", "#9D9BC7", "#DADBE8"]
    ax = df.plot(kind='bar', stacked=True, color=colors, figsize=(10, 5), edgecolor='black')
    ax.set_ylim(bottom=0, top=1.19)
    ax.set_xlabel('Workloads (Preemption Timeslice = 5us)')
    ax.set_xticks([i for i in range(0, len(work_specs))], names, rotation=0, fontsize=8)
    plt.axhline(y=1, linestyle='dashed', linewidth=1, c='grey')
    plt.gca().yaxis.set_major_formatter(ticker.PercentFormatter(xmax=1))
    plt.tight_layout()
    plt.savefig('breakup/{}.pdf'.format('percent'))
    plt.show()
    
    ymax = max(total_data)
    data = {'Caladan Switch Overhead': swtch_data,
            'Uintr Handling Overhead (includes pipeline flushing)': uintr_data,
            'Cache Contention Overhead': cache_data,
            "Overall Overhead": total_data}
    df = pd.DataFrame(data)
    colors = ["#DADBE8", "#9D9BC7", "#6A51A2","#E69600"]
    ax = df.plot(kind='bar', color=colors, figsize=(10, 5), edgecolor='black')
    plt.axhline(y=0.06, linestyle='dashed', linewidth=1, c='grey')
    # ax.set_ylim(bottom=0, top=1.19)
    yticks = [0, 0.06]
    for i in range(1, math.ceil(ymax/0.2)+1):
        yticks.append(0.2 * i)
    ax.set_yticks(yticks)
    ax.set_xlabel('Workloads (Preemption Timeslice = 5us)')
    ax.set_xticks([i for i in range(0, len(work_specs))], names, rotation=0, fontsize=8)
    plt.tight_layout()
    plt.savefig('breakup/{}.pdf'.format('data'))
    
    
# work_specs, timeslice = ['mcf', 'base64', 'matmul_int', 'mcf+base64', 'mcf+matmul_int', 'base64+matmul_int', 'mcf+base64+matmul_int'], 'all'
# work_specs, timeslice = ['mcf', 'mcf+mcf', 'mcf+mcf+mcf', 'mcf+mcf+mcf+mcf'], '5us'
# work_specs, timeslice = ['sum', 'sum+sum', 'sum+sum+sum', 'sum+sum+sum+sum'], '5us'
# work_specs, timeslice = ['matmul_int', 'matmul_int+matmul_int', 'matmul_int+matmul_int+matmul_int', 'matmul_int+matmul_int+matmul_int+matmul_int'], '5us'
# work_specs, timeslice = ['base64', 'base64+base64', 'base64+base64+base64', 'base64+base64+base64+base64'], '5us'

work_specs = ['mcf', 'mcf+mcf', 'mcf+mcf+mcf', 'mcf+mcf+mcf+mcf', \
        'base64', 'base64+base64', 'base64+base64+base64', 'base64+base64+base64+base64', \
        'matmul_int', 'matmul_int+matmul_int', 'matmul_int+matmul_int+matmul_int', 'matmul_int+matmul_int+matmul_int+matmul_int']
names = [r'$\mathrm{mcf}$', r'$\mathrm{mcf} \times 2$', r'$\mathrm{mcf} \times 3$', r'$\mathrm{mcf} \times 4$', \
        r'$\mathrm{base64}$', r'$\mathrm{base64} \times 2$', r'$\mathrm{base64} \times 3$', r'$\mathrm{base64} \times 4$', \
        r'$\mathrm{matmul}$', r'$\mathrm{matmul} \times 2$', r'$\mathrm{matmul} \times 3$', r'$\mathrm{matmul} \times 4$']
timeslice = '5us'

plot(work_specs, names, timeslice)
