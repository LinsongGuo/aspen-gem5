export PATH=$PATH:/usr/local/x86_64-pc-linux-gnu/bin
make -j
make -C bindings/cc
make -C shim
cd apps/rocksdb
make