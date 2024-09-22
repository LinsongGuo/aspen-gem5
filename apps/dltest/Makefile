ROOT_PATH=../..
include $(ROOT_PATH)/build/shared.mk

librt_libs = $(ROOT_PATH)/bindings/cc/librt++.a
lib_shim = $(ROOT_PATH)/shim/libshim.a
INC += -I$(ROOT_PATH)/bindings/cc
LIBS += -lsnappy -lz -llz4 -lbz2 -lzstd -lpthread -ljemalloc

main: main.o $(lib_shim) $(librt_libs) $(RUNTIME_DEPS) $(LIBS)
	$(LDXX) -o $@ $(LDFLAGS) $(WRAP_FLAGS) main.o $(lib_shim) $(librt_libs) $(RUNTIME_LIBS) $(LIBS) -ldl 
	
%.d: %.cc
	@$(CXX) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm main