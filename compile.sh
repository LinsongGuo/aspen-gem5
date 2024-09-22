make clean
make -j

pushd ksched
# make clean
make
popd
# ./scripts/setup_machine.sh 

pushd bindings/cc
make clean
make
popd

pushd shim
make clean
make
popd

# echo "------ rm test"
# rm tests/test_base_gen
# echo "------ remake test"
# gcc -T ./base/base.ld -o tests/test_base_gen tests/test_base_gen.o ./libruntime.a ./libnet.a ./libbase.a -lpthread ./m5/libm5.a bindings/cc/librt++.a

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
popd

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

pushd apps/rocksdb
pushd rocksdb
git checkout gcc-noadvanced
make static_lib -j
popd
make
popd