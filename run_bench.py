import sys
import time
import subprocess
import struct

def runcmd(cmdstr, redirect=False):
    if redirect:
        p = subprocess.Popen(cmdstr, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    else:
        p = subprocess.Popen(cmdstr, shell=True, stdin=subprocess.PIPE)
    return p

mode = sys.argv[1]
tasks = sys.argv[2]

binary, config = None, None
if mode == 'base':
    binary = 'bench_uintr'
    config = 'server.config'
elif mode == 'uintr':
    binary = 'bench_uintr'
    config = 'server5.config'
elif mode == 'fast':
    binary = 'bench_fast'
    config = 'server5.config' 

prefix = '/caladan-uintr'
def run_iokernel():
    iok = runcmd('{}/iokerneld simple nobw noht'.format(prefix))
    iok.poll()
    print('============ iokernel({}): {}'.format(iok.pid, iok.returncode))
    return iok

def run_runtime():
    rt = runcmd('{}/apps/uintr_bench/{} {}/{} {}'.format(prefix, binary, prefix, config, tasks))
    rt.poll()
    print('============ runtime({}): {}'.format(rt.pid, rt.returncode))
    return rt

def init_sync():
    with open("/tmp/experiment", "wb") as file:
        data = struct.pack("<i", 0)
        file.write(data)
    
    with open("/tmp/experiment", "rb") as file:
        data = file.read()
        status = struct.unpack("<i", data)[0]
        print("Read sync status {} from /tmp/experiment".format(status))

init_sync()

iok = run_iokernel()

ts = 2
print('============ sleep time:', ts)

time.sleep(ts)
rt = run_runtime()
