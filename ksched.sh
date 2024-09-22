# cd /caladan-uintr/ksched
# make clean
# make
# insmod build/ksched.ko
# modinfo  build/ksched.ko

insmod build/ksched.ko
dmesg