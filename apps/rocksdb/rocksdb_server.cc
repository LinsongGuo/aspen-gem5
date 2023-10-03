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
#include <stdlib.h>
#include <time.h>
#include <algorithm>

long long now() {
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ts.tv_sec * 1e9 + ts.tv_nsec;
}

#define N 5000
std::string scan_keys[N];
char* keys[N];
char* values[N];
int keys_len[N], values_len[N];

char* gen_key(int id, int &keylen) {
  char* key = (char*) malloc(10);
  snprintf(key, 10, "key%d", id);
  keylen = strlen(key);
  return key;
}

char* gen_value(int &valuelen) {
  valuelen = rand() % 10 + 10;
  char* value = (char*) malloc(valuelen + 10);
  for (int i = 0; i < valuelen; i++) {
    value[i] = 'a' + rand() % 26; 
  }
  value[valuelen] = '\0'; 
  return value;
}

void init_key_value() {
  for (int i = 0; i < N; ++i) {
    keys[i] = gen_key(i, keys_len[i]);
    values[i] = gen_value(values_len[i]);
  }
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
  #ifdef SIGNAL_PREEMPT
		rt::SignalUnblock();
  #else
    _stui();
  #endif

  const char *retr_key;
  size_t klen;
  rocksdb_iterator_t *iter = rocksdb_create_iterator(db, readoptions);
  rocksdb_iter_seek_to_first(iter);
  while (rocksdb_iter_valid(iter)) {
    // printf("while\n");
    retr_key = rocksdb_iter_key(iter, &klen);
    // log_debug("Scanned key %.*s\n", (int)klen, retr_key);
    rocksdb_iter_next(iter);
  } 
  rocksdb_iter_destroy(iter);

  #ifdef SIGNAL_PREEMPT
  	rt::SignalBlock();
  #else
    _clui();
  #endif
}

static inline void DoGet(rocksdb_readoptions_t *readoptions) {
  size_t klen;
  const char *retr_key;
  rocksdb_iterator_t *iter = rocksdb_create_iterator(db, readoptions);
  rocksdb_iter_seek_to_first(iter);
  if (rocksdb_iter_valid(iter)) {
    retr_key = rocksdb_iter_key(iter, &klen);
    // log_debug("Scanned key %.*s\n", (int)klen, retr_key);
  }
  rocksdb_iter_destroy(iter);
}

static inline void DoGet(rocksdb_readoptions_t *readoptions, int i) { 
  // printf("get starts: %s %d\n", keys[i], keys_len[i]);

  char* err = NULL;
  size_t valuelen = 0;
  char *returned_value =
    rocksdb_get(db, readoptions, keys[i], keys_len[i], &valuelen, &err); 
  
  assert(!err);
  assert(returned_value != NULL && strncmp (returned_value, values[i], values_len[i]) == 0);
  if ( strncmp (returned_value, values[i], values_len[i]) != 0 ) {
    printf("wrong value\n");
  }
  free(returned_value);

  // printf("get ends\n");
}

uint64_t trace_start[2*1000*1000], trace_end[2*1000*1000], trace_time[2*1000*1000];
unsigned cnt = 0;

void print() {
  printf("print starts\n");

  FILE* file = fopen("trace.txt", "w");
  for (unsigned i = 1; i < cnt; ++i) {
    fprintf(file, "%.6f  %.6f  %lu\n", 1.*(trace_start[i] - trace_start[0])/2000/1000/1000, 1.*(trace_start[i] - trace_start[i-1])/2000/1000/1000, (trace_end[i] - trace_start[i])/2);
    uint64_t gap = (trace_start[i] - trace_start[i-1])/2; 
    if (gap > 1000000) {
      fprintf(file, "weird %d %.6f\n", i, 1.*(trace_start[i] - trace_start[0])/2000/1000/1000);
    }
  }
  fclose(file);

  unsigned long long total = 0;
  for (unsigned i = 1; i < cnt; ++i) {
    trace_time[i] = trace_end[i] - trace_start[i];
    total += trace_time[i];
  }
  std::sort(trace_time, trace_time+cnt);
  printf("avg: %.3f us\n", 1.*total/cnt/2000);
  printf("mean: %.3f us\n", 1.*trace_time[cnt/2]/2000);
  printf("90th: %.3f us\n", 1.*trace_time[int(cnt*0.9)]/2000);
  printf("99th: %.3f us\n", 1.*trace_time[int(cnt*0.99)]/2000);
  printf("99.9th: %.3f us\n", 1.*trace_time[int(cnt*0.999)]/2000);
  printf("print ends\n");
}

