LLVM_VERSION=11

CC=clang-$(LLVM_VERSION)
OPT=opt-$(LLVM_VERSION)
CXX=clang++-$(LLVM_VERSION)

CFLAGS=-O3 -g -Wall
PASS_FLAGS=-c -emit-llvm

SRCS = $(wildcard *.c)
BCS = $(patsubst %.c,%.bc,$(SRCS))
OBJS = $(patsubst %.c,%.o,$(SRCS))
OBJS_CONCORD = $(patsubst %.c,%_concord.o,$(SRCS))

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
OPT_CONFIG=-postdomtree -mem2reg -indvars -loop-simplify -branch-prob -scalar-evolution 
CONCORD_PASS=$(ROOT_DIR)/../../../concord/src/cache-line-pass/build/src/libConcordPass.so

all: $(OBJS) $(OBJS_CONCORD)

%.o: %.bc
	$(CC) $(CFLAGS) -c -o $@ $<

%_concord.o: %.pass.opt.bc
	$(CC) $(CFLAGS) -c -o $@ $<

%.pass.opt.bc: %.opt.bc
	$(OPT) -S -load $(CONCORD_PASS) -yield -o $@ $<

%.opt.bc: %.bc
	$(OPT) -S $(OPT_CONFIG) -o $@ $<

%.bc: %.c
	$(CC) $(CFLAGS) $(PASS_FLAGS) -c $< -o $@
	
# %.bc: %.cc
# 	$(CXX) $(CFLAGS) $(PASS_FLAGS) -c $< -o $@

clean:
	rm $(OBJS) $(OBJS_CONCORD)