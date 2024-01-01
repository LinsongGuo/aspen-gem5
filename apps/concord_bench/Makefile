LLVM_VERSION=11

CC=clang-$(LLVM_VERSION)
OPT=opt-$(LLVM_VERSION)
CXX=clang++-$(LLVM_VERSION)

# CFLAGS=-O3 -g -Wall -D_GNU_SOURCE -mavx512f -Wno-sync-fetch-and-nand-semantics-changed
CFLAGS=-O3 -g -Wall -D_GNU_SOURCE -Wno-sync-fetch-and-nand-semantics-changed
# PASS_FLAGS=-c -emit-llvm
# WRAP_FLAGS=-Wl,-wrap=malloc -Wl,-wrap=free -Wl,-wrap=realloc -Wl,-wrap=calloc -Wl,-wrap=aligned_alloc -Wl,-wrap=posix_memalign

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

CALADAN_PATH=$(ROOT_DIR)/../..
CALADAN_INC=-I$(CALADAN_PATH)/inc -I$(CALADAN_PATH)/bindings/cc
CALADAN_LIBS=$(CALADAN_PATH)/bindings/cc/librt++.a $(CALADAN_PATH)/libruntime.a \
		$(CALADAN_PATH)/libnet.a $(CALADAN_PATH)/libbase.a 
# CALADAN_LIBS=$(CALADAN_PATH)/shim/libshim.a \
# 		$(CALADAN_PATH)/bindings/cc/librt++.a $(CALADAN_PATH)/libruntime.a \
# 		$(CALADAN_PATH)/libnet.a $(CALADAN_PATH)/libbase.a 

MLX5_LIBS=-L$(CALADAN_PATH)/rdma-core/build/lib/
MLX5_LIBS += -L$(CALADAN_PATH)/rdma-core/build/lib/statics/
MLX5_LIBS += -L$(CALADAN_PATH)/rdma-core/build/util/
MLX5_LIBS += -L$(CALADAN_PATH)/rdma-core/build/ccan/
MLX5_LIBS += -l:libmlx5.a -l:libibverbs.a -lnl-3 -lnl-route-3 -lrdmacm -lrdma_util -lccan
CALADAN_LIBS += $(MLX5_LIBS)

LDFLAGS=-T $(CALADAN_PATH)/base/base.ld

# OPT_CONFIG=-postdomtree -mem2reg -indvars -loop-simplify -branch-prob -scalar-evolution 
# CONCORD_PASS=$(ROOT_DIR)/../../../concord/src/cache-line-pass/build/src/libConcordPass.so

all: bench bench_concord bench_concordf bench_concordfl bench_unroll bench_concord_unroll

bench_src = bench.cc
bench_obj = $(bench_src:.cc=.o)

# For other programs:
program_src = $(wildcard programs/*.c)
program_obj = $(patsubst %.c,%.o,$(program_src))
program_concord_obj = $(patsubst %.c,%_concord.o,$(program_src))
program_unroll_src = $(wildcard programs_unroll/*.c)
program_unroll_obj = $(patsubst %.c,%.o,$(program_unroll_src))
program_unroll_concord_obj = $(patsubst %.c,%_concord.o,$(program_unroll_src))

# For mcf:
libmcf = programs/mcf/src/libmcf.a
libmcf_concord = programs/mcf/src/libmcf_concord.a
libmcf_concordf = programs/mcf_f/src/libmcf_concord.a
libmcf_concordfl = programs/mcf_fl/src/libmcf_concord.a

libmcf_unroll = programs_unroll/mcf/src/libmcf.a
libmcf_concord_unroll = programs_unroll/mcf/src/libmcf_concord.a

bench: $(bench_obj)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(WRAP_FLAGS) -Iprograms $(CALADAN_INC) -o $@ $(bench_obj) $(libmcf) $(program_obj) $(CALADAN_LIBS)

bench_concord: $(bench_obj)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(WRAP_FLAGS) -Iprograms $(CALADAN_INC) -o $@ $(bench_obj) $(libmcf_concord) $(program_concord_obj) $(CALADAN_LIBS)

bench_concordf: $(bench_obj)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(WRAP_FLAGS) -Iprograms $(CALADAN_INC) -o $@ $(bench_obj) $(libmcf_concordf) $(program_concord_obj) $(CALADAN_LIBS)

bench_concordfl: $(bench_obj)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(WRAP_FLAGS) -Iprograms $(CALADAN_INC) -o $@ $(bench_obj) $(libmcf_concordfl) $(program_concord_obj) $(CALADAN_LIBS)

bench_unroll: $(bench_obj)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(WRAP_FLAGS) -Iprograms_unroll $(CALADAN_INC) -o $@ $(bench_obj) $(libmcf_unroll) $(program_unroll_obj) $(CALADAN_LIBS)

bench_concord_unroll: $(bench_obj)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(WRAP_FLAGS) -Iprograms_unroll $(CALADAN_INC) -o $@ $(bench_obj) $(libmcf_concord_unroll) $(program_unroll_concord_obj) $(CALADAN_LIBS)

%.o: %.cc
	$(CXX) $(CFLAGS) -Iprograms $(CALADAN_INC) -c $< -o $@

clean:
	rm -f bench bench_concord bench_concordf bench_concordfl bench_unroll bench_unroll_concord $(bench_obj)

