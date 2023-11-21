
ROOT_PATH=../../..
include $(ROOT_PATH)/build/shared.mk

librt_libs = $(ROOT_PATH)/bindings/cc/librt++.a
INC += -I$(ROOT_PATH)/bindings/cc

leveldb_server_src = leveldb_server.cc
leveldb_server_obj = $(leveldb_server_src:.cc=.o)

INC += -I../leveldb/include
LIBS += -lsnappy -lpthread -lrt -lm
lib_leveldb = ../leveldb/build/libleveldb.a
lib_shim = $(ROOT_PATH)/shim/libshim.a

src = $(leveldb_server_src)
obj = $(leveldb_server_obj)
dep = $(obj:.o=.d)

# must be first
all: leveldb_server

leveldb_server: $(leveldb_server_obj) concord_leveldb.o $(lib_leveldb) $(lib_shim) $(librt_libs) $(RUNTIME_DEPS) $(LIBS)
	$(LDXX) -o $@ $(LDFLAGS) $(WRAP_FLAGS) -ldl $(leveldb_server_obj) concord_leveldb.o $(lib_leveldb) $(lib_shim) $(librt_libs) $(RUNTIME_LIBS) $(LIBS) 

create_db: $(create_db_obj) $(lib_rocksdb) $(LIBS)
	$(LDXX) -o $@ $(LDFLAGS) -ldl $(create_db_obj) $(lib_rocksdb) $(LIBS)

ifneq ($(MAKECMDGOALS),clean)
-include $(dep)   # include all dep files in the makefile
endif

%.d: %.cc
	@$(CXX) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@
concord_leveldb.o: concord-leveldb.c 
	$(CC) $(CFLAGS) -I../leveldb/include -c $< -o $@

.PHONY: clean
clean:
	rm -f $(obj) $(dep) level_server_obj concord_leveldb.o