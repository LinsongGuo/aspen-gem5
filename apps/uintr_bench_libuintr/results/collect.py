import sys
import re
import os
import pandas as pd

T = [100000000, 10000, 5000, 1000, 500, 100, 50, 20, 10, 7, 5, 3, 1]
Tname=['baseline', '10ms', '5ms', '1ms', '500us', '100us', '50us', '20us', '10us', '7us', '5us', '3us', '1us']
# T = [100000000,  5, 3, 1]
# Tname=['baseline', '5us', '3us', '1us']
trial = 33

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

def get_median_index(arr):
    sorted_arr = sorted(arr) 
    median_index = len(arr) // 2  
    if len(arr) % 2 == 0:
        median_index -= 1
    return arr.index(sorted_arr[median_index])


def collect(work_spec):
	data = {"timeslice": Tname}

	exes = []
	uintrs = []	
	
	for tname in Tname:
		exe_, uintr_ = [], []
		for i in range(1, trial+1):
			exe, uintr = get_data('{}/{}/{}'.format(work_spec, tname, i))
			if (exe is not None) and (uintr is not None): 
				exe_.append(exe)
				uintr_.append(uintr)
		idx = get_median_index(exe_)
		# if tname == 'baseline' and work_spec == 'mcf':
		# 	exes.append(1.662951936)
		# 	uintrs.append(0)
		# else:
		exes.append(exe_[idx])
		uintrs.append(uintr_[idx])

	data['execution_time'] = exes 
	data['#preemptions'] = uintrs 

	overheads = [None]
	baseline = exes[0]
	for i in range(1, len(exes)):
		overhead = (exes[i] - baseline) / uintrs[i] * (10**6) if uintrs[i] != 0 else None
		overheads.append(overhead)
		
	data['overhead'] = overheads

	df = pd.DataFrame(data)
	df.to_csv('{}/{}.csv'.format(work_spec, work_spec), index=False)
		
cur_path = '.'
work_specs = []
for name in os.listdir(cur_path):
    if 'collect' in name or 'plot' in name or 'overhead' in name or 'slowdown' in name or 'cache_contention' in name or 'breakup' in name:
        continue
    if  name == 'sum+sum+sum+sum':
        continue
    work_specs.append(name)
    
for work_spec in work_specs:
	collect(work_spec)

 
        