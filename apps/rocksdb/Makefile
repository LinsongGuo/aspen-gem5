# Makefile for rocksdb
ROOT_PATH=../..
include $(ROOT_PATH)/build/shared.mk

librt_libs = $(ROOT_PATH)/bindings/cc/librt++.a
INC += -I$(ROOT_PATH)/bindings/cc

rocksdb_server_src = rocksdb_server.cc
rocksdb_server_obj = $(rocksdb_server_src:.cc=.o)

test_src = test.cc 
test_obj = $(test_src:.cc=.o)

create_db_src = create_db.cc
create_db_obj = $(create_db_src:.cc=.o)

new_src = new.cc
new_obj = $(new_src:.cc=.o)

measure_src = measure_wrap.cc
measure_obj = $(measure_src:.cc=.o)

INC += -Irocksdb/include/rocksdb/
# LIBS += rocksdb/librocksdb.a -lgflags -lsnappy -lz -llz4 -lpthread -ljemalloc -ldl
LIBS += rocksdb/librocksdb.a -lgflags -lsnappy -lz -llz4 -lpthread -ldl

lib_shim = $(ROOT_PATH)/shim/libshim.a

src = $(rocksdb_server_src) $(create_db_src) $(measure_src)
obj = $(rocksdb_server_obj) $(create_db_obj) $(measure_obj)
dep = $(obj:.o=.d)

WRAP_FLAGS = -Wl,-wrap=malloc -Wl,-wrap=free -Wl,-wrap=realloc -Wl,-wrap=calloc -Wl,-wrap=aligned_alloc -Wl,-wrap=posix_memalign -Wl,-wrap=memcpy -Wl,-wrap=memcmp -Wl,-wrap=strcmp 
wrap_obj = wrap.o

# must be first
all: rocksdb_server create_db test

rocksdb_server: $(new_obj) $(wrap_obj) $(rocksdb_server_obj) $(LIBS) $(lib_shim) $(librt_libs) $(RUNTIME_DEPS)
	$(LDXX) -o $@ $(LDFLAGS) $(WRAP_FLAGS) -ldl $(rocksdb_server_obj) $(LIBS) $(new_obj) $(wrap_obj) $(lib_shim) $(librt_libs) $(RUNTIME_LIBS) 

test: $(new_obj) $(wrap_obj) $(test_obj) $(LIBS) $(lib_shim) $(librt_libs) $(RUNTIME_DEPS)
	$(LDXX) -o $@ $(LDFLAGS) $(WRAP_FLAGS) -ldl $(test_obj) $(LIBS) $(new_obj) $(wrap_obj) $(lib_shim) $(librt_libs) $(RUNTIME_LIBS) 

create_db: $(create_db_obj)
	$(LDXX) -o $@ $(LDFLAGS) -ldl $(create_db_obj) $(LIBS)

ifneq ($(MAKECMDGOALS),clean)
-include $(dep)   # include all dep files in the makefile
endif

CXXFLAGS += -muintr 
CFLAGS += -muintr
%.d: %.cc
	@$(CXX) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@
%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(obj) $(dep) $(wrap_obj) $(new_obj) rocksdb_server create_db measure_wrap measure_nowrap
