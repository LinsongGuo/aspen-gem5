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
#include "timer.h"
#include <x86intrin.h>
#include <mutex>

long long now() {
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ts.tv_sec * 1e9 + ts.tv_nsec;
}

static rocksdb_t *db;
static netaddr listen_addr;

struct Payload {
  uint32_t id;
  uint32_t req_type;
  uint32_t reqsize;
  uint32_t run_ns;
};

void rocksdb_init();

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
int Get5000() {
  int res = 0;
  char *err = NULL;
  uint64_t durations[5000];
  rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
  int cnt = 0;
  for (int i = 0; i < 5000; i++) {

    size_t len = 0; 

    // const char* key = scan_keys[i].c_str();
    // size_t keylen = scan_keys[i].length();
    // char key_[10];
    // memcpy(key_, key, keylen);
    // key_[keylen] = '\0';

    std::string scan_key = "key" + std::to_string(i);
    const char* key = scan_key.c_str();
    size_t keylen = scan_key.length();
    char key_[10];
    memcpy(key_, key, keylen);
    key_[keylen] = '\0';



    uint64_t start = rdtscp(NULL);
    char *returned_value =
      rocksdb_get(db, readoptions, key_, keylen, &len, &err);
    uint64_t end = rdtscp(NULL);
    durations[cnt++] = end - start;
    
    assert(!err);

    if (returned_value == NULL || strcmp(returned_value, "value") != 0) {
      unsigned char uif = _testui();
      if (uif)
      _clui();
      printf("for %d, key: %s (%d), value: %p, value len: %d\n", i, key_, strlen(key_), returned_value, len);
      if (len > 0)
        printf("value: %s %d %d\n", returned_value, returned_value == NULL, strcmp(returned_value, "value") == 0);
      if (uif)
      _stui();
    } 
    else {
      res++;
    }
    assert(strcmp(returned_value, "value") == 0);
    free(returned_value);
  }

  unsigned char uif = _testui();
  if (uif)
    _clui();
  std::sort(std::begin(durations), std::end(durations));
  fprintf(stderr, "stats for %u iterations (GET): \n", cnt);
  fprintf(stderr, "median: %0.3f\n",
          (double)durations[cnt / 2] / (double)cycles_per_us);
  fprintf(stderr, "p99.9: %0.3f\n",
          (double)durations[cnt * 999 / 1000] / (double)cycles_per_us);
  if (uif)
    _stui();
}

void Put5000() {
  printf("~~~~~~~~~~ Put5000()\n");
  // Put key-value
  char *err = NULL;
  rocksdb_writeoptions_t *writeoptions = rocksdb_writeoptions_create();
  const char *value = "value";
  for (int i = 0; i < 5000; i++) {
    char key[10];
    // char value[64];
    // snprintf(key, sizeof(key), "%d", i);
    snprintf(key, 10, "key%d", i);
    //  snprintf(value, sizeof(value), "%lld", dist(e2));
    rocksdb_put(db, writeoptions, key, strlen(key), value, strlen(value) + 1,
                &err);
    if (err) {
      printf("PUT failed: %s\n", err);
      exit(-1);
    }
    assert(!err);
  }
}
static inline void ScanInit(rocksdb_readoptions_t *readoptions) {
  const char *retr_key;
  size_t klen;

  rocksdb_iterator_t *iter = rocksdb_create_iterator(db, readoptions);
  rocksdb_iter_seek_to_first(iter);
  int idx = 0;
  while (rocksdb_iter_valid(iter)) {
    retr_key = rocksdb_iter_key(iter, &klen);
    // log_debug("Scanned key %.*s", (int)klen, retr_key);
    scan_keys[idx++] = std::string(retr_key, (int)klen);
    rocksdb_iter_next(iter);
  }
  rocksdb_iter_destroy(iter);

  // for (int i = 0; i < 5000; ++i) {
  //   std::cout << scan_keys[i] << ' ';
  // }
}

std::mutex mu_;
  
void MainHandler(void *arg) {
  rt::WaitGroup wg(1);

  std::unique_lock<std::mutex> lock(mu_);

  rocksdb_init();

  cycles_per_us = 2000;

  Put5000();

  rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
  ScanInit(readoptions);
  rocksdb_readoptions_destroy(readoptions);
  
  // Get5000();
 
  printf("############## main handler\n");
  rt::Sleep(1000 * rt::kMilliseconds);
  
  // thread_local int res = 0;

  long long start = now();
  int task_num = 2, started = 0, finished = 0;
  for (int i = 0; i < task_num; ++i) {
		rt::Spawn([&, i]() {
			started += 1;

      // Get5000();
      
			if (started < task_num) {
        rt::UintrTimerStart();
				rt::Yield();
			}
			
      printf("call Get()\n");

			_stui();
			Get5000();
      _clui();
			
      // res += tmp;
			
			finished += 1;
			if (finished == task_num) {
				rt::UintrTimerEnd();
				rt::UintrTimerSummary();
				wg.Done();
        // printf("res: %d\n", res);
			}
		});
	}

  long long end = now();
  printf("Get execution time: %.6f\n", 1. * (end - start) / 1e9);
  
  wg.Wait();
}

void rocksdb_init() {
  printf("rocksdb_init()\n");

  rocksdb_options_t *options = rocksdb_options_create();
  printf("rocksdb_options_create()\n");

  rocksdb_options_set_allow_mmap_reads(options, 1);
  rocksdb_options_set_allow_mmap_writes(options, 1);
  rocksdb_slicetransform_t *prefix_extractor =
      rocksdb_slicetransform_create_fixed_prefix(4);
  rocksdb_options_set_prefix_extractor(options, prefix_extractor);
  rocksdb_options_set_plain_table_factory(options, 0, 10, 0.75, 3);
  // Optimize RocksDB. This is the easiest way to
  // get RocksDB to perform well
  rocksdb_options_increase_parallelism(options, 0);
  // rocksdb_options_optimize_level_style_compaction(options, 0);
  // create the DB if it's not already present
  rocksdb_options_set_create_if_missing(options, 1);

  printf("options ends\n");

  // open DB
  char *err = NULL;
  char DBPath[] = "/tmp/my_db";
  db = rocksdb_open(options, DBPath, &err);
  if (err) {
    log_err("Could not open RocksDB database: %s\n", err);
    exit(1);
  }
  log_info("Initialized RocksDB\n");
}

int main(int argc, char *argv[]) {
  int ret;

  if (argc != 3) {
    std::cerr << "usage: [cfg_file] [portno]" << std::endl;
    return -EINVAL;
  }


  listen_addr.port = atoi(argv[2]);

  ret = runtime_init(argv[1], MainHandler, NULL);
  if (ret) {
    std::cerr << "failed to start runtime" << std::endl;
    return ret;
  }

  // MainHandler(NULL);
  return 0;
}