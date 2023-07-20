//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

extern "C" {
#include <base/byteorder.h>
#include <base/log.h>
#include <runtime/runtime.h>
#include <runtime/udp.h>
}

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <c.h>

#if defined(OS_WIN)
#include <Windows.h>
#else
#include <unistd.h>  // sysconf() - get CPU count
#endif

const char DBPath[] = "/tmp/rocksdb_c_simple_example";

static rocksdb_t *db;

void MainHandler(void *arg) {
  // Get value
  char *err = NULL;
  const char key[] = "key";
  rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
  size_t len;
  char *returned_value =
      rocksdb_get(db, readoptions, key, strlen(key), &len, &err);
  assert(!err);
  assert(strcmp(returned_value, "value") == 0);
  free(returned_value);

  rocksdb_readoptions_destroy(readoptions);
}

int main(int argc, char **argv) {
  rocksdb_options_t *options = rocksdb_options_create();
  // Optimize RocksDB. This is the easiest way to
  // get RocksDB to perform well.
#if defined(OS_WIN)
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  long cpus = system_info.dwNumberOfProcessors;
#else
  long cpus = sysconf(_SC_NPROCESSORS_ONLN);
#endif
  rocksdb_options_set_allow_mmap_reads(options, 1);
  rocksdb_options_set_allow_mmap_writes(options, 1);
  rocksdb_slicetransform_t *prefix_extractor =
      rocksdb_slicetransform_create_fixed_prefix(8);
  rocksdb_options_set_prefix_extractor(options, prefix_extractor);
//   rocksdb_options_set_plain_table_factory(options, 0, 10, 0.75, 3);
  // Set # of online cores
  rocksdb_options_increase_parallelism(options, (int)(cpus));
  rocksdb_options_optimize_level_style_compaction(options, 0);
  // create the DB if it's not already present
  rocksdb_options_set_create_if_missing(options, 1);

//   rocksdb_options_set_allow_mmap_reads(options, 1);
//   rocksdb_options_set_allow_mmap_writes(options, 1);
//   rocksdb_slicetransform_t *prefix_extractor =
//       rocksdb_slicetransform_create_fixed_prefix(8);
//   rocksdb_options_set_prefix_extractor(options, prefix_extractor);
//   rocksdb_options_set_plain_table_factory(options, 0, 10, 0.75, 3);
//   // Optimize RocksDB. This is the easiest way to
//   // get RocksDB to perform well
//   rocksdb_options_increase_parallelism(options, 0);
//   rocksdb_options_optimize_level_style_compaction(options, 0);
//   // create the DB if it's not already present
//   rocksdb_options_set_create_if_missing(options, 1);


  // open DB
  char *err = NULL;
  db = rocksdb_open(options, DBPath, &err);
  assert(!err);

  // Put key-value
  rocksdb_writeoptions_t *writeoptions = rocksdb_writeoptions_create();
  const char key[] = "key";
  const char *value = "value";
  rocksdb_put(db, writeoptions, key, strlen(key), value, strlen(value) + 1,
              &err);
  assert(!err);


  // cleanup
  rocksdb_writeoptions_destroy(writeoptions);
  rocksdb_options_destroy(options);

  // MainHandler(NULL);
  
  int ret = runtime_init(argv[1], MainHandler, NULL);
  if (ret) {
    std::cerr << "failed to start runtime" << std::endl;
    return ret;
  }

  rocksdb_close(db);

  return 0;
}