static void HandleRequest(udp_spawn_data *d) {
  const Payload *p = static_cast<const Payload *>(d->buf);
 
  rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
  if (p->req_type == 11)
    DoScan(readoptions);
  else if (p->req_type == 10) {
    // trace_start[cnt] = rdtscp(NULL);
    DoGet(readoptions, p->reqsize);
    // trace_end[cnt++] = rdtscp(NULL);
    // if (cnt == 1*1000*1000) {
    //   print();
    //   exit(-1);
    // }
  }
  else
    panic("bad req type %u", p->req_type);
  rocksdb_readoptions_destroy(readoptions);
  // barrier();

  Payload rp = *p;
  rp.run_ns = 0;

  ssize_t wret = udp_respond(&rp, sizeof(rp), d);
  if (unlikely(wret <= 0)) panic("wret");
  udp_spawn_data_release(d->release_data);
}

void Get5000() {
  char *err = NULL;
  uint64_t durations[5000];
  rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
  int cnt = 0;
  long long total = 0;
  // for (int j = 0; j < 20000; ++j) {
  //  int i = rand() % N;
  for (int i = 0; i < N; i++) {
    size_t valuelen = 0; 
    char* key_ = keys[i];
    int keylen = keys_len[i];

    uint64_t start = rdtscp(NULL);
    barrier();
    char *returned_value =
      rocksdb_get(db, readoptions, key_, keylen, &valuelen, &err);
    barrier();
    uint64_t end = rdtscp(NULL);
    durations[cnt++] = end - start;
    total += end - start;
    
    assert(!err);
    assert(returned_value != NULL && strncmp (returned_value, values[i], values_len[i]) == 0);
    // if ( strncmp (returned_value, values[i], values_len[i]) != 0 ) {
    //   unsigned char uif = _testui();
    //   if (uif)
    //     _clui();
    //   printf("wrong value\n");
    //   printf("%s %s %d\n", returned_value, values[i], values_len[i]);
    //   if (uif)
    //     _stui();
    // }
    free(returned_value);
  }

  unsigned char uif = _testui();
  if (uif)
    _clui();
  uint64_t first = durations[0];
  std::sort(std::begin(durations), std::end(durations));
  fprintf(stderr, "stats for %u iterations (GET): \n", cnt);
  fprintf(stderr, "avg: %0.3f\n",
          (double)total / cnt / (double)cycles_per_us);
  fprintf(stderr, "median: %0.3f\n",
          (double)durations[cnt / 2] / (double)cycles_per_us);
  fprintf(stderr, "p99.9: %0.3f\n",
          (double)durations[cnt * 999 / 1000] / (double)cycles_per_us);
  fprintf(stderr, "first: %0.3f\n",
          (double)first / (double)cycles_per_us);
  if (uif)
    _stui();
}

void Put5000() {
  uint64_t durations[N];
  int cnt = 0;

  char *err = NULL;
  rocksdb_writeoptions_t *writeoptions = rocksdb_writeoptions_create();
  // const char *value = "value";
  for (int i = 0; i < N; i++) {
    uint64_t start = rdtscp(NULL);
    rocksdb_put(db, writeoptions, keys[i], keys_len[i], values[i], values_len[i]+1, &err);
    // rocksdb_put(db, writeoptions, keys[i], keys_len[i], value, strlen(value)+1, &err);
    uint64_t end = rdtscp(NULL);
    durations[cnt++] = end - start;

    if (err) {
      printf("PUT failed: %s\n", err);
      exit(-1);
    }
    assert(!err);
  }

  unsigned char uif = _testui();
  if (uif)
    _clui();
  std::sort(std::begin(durations), std::end(durations));
  fprintf(stderr, "stats for %u iterations (PUT): \n", cnt);
  fprintf(stderr, "median: %0.3f\n",
          (double)durations[cnt / 2] / (double)cycles_per_us);
  fprintf(stderr, "p99.9: %0.3f\n",
          (double)durations[cnt * 999 / 1000] / (double)cycles_per_us);
  if (uif)
    _stui();
}

