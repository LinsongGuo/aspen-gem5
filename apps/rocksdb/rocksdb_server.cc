extern "C" {
#include <base/byteorder.h>
#include <base/log.h>
#include <runtime/runtime.h>
#include <runtime/udp.h>
#include <runtime/uintr.h>
}

#include <c.h>
#include <algorithm>
#include <iostream>
#include "sync.h"
#include "uintr.h"
#include "thread.h"
#include <x86intrin.h>

static rocksdb_t *db;
static netaddr listen_addr;

struct Payload {
  uint32_t id;
  uint32_t req_type;
  uint32_t reqsize;
  uint32_t run_ns;
};

static inline void DoScan(rocksdb_readoptions_t *readoptions) {
  const char *retr_key;
  size_t klen;

  rocksdb_iterator_t *iter = rocksdb_create_iterator(db, readoptions);
  rocksdb_iter_seek_to_first(iter);
  while (rocksdb_iter_valid(iter)) {
    retr_key = rocksdb_iter_key(iter, &klen);
    log_debug("Scanned key %.*s\n", (int)klen, retr_key);
    rocksdb_iter_next(iter);
  }
  rocksdb_iter_destroy(iter);
}

static inline void DoGet(rocksdb_readoptions_t *readoptions) {
  const char *retr_key;
  size_t klen;

  rocksdb_iterator_t *iter = rocksdb_create_iterator(db, readoptions);
  rocksdb_iter_seek_to_first(iter);
  if (rocksdb_iter_valid(iter)) {
    retr_key = rocksdb_iter_key(iter, &klen);
    log_debug("Scanned key %.*s\n", (int)klen, retr_key);
  }
  rocksdb_iter_destroy(iter);
}

static void HandleRequest(udp_spawn_data *d) {
  const Payload *p = static_cast<const Payload *>(d->buf);

  rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();

  if (p->req_type == 11)
    DoScan(readoptions);
  else if (p->req_type == 10)
    DoGet(readoptions);
  else
    panic("bad req type %u", p->req_type);
  rocksdb_readoptions_destroy(readoptions);
  barrier();

  Payload rp = *p;
  rp.run_ns = 0;

  ssize_t wret = udp_respond(&rp, sizeof(rp), d);
  if (unlikely(wret <= 0)) panic("wret");
  udp_spawn_data_release(d->release_data);
}

std::string scan_keys[5000];
void Get5000(int sp) {
  char *err = NULL;
  uint64_t durations[5000];
  rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
  int cnt = 0;
  for (int i = sp*2500; i < sp*2500+2500; i++) {
    size_t len = scan_keys[i].length();
    const char* key = scan_keys[i].c_str();

    uint64_t start = rdtscp(NULL);
    char *returned_value =
      rocksdb_get(db, readoptions, key, strlen(key), &len, &err);
    uint64_t end = rdtscp(NULL);
    durations[cnt++] = end - start;
    
    if (err != NULL) {
      _clui();
      printf("err: %d\n", i);
      _stui();
    }
    assert(!err);

    _clui();
    if (strcmp(returned_value, "value")) {
      printf("return %d: %s\n", i, returned_value);
    }
    assert(strcmp(returned_value, "value") == 0);
    _stui();
    // free(returned_value);
  }

  _clui();
  std::sort(std::begin(durations), std::end(durations));
  fprintf(stderr, "stats for %u iterations (GET): \n", cnt);
  fprintf(stderr, "median: %0.3f\n",
          (double)durations[cnt / 2] / (double)cycles_per_us);
  fprintf(stderr, "p99.9: %0.3f\n",
          (double)durations[cnt * 999 / 1000] / (double)cycles_per_us);
  _stui();
}

static inline void ScanInit(rocksdb_readoptions_t *readoptions) {
  const char *retr_key;
  size_t klen;

  rocksdb_iterator_t *iter = rocksdb_create_iterator(db, readoptions);
  rocksdb_iter_seek_to_first(iter);
  int idx = 0;
  while (rocksdb_iter_valid(iter)) {
    retr_key = rocksdb_iter_key(iter, &klen);
    log_debug("Scanned key %.*s", (int)klen, retr_key);
    scan_keys[idx++] = std::string(retr_key, (int)klen);
    rocksdb_iter_next(iter);
  }
  rocksdb_iter_destroy(iter);
}

