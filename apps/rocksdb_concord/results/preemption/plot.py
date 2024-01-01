import matplotlib.pyplot as plt
from matplotlib import ticker
import pandas as pd 
import sys 
import math

# color = ['#56B4E9', '#D62728',  '#E69600', '#CC7AA8']
color = ['#56B4E9', '#D62728',  '#E69600', '#CC7AA8']
# T = [1000, 100, 50, 25, 20, 15, 10, 5, 3, 2, 1]
T = [1, 2, 3, 5, 10, 15, 20, 25, 50, 100]

systems = ['signal', 'uintr', 'concord', 'concordfl']
system_labels = ['Signal', 'UINTR', 'Compiler (Instrument loops)', 'Compiler (Instrument loops+funcs)']
bench = sys.argv[1]

def get_overhead(system):
    df = pd.read_csv('{}/result.csv'.format(bench))
    exe = df['{}_exe'.format(system)].tolist()
    preempt = df[system].tolist()
    baseline = exe[0]
    exe = exe[2:]
    preempt = preempt[2:]
    overhead = []
    
    for i in range(len(exe)):
        if math.isnan(exe[i]):
            overhead.append(3e-6)
        else:
            overhead.append((exe[i]-baseline)/preempt[i])
    
    overhead.reverse()
    return overhead 

def get_slowdown(system):
    df = pd.read_csv('{}/result.csv'.format(bench))
    exe = df['{}_exe'.format(system)].tolist()
    preempt = df[system].tolist()
    baseline = exe[0]
    exe = exe[2:]
    preempt = preempt[2:]
    slowdown = []
    for i in range(len(exe)):
        if math.isnan(exe[i]):
            slowdown.append(10)
        else:
            slowdown.append(max(0, (exe[i]-baseline)/baseline))
        
    slowdown.reverse()
    return slowdown 

def plot_overhead():
    plt.figure(figsize=(5, 3))

    for i in range(len(systems[:-1])):
        system = systems[i]
        label = system_labels[i]
        overhead = get_overhead(system)
        print(system, overhead)
        plt.plot(T, overhead, linewidth=1, marker='o', markersize=4, c=color[i], markerfacecolor='none', label=label)
        
    plt.xscale('log')
    plt.legend()
    # plt.xlim(max(T_merge) * 2, min(T_merge) / 2)
    plt.ylim(bottom=0, top=3e-6)
    # plt.axhline(y=0.4, linewidth=1, linestyle='--', color='grey')
    # # plt.axhline(y=2.7, linewidth=1, linestyle='--', color='grey')
    # plt.yticks([0, 0.4, 2, 4, 6, 8], fontsize=8)
    # plt.xticks(T_merge, Tname_merge, fontsize=7)
    plt.xlabel('Preemption quantum (log scale)')
    plt.ylabel('Average Preemption Overhead (us)')
        
    # Show the plot
    plt.tight_layout()
    plt.savefig('{}/overhead.pdf'.format(bench))
    plt.show()
    
def plot_slowdown():
    plt.figure(figsize=(5, 2.3))

    for i in range(len(systems)):
        system = systems[i]
        label = system_labels[i]
        slowdown = get_slowdown(system)
        plt.plot(T, slowdown, linewidth=1.4, marker='o', markersize=4, c=color[i], markerfacecolor='none', label=label)
        
    plt.xscale('log')
    xticks = [1, 2, 3, 5, 10, 25, 50, 100]
    plt.tick_params(axis='x', which='both', bottom=True, top=False, direction='out')
    plt.xticks(xticks, labels=xticks)
    plt.xlim(1, 100)
    plt.ylim(bottom=0, top=0.6)
    plt.yticks([0, 0.1, 0.2, 0.4, 0.6])
    plt.gca().yaxis.set_major_formatter(ticker.PercentFormatter(xmax=1))
    plt.axhline(y=0.1, linewidth=1, linestyle='--', color='grey')
    plt.xlabel('Preemption quantum (Log scale)', fontsize=12)
    plt.ylabel('Slowdown', fontsize=12)
    plt.legend(fontsize=11,  loc='upper center', bbox_to_anchor=(0.5, 1.4), columnspacing=1, frameon=False, ncol=2, handlelength=1.5)
        
    # Show the plot
    plt.tight_layout()
    plt.subplots_adjust(left=0.13, right=0.96, top=0.8, bottom=0.2, wspace=0.25)
    plt.savefig('{}/slowdown.pdf'.format(bench))
    plt.show()
    
plot_overhead()
# plot_slowdown()