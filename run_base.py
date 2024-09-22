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

mpps = float(sys.argv[1])

prefix = '/caladan-uintr'
def run_iokernel():
    iok = runcmd('{}/iokerneld simple nobw noht'.format(prefix))
    # iok = runcmd('gdb -ex run -ex bt --args {}/iokerneld simple nobw noht')
    iok.poll()
    print('============ iokernel({}): {}'.format(iok.pid, iok.returncode))
    return iok

def run_runtime():
    # rt = runcmd('{}/apps/uintr_bench/bench {}/server.config sum*1'.format(prefix, prefix))
    # rt = runcmd('gdb -ex run -ex bt --args {}/apps/uintr_bench/bench {}/server.config sum*1'.format(prefix, prefix))
    # rt = runcmd('{}/apps/rocksdb/rocksdb_server {}/server.config udp'.format(prefix, prefix))
    # rt = runcmd('gdb -ex run -ex bt --args {}/apps/rocksdb/rocksdb_server {}/server.config udpconn 1 32'.format(prefix, prefix))
    rt = runcmd('{}/apps/rocksdb/rocksdb_server_base {}/server.config udpconn 1 32'.format(prefix, prefix))
    # rt = runcmd('{}/apps/rocksdb/rocksdb_server {}/server.config local'.format(prefix, prefix))
    rt.poll()
    print('============ runtime({}): {}'.format(rt.pid, rt.returncode))
    return rt

def run_client():
    cl = runcmd('{}/apps/synthetic/target/release/synthetic --config {}/client.config 192.168.1.4 --mode runtime-client --protocol rocksdb --transport udp --threads 1 --mpps={} --start_mpps=0 --runtime=1 --samples=1'.format(prefix, prefix, mpps))
    # cl = runcmd('{}/apps/synthetic/target/release/synthetic --config {}/client.config 192.168.1.4 --mode runtime-client --protocol rocksdb --transport udp --distribution exponential --mean=65 --threads 1 --mpps={} --start_mpps=0 --runtime=1 --samples=1'.format(prefix, prefix, mpps))
    # cl = runcmd('gdb -ex run -ex bt --args {}/apps/synthetic/target/release/synthetic --config {}/client.config 192.168.1.4 --mode runtime-client --protocol rocksdb --transport udp --threads 1 --mpps=0.05 --start_mpps=0 --runtime=1 --samples=1'.format(prefix, prefix))
    cl.poll()
    print('============ client({}): {}'.format(cl.pid, cl.returncode))

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

# ts = float(sys.argv[1]) if len(sys.argv) > 1 else 2
ts = 2
print('============ sleep time:', ts)

time.sleep(ts)
rt = run_runtime()

time.sleep(0.5)
cl = run_client()