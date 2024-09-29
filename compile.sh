# First we need to remove stui instructions in runtime to compile client code.
./turn_stui_no.sh

# Compile the user interrupt version.
sed -i '26s/.*/CONFIG_M5_UTIMER=n/' build/config
make clean
make -j

pushd ksched
make
popd

pushd bindings/cc
make clean
make -j
popd

pushd shim
make clean
make -j
popd

# echo "------ rm test"
# rm tests/test_base_gen
# echo "------ remake test"
# gcc -T ./base/base.ld -o tests/test_base_gen tests/test_base_gen.o ./libruntime.a ./libnet.a ./libbase.a -lpthread ./m5/libm5.a bindings/cc/librt++.a

mount -t proc proc /proc
mkdir /cargo
export CARGO_HOME=/cargo
export RUSTUP_HOME=/cargo
export PATH=$CARGO_HOME/bin:$PATH
./install_rust.sh -y
source /cargo/env
rustup default nightly
cargo --version

pushd apps/synthetic
cargo clean
cargo update
cargo build --release
popd


# Resume stui instructions in runtime.
./turn_stui_yes.sh

make clean
make -j

pushd bindings/cc
make clean
make -j
popd

pushd shim
make clean
make -j
popd

pushd apps/uintr_bench
pushd programs
pushd mcf/src
make clean
make -j
popd
make clean
make -j
popd
make clean
make -j
mv bench bench_uintr
echo "bench_uintr built now."
popd

pushd apps/rocksdb
pushd rocksdb
git checkout gcc-noadvanced
make static_lib -j
popd
make clean
make
mv rocksdb_server rocksdb_server_uintr
echo "rocksdb_server_uintr built now."
popd



# Compile the fast interrupt version.
sed -i '26s/.*/CONFIG_M5_UTIMER=y/' build/config
make clean
make -j

pushd bindings/cc
make clean
make -j
popd

pushd shim
make clean
make -j
popd

pushd apps/uintr_bench
pushd programs
pushd mcf/src
make clean
make -j
popd
make clean
make -j
popd
make clean
make -j
mv bench bench_fast
echo "bench_fast built now."
popd

pushd apps/rocksdb
make clean
make
mv rocksdb_server rocksdb_server_fast
echo "rocksdb_server_fast built now."
popd