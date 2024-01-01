LLVM_VERSION=11

CC=clang-$(LLVM_VERSION)
OPT=opt-$(LLVM_VERSION)
CXX=clang++-$(LLVM_VERSION)

CFLAGS=-O3 -g -Wall
PASS_FLAGS=-c -emit-llvm
OPT_CONFIG=-postdomtree -mem2reg -indvars -loop-simplify -branch-prob -scalar-evolution
CONCORD_PASS=../../../../../concord/src/cache-line-pass/build/src/libConcordPass.so

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))
BCS = $(patsubst %.c,%.bc,$(SRCS))

all: libmcf.a libmcf_concord.a

libmcf.a: mcf.ll
	rm -f $@
	$(CC) -c -O3 -o $@ $<

libmcf_concord.a: mcf_concord.opt.ll
	rm -f $@
	$(CC) -c -O3 -o $@ $<

mcf_concord.opt.ll: mcf.opt.ll
	rm -f $@
	$(OPT) -S -load $(CONCORD_PASS) -yield -o $@ $<

mcf.opt.ll: mcf.ll
	rm -f $@
	$(OPT) -S $(OPT_CONFIG) -o $@ $<

mcf.ll: mcfall.bc
	rm -f $@
	llvm-dis-$(LLVM_VERSION) -o $@ $<

mcfall.bc: $(BCS)
	rm -f $@
	llvm-link-$(LLVM_VERSION) -o $@ $^

%.bc: %.c 
	$(CC) $(CFLAGS) $(PASS_FLAGS) -I../.. $< -o $@ 

clean:
	rm -rf $(OBJS) $(BCS) *.bc *.ll libmcf.a libmcf_concord.a