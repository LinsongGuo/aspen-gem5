import sys
import re
import os
import pandas as pd

def get_data(filename):	
    with open(filename, "r") as file:
        for line in file:
            columns = re.split(r'\s+', line)
            if '\'BenchYield_work\'' in columns:
                return float(columns[3])
			

bench = sys.argv[1]

uths = []
caladan = []
simdreg = []
custom = []
data = {'uths': [], 'caladan': [],  'caladan-simdreg': [], 'caladan-simdreg-custom': [], 'native': []}
configs = ['caladan', 'caladan-simdreg', 'caladan-simdreg-custom']

native=get_data(bench + '/native')

for config in configs:
    for root, dirs, files in os.walk(bench + '/' + config):
        sorted_files = sorted(files, key=lambda x: int(x))
        for file in sorted_files:
            filepath = os.path.join(root, file)
            if config == 'caladan':
                data['uths'].append(int(file))
            data[config].append(get_data(filepath))
data['native'] = [native] * len(data['uths'])


print(data['uths'])
print(data['caladan'])
df = pd.DataFrame(data)
df.to_csv('{}.csv'.format(bench), index=False)