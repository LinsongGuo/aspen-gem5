import sys
import time
import subprocess
import struct

def runcmd(cmdstr, nonblocking=True):
    if nonblocking:
        # subprocess.Popen(cmdstr, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        return subprocess.Popen(cmdstr, shell=True, stdin=subprocess.PIPE)
    else:
        subprocess.run(cmdstr, shell=True, capture_output=True, text=True)
        return None 
    
mode = sys.argv[1]
quantum = int(sys.argv[2])
tasks = sys.argv[3]

prefix = '/caladan-uintr'
binary = None
if mode == 'base':
    binary = 'rocksdb_server_uintr'
elif mode == 'uintr':
    binary = 'rocksdb_server_uintr'
elif mode == 'fast':
    binary = 'rocksdb_server_fast'

prefix = '/caladan-uintr'
def run_iokernel():
    iok = runcmd('{}/iokerneld simple nobw noht'.format(prefix))
    iok.poll()
    print('============ iokernel({}): {}'.format(iok.pid, iok.returncode))
    return iok

def run_runtime():
    # rt = runcmd('{}/apps/uintr_bench/bench {}/server.config sum*1'.format(prefix, prefix))
    # rt = runcmd('gdb -ex run -ex bt --args {}/apps/uintr_bench/bench {}/server.config sum*1'.format(prefix, prefix))
    # rt = runcmd('{}/apps/rocksdb/rocksdb_server {}/server.config udp'.format(prefix, prefix))
    # rt = runcmd('gdb -ex run -ex bt --args {}/apps/rocksdb/rocksdb_server {}/server.config udpconn 1 32'.format(prefix, prefix))
    # rt = runcmd('{}/apps/rocksdb/rocksdb_server_base {}/server.config udpconn 1 32'.format(prefix, prefix))
    rt = runcmd('{}/apps/rocksdb/{} {}/server.config local {}'.format(prefix, binary, prefix, tasks))
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


quantum_cmd = 'sed -i \"8s/.*/runtime_uthread_quantum_us {}/\" {}/server.config'.format(quantum, prefix)
runcmd(quantum_cmd)

init_sync()

iok = run_iokernel()

ts = 2
print('============ sleep time:', ts)

time.sleep(ts)
rt = run_runtime()