void PutInit() {
  char *err = NULL;
  rocksdb_writeoptions_t *writeoptions = rocksdb_writeoptions_create();
  // const char *value = "value";
  for (int i = 0; i < N; i++) {
    //printf("For %d: %s %d %d %s %d\n", i, keys[i], keys_len[i], strlen(keys[i]), values[i], values_len[i]);
    rocksdb_put(db, writeoptions, keys[i], keys_len[i], values[i], values_len[i]+1, &err);
    // rocksdb_put(db, writeoptions, keys[i], keys_len[i], value, strlen(value)+1, &err);
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
    // printf("%s\n", retr_key);
    scan_keys[idx++] = std::string(retr_key, (int)klen);
    rocksdb_iter_next(iter);
  }
  rocksdb_iter_destroy(iter);
}

typedef void (*bench_type)(void);

void MainHandler2(void *arg) {
  srand(123);

  rt::WaitGroup wg(1);

  cycles_per_us = 2000;
  init_key_value();
  rocksdb_init();
  PutInit();

  const int task_num = 2;
  std::string bench_name[task_num] = {"Get5000", "Get5000"};
  bench_type bench_ptr[task_num] = {Get5000, Get5000};
  
  int started = 0, finished = 0;
  for (int i = 0; i < task_num; ++i) {
		rt::Spawn([&, i]() {
			started += 1;

      // printf("call %s()\n", bench_name[i].c_str());
     	if (started < task_num) {
        rt::UintrTimerStart();
				rt::Yield();
			}

#ifdef SIGNAL_PREEMPT
			rt::SignalUnblock();
#else
			_stui();
#endif
			bench_ptr[i]();
#ifdef SIGNAL_PREEMPT
			rt::SignalBlock();
#else
			_clui();
#endif
			
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

void MainHandler3(void *arg) {
  rt::WaitGroup wg(1);

  cycles_per_us = 2000;
  init_key_value();
  rocksdb_init();
  PutInit();

  for (int i = 0; i < 5000; ++i) {
    rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
    DoGet(readoptions, i);
    rocksdb_readoptions_destroy(readoptions);
	}

  const int task_num = 20000;
  uint64_t durations[task_num];
  int cnt = 0;
  unsigned long long total = 0;
  for (int i = 0; i < task_num; ++i) {
		rt::Spawn([&, i]() {
      int j = rand() % 5000;
      rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
      uint64_t start = rdtscp(NULL);
      barrier();
      DoGet(readoptions, j);
      barrier();
      uint64_t end = rdtscp(NULL);
      rocksdb_readoptions_destroy(readoptions);
      durations[i] += end - start;
      total += end - start;
      cnt++;
      if (cnt == task_num) {
        wg.Done();
      }
		});
	}
          
  wg.Wait();

  uint64_t first = durations[0];
  std::sort(std::begin(durations), std::end(durations));
  fprintf(stderr, "stats for %u iterations (GET): \n", cnt);
  fprintf(stderr, "avg: %0.3f\n",
          (double)total / cnt / (double)cycles_per_us);
  fprintf(stderr, "median: %0.3f\n",
          (double)durations[cnt / 2] / (double)cycles_per_us);
  fprintf(stderr, "p99.9: %0.3f\n",
          (double)durations[cnt * 999 / 1000] / (double)cycles_per_us);
  fprintf(stderr, "first: %0.3f\n",
          (double)first / (double)cycles_per_us);


  rt::WaitGroup wg2(1);
  int task_num2 = 5000;
  cnt = 0;
  total = 0;
  for (int i = 0; i < task_num2; ++i) {
		rt::Spawn([&, i]() {
      rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
      uint64_t start = rdtscp(NULL);
      barrier();
      DoScan(readoptions);
      barrier();
      uint64_t end = rdtscp(NULL);
      rocksdb_readoptions_destroy(readoptions);
      durations[i] += end - start;
      total += end - start;
      cnt++;
      if (cnt == task_num2) {
        wg2.Done();
      }
		});
	}
          
  wg2.Wait();

  first = durations[0];
  std::sort(durations, durations+cnt);
  fprintf(stderr, "stats for %u iterations (Scan): \n", cnt);
  fprintf(stderr, "avg: %0.3f\n",
          (double)total / cnt / (double)cycles_per_us);
  fprintf(stderr, "median: %0.3f\n",
          (double)durations[cnt / 2] / (double)cycles_per_us);
  fprintf(stderr, "p99.9: %0.3f\n",
          (double)durations[cnt * 999 / 1000] / (double)cycles_per_us);
  fprintf(stderr, "first: %0.3f\n",
          (double)first / (double)cycles_per_us);
}

void MainHandler4(void *arg) {
  cycles_per_us = 2000;
  init_key_value();
  rocksdb_init();
  PutInit();

  for (int i = 0; i < 100; ++i) {
    rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
    DoScan(readoptions);
    rocksdb_readoptions_destroy(readoptions);
	}

  rt::WaitGroup wg(1);
  int task_num = 100, cnt = 0;
  rt::UintrTimerStart();
  uint64_t start = rdtscp(NULL);
  barrier();
  for (int i = 0; i < task_num; ++i) {
		rt::Spawn([&, i]() {
      if (i + 1 < task_num) {
        rt::Yield();
      }
      
      rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
      DoScan(readoptions);
      rocksdb_readoptions_destroy(readoptions);
      
      cnt++;
      if (cnt == task_num) {
        rt::UintrTimerEnd();
        rt::UintrTimerSummary();
        wg.Done();
      }
		});
	}
          
  wg.Wait();
  barrier();
  uint64_t end = rdtscp(NULL);
  unsigned long long total = end - start;

  fprintf(stderr, "stats for %u iterations (Scan): \n", cnt);
  fprintf(stderr, "avg: %0.3f\n",
          (double)total / cnt / (double)cycles_per_us);
}

void MainHandler(void *arg) {
  cycles_per_us = 2000;
  init_key_value();
  rocksdb_init();
  PutInit();

  // char *err = NULL;
  unsigned int i = 0;

  uint64_t durations[1000];
  unsigned long long total = 0;
  for (i = 0; i < 1000; i++) {
    rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
    uint64_t start = rdtscp(NULL);
    // barrier();
    DoScan(readoptions);
    // barrier();
    uint64_t end = rdtscp(NULL);
    rocksdb_readoptions_destroy(readoptions);
    durations[i] = end - start;
    total += end - start;
  }
  std::sort(std::begin(durations), std::end(durations));
  fprintf(stderr, "stats for %u Scan iterations: \n", i);
  fprintf(stderr, "avg: %0.3f\n",
          (double)total / i / (double)cycles_per_us);
  fprintf(stderr, "median: %0.3f\n",
          (double)durations[i / 2] / (double)cycles_per_us);
  fprintf(stderr, "p99.9: %0.3f\n",
          (double)durations[i * 999 / 1000] / (double)cycles_per_us);

  uint64_t durations2[5000];  
  total = 0;
  for (i = 0; i < 5000; i++) {
    int j = rand() % 5000;
    rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
    uint64_t start = rdtscp(NULL);
    barrier();
    DoGet(readoptions, j);
    barrier();
    uint64_t end = rdtscp(NULL);
    rocksdb_readoptions_destroy(readoptions);
    durations2[i] = end - start;
    total += end - start;
  }
 
  std::sort(std::begin(durations2), std::end(durations2));
  fprintf(stderr, "stats for %u Get iterations: \n", i);
  fprintf(stderr, "avg: %0.3f\n",
          (double)total / i / (double)cycles_per_us);
  fprintf(stderr, "median: %0.3f\n",
          (double)durations2[i / 2] / (double)cycles_per_us);
  fprintf(stderr, "p99.9: %0.3f\n",
          (double)durations2[i * 999 / 1000] / (double)cycles_per_us);

  rt::UintrTimerStart();
 
  udpspawner_t *s;
  int ret = udp_create_spawner(listen_addr, HandleRequest, &s);
  if (ret) panic("ret %d", ret);

  rt::WaitGroup w(1);
  w.Wait();
}

void rocksdb_init() {
  rocksdb_options_t *options = rocksdb_options_create();

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
  // printf("options ends\n");

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
  printf("######### main\n");
  srand(time(NULL));
  
  int ret;

  if (argc != 3) {
    std::cerr << "usage: [cfg_file] [portno]" << std::endl;
    return -EINVAL;
  }

  listen_addr.port = atoi(argv[2]);
  	
#ifdef SIGNAL_PREEMPT
	rt::SignalBlock();
#endif

  ret = runtime_init(argv[1], MainHandler2, NULL);
  if (ret) {
    std::cerr << "failed to start runtime" << std::endl;
    return ret;
  }

  // MainHandler(NULL);
  return 0;
}