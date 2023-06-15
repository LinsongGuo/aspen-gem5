# Makefile for uintr_bench
ROOT_PATH=../..
include $(ROOT_PATH)/build/shared.mk

bench_src = bench.cc
bench_obj = $(bench_src:.cc=.o)

librt_libs = $(ROOT_PATH)/bindings/cc/librt++.a
INC += -I$(ROOT_PATH)/bindings/cc

RUNTIME_LIBS := $(RUNTIME_LIBS) -lnuma

# CXXFLAGS := $(subst -O3,-O2,$(CXXFLAGS))

program_src = $(wildcard programs/*.c)
program_obj = $(patsubst %.c,%.o,$(program_src))
uintr_obj = uintr.o 

# must be first
all: bench 

bench: $(program_obj) $(bench_obj) $(librt_libs) $(RUNTIME_DEPS)  
	$(LDXX) -g -o $@ $(LDFLAGS) $(program_obj) $(bench_obj) $(librt_libs) $(RUNTIME_LIBS)

# general build rules for all targets
src = $(bench_src)
obj = $(src:.cc=.o)
dep = $(obj:.o=.d)

ifneq ($(MAKECMDGOALS),clean)
-include $(dep)   # include all dep files in the makefile
endif

# programs/*.o: programs/*.c
#	gcc -g -muintr -Iprograms -c $< -o $@

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
%.d: %.cc
	@$(CXX) $(CXXFLAGS) -muintr -Iprograms $< -MM -MT $(@:.d=.o) >$@
%.o: %.cc
	$(CXX) $(CXXFLAGS) -g -muintr -Iprograms -c $< -o $@

# programs/*.o: programs/*.c
#	$(CC) -g -muintr -Iprograms -c $< -o $@

.PHONY: clean
clean:
	rm -f $(obj) $(dep) $(uintr_obj) bench # $(program_obj)