void MainHandler(void *arg) {
  rt::WaitGroup wg(1);

  cycles_per_us = 2000;

  rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
  ScanInit(readoptions);
  rocksdb_readoptions_destroy(readoptions);
  
  int task_num = 2, started = 0, finished = 0;
  for (int i = 0; i < task_num; ++i) {
		rt::Spawn([&, i]() {
			started += 1;
    	
			if (started < task_num) {
				rt::Yield();
			}
			
			_stui();
			Get5000(i);
			_clui();
			
			finished += 1;
			if (finished == task_num) {
				rt::UintrTimerEnd();
				rt::UintrTimerSummary();
				wg.Done();
			}
		});
	}
  wg.Wait();
}

void MainHandler2(void *arg) {
  // udpspawner_t *s;

  cycles_per_us = 2000;
  uintr_timer_start();
  uint64_t durations[5000];
  unsigned int i = 0;
  
  _stui();
  for (i = 0; i < 5000; i++) {
    uint64_t start = rdtscp(NULL);
    rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
    DoScan(readoptions);
    rocksdb_readoptions_destroy(readoptions);
    uint64_t end = rdtscp(NULL);
    durations[i] = end - start;
  }
  _clui();

  std::sort(std::begin(durations), std::end(durations));
  fprintf(stderr, "stats for %u iterations: \n", i);
  fprintf(stderr, "median: %0.3f\n",
          (double)durations[i / 2] / (double)cycles_per_us);
  fprintf(stderr, "p99.9: %0.3f\n",
          (double)durations[i * 999 / 1000] / (double)cycles_per_us);
  
  _stui();
  for (i = 0; i < 5000; i++) {
    uint64_t start = rdtscp(NULL);
    rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
    DoGet(readoptions);
    rocksdb_readoptions_destroy(readoptions);
    uint64_t end = rdtscp(NULL);
    durations[i] = end - start;
  }
  _clui();

  std::sort(std::begin(durations), std::end(durations));
  fprintf(stderr, "stats for %u iterations: \n", i);
  fprintf(stderr, "median: %0.3f\n",
          (double)durations[i / 2] / (double)cycles_per_us);
  fprintf(stderr, "p99.9: %0.3f\n",
          (double)durations[i * 999 / 1000] / (double)cycles_per_us);
  
  uintr_timer_end();
  uintr_timer_summary();

//   int ret = udp_create_spawner(listen_addr, HandleRequest, &s);
//   if (ret) panic("ret %d", ret);

//   rt::WaitGroup w(1);
//   w.Wait();
}

int main(int argc, char *argv[]) {
  int ret;

  if (argc != 3) {
    std::cerr << "usage: [cfg_file] [portno]" << std::endl;
    return -EINVAL;
  }

  listen_addr.port = atoi(argv[2]);

  rocksdb_options_t *options = rocksdb_options_create();
  rocksdb_options_set_allow_mmap_reads(options, 1);
  rocksdb_options_set_allow_mmap_writes(options, 1);
  rocksdb_slicetransform_t *prefix_extractor =
      rocksdb_slicetransform_create_fixed_prefix(8);
  rocksdb_options_set_prefix_extractor(options, prefix_extractor);
  // rocksdb_options_set_plain_table_factory(options, 0, 10, 0.75, 3);
  // Optimize RocksDB. This is the easiest way to
  // get RocksDB to perform well
  rocksdb_options_increase_parallelism(options, 0);
  rocksdb_options_optimize_level_style_compaction(options, 0);
  // create the DB if it's not already present
  rocksdb_options_set_create_if_missing(options, 1);


  // open DB
  char *err = NULL;
  char DBPath[] = "/tmp/my_db";
  db = rocksdb_open(options, DBPath, &err);
  if (err) {
    log_err("Could not open RocksDB database: %s\n", err);
    exit(1);
  }
  log_info("Initialized RocksDB\n");
  ret = runtime_init(argv[1], MainHandler, NULL);
  if (ret) {
    std::cerr << "failed to start runtime" << std::endl;
    return ret;
  }

  // MainHandler(NULL);
  return 0;
}