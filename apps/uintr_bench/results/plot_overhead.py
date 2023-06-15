import matplotlib.pyplot as plt
import pandas as pd 

# color = ['#1F77B4', '#FF7F0E', '#38A538',  '#D62728'] # blue, orange, green, red
color = ['#56B4E9', '#CC7AA8', '#E69600', '#1F77B4', '#FF7F0E', '#38A538', '#D62728']
# color = ['#56B4E9', '#CC7AA8', '#E69600', '#CEF089', '#FCB771', '#C79CE9', '#D62728']
# T = [10000, 1000, 100, 50, 20, 10, 5, 4, 3, 2, 1]
# Tname = ['10ms', '1ms', '100us', '50us', '20us', '10us', '5us', '4us', '3us', '2us', '1us']
T = [1000, 100, 50, 20, 10, 5, 4, 3, 2, 1]
Tname = ['1ms', '100us', '50us', '20us', '10us', '5us', '4us', '3us', '2us', '1us']

def get_overhead(work_spec):
    df = pd.read_csv('{}/{}.csv'.format(work_spec, work_spec))
    return df['overhead'][2:]

def plot_overhead(work_specs, title):
    plt.figure(figsize=(8, 5))

    for i in range(len(work_specs)):
        overhead = get_overhead(work_specs[i])
        plt.plot(T, overhead, linewidth=1, marker='o', markersize=4, c=color[i], markerfacecolor='none', label=work_specs[i])
        
    plt.xscale('log')
    plt.legend()
    plt.xlim(max(T) * 2, min(T) / 2)
    plt.ylim(bottom=0, top=2)
    plt.xticks(T, Tname, fontsize=8)
    plt.xlabel('Timeslice')
    plt.ylabel('Average Preemption Overhead (us)')
        
    # Show the plot
    plt.tight_layout()
    plt.savefig('overhead/{}.pdf'.format(title))
    plt.show()
  
# work_specs, title = ['sum', 'sum+sum', 'sum+sum+sum'], 'sum'  
work_specs, title = ['base64', 'base64+base64', 'base64+base64+base64', 'base64+base64+base64+base64'], 'base64-2'
# work_specs, title = ['matmul', 'matmul+matmul', 'matmul+matmul+matmul', 'matmul+matmul+matmul+matmul'], 'matmul-2'
# work_specs, title = ['linpack', 'linpack+linpack', 'linpack+linpack+linpack', 'linpack+linpack+linpack+linpack'], 'linpack-2'
# work_specs, title = ['base64', 'matmul', 'base64+matmul'], 'base64+matmul'
# work_specs, title = ['base64', 'matmul', 'linpack', 'base64+matmul', 'base64+linpack', 'matmul+linpack', 'base64+matmul+linpack'], 'all' 
# work_specs, title = ['sum', 'base64', 'matmul', 'linpack', 'base64+sum', 'matmul+sum', 'linpack+sum'], 'sum+others'
   
plot_overhead(work_specs, title)
