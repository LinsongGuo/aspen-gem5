import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import pandas as pd 
import sys 
import math 
from matplotlib.ticker import ScalarFormatter
from matplotlib.ticker import FuncFormatter
import numpy as np

# color = ['black',  '#E69600', '#56B4E9',  '#D62728',  '#E69600', 'darkseagreen', 'sienna' ]

# color = ['#56B4E9', '#CC7AA8', '#E69600', '#1F77B4', '#FF7F0E', '#38A538', '#D62728']


# color = ['violet', 'plum', 'thistle']

color = ['#6A51A2', '#9D9BC7', '#DADBE8']

# color = ['darkgoldenrod', 'orange', 'lemonchiffon']

def plot_scalability():
    plt.figure(figsize=(4, 1.6))
    
    # T = [50, 20, 15, 10, 5, 3, 2, 1]
    # signal = [24, 10, 7, 5, 2]
    # uintr = [24, 24, 24, 24, 21, 12, 8, 4]
    # concord = [24, 24, 24, 24, 24, 24, 24, 13]

    T = [50, 15, 10, 5, 2, 1]
    signal = [24, 7, 5, 2]
    uintr = [24, 24, 24, 21, 8, 4]
    concord = [24, 24, 24, 24, 24, 13]

    bar_width = 0.25

    bar_positions_a = np.arange(len(T))
    bar_positions_b = bar_positions_a + bar_width
    bar_positions_c = bar_positions_b + bar_width

    plt.bar(bar_positions_a[:-2], signal, width=bar_width, label='Signal', color=color[0], edgecolor='black', linewidth=0.5)
    plt.bar(bar_positions_b, uintr, width=bar_width, label='UINTR', color=color[1], edgecolor='black', linewidth=0.5)
    plt.bar(bar_positions_c, concord, width=bar_width, label='Compiler-based', color=color[2], edgecolor='black', linewidth=0.5)

    plt.xlabel(r'Preemption quantum ($\mu$s)', fontsize=9)
    plt.ylabel('Number of cores', fontsize=9)

    # plt.axhline(y=4, linestyle='--', linewidth=0.5, c='grey')
    # plt.axhline(y=13, linestyle='--', linewidth=0.5, c='grey')

    plt.xticks(bar_positions_b, T, fontsize=8)
    plt.yticks([0, 8, 16, 24], fontsize=8)

    plt.legend(fontsize=9, loc='upper center', columnspacing=0.5, bbox_to_anchor=(0.5, 1.25), ncol=3, frameon=False, handlelength=1)

    plt.subplots_adjust(left=0.12, right=0.97, top=0.88, bottom=0.25)

    plt.show()
    plt.savefig('scalability.pdf'.format())
    
    
plot_scalability()