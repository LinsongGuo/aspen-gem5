

LLVM_VERSION=11

CC=clang-$(LLVM_VERSION)
OPT=opt-$(LLVM_VERSION)
CXX=clang++-$(LLVM_VERSION)

CFLAGS=-O3 -g -Wall -D_GNU_SOURCE -mavx512f -Wno-sync-fetch-and-nand-semantics-changed
# CFLAGS=-O3 -g -Wall -D_GNU_SOURCE -Wno-sync-fetch-and-nand-semantics-changed

WRAP_FLAGS=-Wl,-wrap=malloc -Wl,-wrap=free -Wl,-wrap=realloc -Wl,-wrap=calloc -Wl,-wrap=aligned_alloc -Wl,-wrap=posix_memalign

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

CALADAN_PATH=$(ROOT_DIR)/../..
CALADAN_INC=-I$(CALADAN_PATH)/inc -I$(CALADAN_PATH)/bindings/cc
CALADAN_LIBS=$(CALADAN_PATH)/shim/libshim.a \
		$(CALADAN_PATH)/bindings/cc/librt++.a $(CALADAN_PATH)/libruntime.a \
		$(CALADAN_PATH)/libnet.a $(CALADAN_PATH)/libbase.a 

MLX5_LIBS=-L$(CALADAN_PATH)/rdma-core/build/lib/
MLX5_LIBS += -L$(CALADAN_PATH)/rdma-core/build/lib/statics/
MLX5_LIBS += -L$(CALADAN_PATH)/rdma-core/build/util/
MLX5_LIBS += -L$(CALADAN_PATH)/rdma-core/build/ccan/
MLX5_LIBS += -l:libmlx5.a -l:libibverbs.a -lnl-3 -lnl-route-3 -lrdmacm -lrdma_util -lccan
CALADAN_LIBS += $(MLX5_LIBS)

LDFLAGS=-T $(CALADAN_PATH)/base/base.ld

ROCKSDB_INC=-Irocksdb/include/rocksdb/
ROCKSDB_LIB=rocksdb/librocksdb.a

all: clean rocksdb_server

rocksdb_server:
	$(CXX) $(CFLAGS) $(LDFLAGS) $(WRAP_FLAGS) -o rocksdb_server -ldl rocksdb_server.cc $(ROCKSDB_INC) $(ROCKSDB_LIB) $(CALADAN_INC) $(CALADAN_LIBS) -lgflags -lsnappy -lz -llz4 -lpthread -ljemalloc

clean:
	rm -f rocksdb_server rocksdb_server.o

