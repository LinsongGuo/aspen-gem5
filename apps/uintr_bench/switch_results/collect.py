import sys
import re
import os
import pandas as pd


Num = 16
trial = 21

def get_data(filename):
	exe, uintr = None, None
	with open(filename, "r") as file:
		for line in file:
			columns = re.split(r'\s+', line)
			if 'Execution:' in columns:
				exe = float(columns[1])
			if 'Uintrs_received:' in columns:
				uintr = int(columns[1])
	return exe, uintr

# def get_median_index(arr):
#     sorted_arr = sorted(arr) 
#     median_index = len(arr) // 2  
#     if len(arr) % 2 == 0:
#         median_index -= 1
#     return arr.index(sorted_arr[median_index])

def collect(path):
    data = {"trial": [i for i in range(1, trial+1)]}
    
    for n in range(1, Num+1):
        exes = []
        uintrs = []
        for i in range(1, trial+1):
            filename = path + '/' + str(n) + '/' + str(i)
            exe, uintr = get_data(filename)
            exes.append(exe)
            uintrs.append(uintr)
            data['{}-exe'.format(n)] = exes 
            data['{}-uintr'.format(n)] = uintrs 
    df = pd.DataFrame(data)
    df.to_csv('{}/summary.csv'.format(path), index=False)


path = sys.argv[1]
print('path: ', path)
collect(path) 
        