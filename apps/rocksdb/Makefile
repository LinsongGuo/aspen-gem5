# Makefile for rocksdb
ROOT_PATH=../..
include $(ROOT_PATH)/build/shared.mk

librt_libs = $(ROOT_PATH)/bindings/cc/librt++.a
INC += -I$(ROOT_PATH)/bindings/cc

rocksdb_server_src = rocksdb_server.cc
rocksdb_server_obj = $(rocksdb_server_src:.cc=.o)

create_db_src = create_db.cc
create_db_obj = $(create_db_src:.cc=.o)

INC += -Irocksdb/include/rocksdb/
LIBS += -lgflags -lsnappy -lz -llz4 -lbz2 -lzstd # -ljemalloc # -lpthread 
lib_rocksdb = rocksdb/librocksdb.a
lib_shim = $(ROOT_PATH)/shim/libshim.a

src = $(rocksdb_server_src) $(create_db_src)
obj = $(rocksdb_server_obj) $(create_db_obj) dltest.o
dep = $(obj:.o=.d)

# WRAP_FLAGS = -Wl,-wrap=malloc -Wl,-wrap=free -Wl,-wrap=realloc -Wl,-wrap=calloc -Wl,-wrap=aligned_alloc -Wl,-wrap=posix_memalign
# ifeq ($(CONFIG_UNSAFE_PREEMPT),flag)
# WRAP_FLAGS += -Wl,-wrap=memcpy -Wl,-wrap=memcmp -Wl,-wrap=memmove -Wl,-wrap=memset -Wl,-wrap=strcmp -Wl,-wrap=strncmp   
# endif 
# ifeq ($(CONFIG_UNSAFE_PREEMPT),clui)
# WRAP_FLAGS += -Wl,-wrap=memcpy -Wl,-wrap=memcmp -Wl,-wrap=memmove -Wl,-wrap=memset -Wl,-wrap=strcmp -Wl,-wrap=strncmp   
# endif 

# must be first
all: rocksdb_server 

dltest: dltest.o $(lib_shim) $(librt_libs) $(RUNTIME_DEPS)
	$(LDXX) -o $@ $(LDFLAGS) $(WRAP_FLAGS) dltest.o $(lib_shim) $(librt_libs) $(RUNTIME_LIBS) -ldl 

rocksdb_server: $(rocksdb_server_obj) $(lib_rocksdb) $(lib_shim) $(librt_libs) $(RUNTIME_DEPS)
	$(LDXX) -o $@ $(LDFLAGS) $(WRAP_FLAGS) $(rocksdb_server_obj) $(lib_rocksdb) $(lib_shim) -ldl $(librt_libs) $(RUNTIME_LIBS) $(LIBS) 

create_db: $(create_db_obj) $(lib_rocksdb)
	$(LDXX) -o $@ $(LDFLAGS) $(create_db_obj) $(lib_rocksdb) $(LIBS) -ldl

ifneq ($(MAKECMDGOALS),clean)
-include $(dep)   # include all dep files in the makefile
endif

%.d: %.cc
	@$(CXX) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@
%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(obj) $(dep) rocksdb_server create_db
