

LLVM_VERSION=11

CC=clang-$(LLVM_VERSION)
OPT=opt-$(LLVM_VERSION)
CXX=clang++-$(LLVM_VERSION)

CFLAGS=-O3 -g -Wall -D_GNU_SOURCE -Wno-sync-fetch-and-nand-semantics-changed
PASS_FLAGS=-c -emit-llvm

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

CALADAN_PATH=$(ROOT_DIR)/../..
CALADAN_INC=-I$(CALADAN_PATH)/inc -I$(CALADAN_PATH)/bindings/cc
CALADAN_LIBS=\
		$(CALADAN_PATH)/bindings/cc/librt++.a $(CALADAN_PATH)/libruntime.a \
		$(CALADAN_PATH)/libnet.a $(CALADAN_PATH)/libbase.a 

MLX5_LIBS=-L$(CALADAN_PATH)/rdma-core/build/lib/
MLX5_LIBS += -L$(CALADAN_PATH)/rdma-core/build/lib/statics/
MLX5_LIBS += -L$(CALADAN_PATH)/rdma-core/build/util/
MLX5_LIBS += -L$(CALADAN_PATH)/rdma-core/build/ccan/
MLX5_LIBS += -l:libmlx5.a -l:libibverbs.a -lnl-3 -lnl-route-3 -lrdmacm -lrdma_util -lccan
CALADAN_LIBS += $(MLX5_LIBS)

LDFLAGS=-T $(CALADAN_PATH)/base/base.ld

OPT_CONFIG=-postdomtree -mem2reg -indvars -loop-simplify -branch-prob -scalar-evolution 
CONCORD_PASS=$(ROOT_DIR)/../../../concord/src/cache-line-pass/build/src/libConcordPass.so

BENCH=loop

all: clean loop test test_concord 

loop:
	$(CC) $(CFLAGS) $(PASS_FLAGS) $(BENCH).c -o $(BENCH).bc
	$(OPT) -S $(OPT_CONFIG) < $(BENCH).bc > $(BENCH).opt.bc

	$(OPT) -S -load $(CONCORD_PASS) -yield < $(BENCH).opt.bc > $(BENCH).pass.opt.bc
	
	$(CC) $(BENCH).bc $(CFLAGS) -c -o loop.a
	$(CC) $(BENCH).pass.opt.bc $(CFLAGS) -c -o loop_concord.a
	

test:
	$(CXX) $(CFLAGS) $(LDFLAGS) $(CALADAN_INC) -o test -ldl test.cc loop.a  $(CALADAN_LIBS) -lpthread

test_concord:
	$(CXX) $(CFLAGS) $(LDFLAGS) $(CALADAN_INC) -o test_concord -ldl test.cc loop_concord.a $(CALADAN_LIBS) -lpthread

clean:
	rm -f *.bc *.a loop test test_concord

