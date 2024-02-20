extern "C" {
#include <base/byteorder.h>
#include <base/log.h>
#include <runtime/runtime.h>
#include <runtime/udp.h>
#include <runtime/uintr.h>
#include "concord-rocksdb.h"
}

#include <c.h>
#include <algorithm>
#include <iostream>
#include "sync.h"
#include "uintr.h"
#include "thread.h"
#include "timer.h"
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
    // values[i] = gen_value(values_len[i]);
    values[i] = gen_key(i, values_len[i]);
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

void get_test() {
  char *err = NULL;
  for (int k = 0; k < 720; ++k) {
    rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
    for (int i = 0; i < N; i++) {
      int k = 1LL * i * i * i % N;
      DoGet(db, readoptions, keys[k], keys_len[k]);
    }
    rocksdb_readoptions_destroy(readoptions);
  }
}

void scan_test() {
  char *err = NULL;
  for (int i = 0; i < 10000; i++) {
    rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
    DoScan(db, readoptions);
    rocksdb_readoptions_destroy(readoptions);
  }
}

void hybrid_test() {
  rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
  for (int i = 0; i < 10000; i++) {
    if (i & 1) {
      int k = 1LL * i * i * i % N;
      DoGet(db, readoptions, keys[k], keys_len[k]);
    }
    else {
      DoScan(db, readoptions);
    }
  }
  rocksdb_readoptions_destroy(readoptions);
}

static void HandleRequest(udp_spawn_data *d) {
  const Payload *p = static_cast<const Payload *>(d->buf);
 
  rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
  if (p->req_type == 11) {
    DoScan(db, readoptions);
  }
  else if (p->req_type == 10) {
    DoGet(db, readoptions, keys[p->reqsize], keys_len[p->reqsize]);
  }
  else
    panic("bad req type %u", p->req_type);
  rocksdb_readoptions_destroy(readoptions);
 
  Payload rp = *p;
  rp.run_ns = 0;

  ssize_t wret = udp_respond(&rp, sizeof(rp), d);
  if (unlikely(wret <= 0)) panic("wret");
  udp_spawn_data_release(d->release_data);
}

static void HandleLoop(udpconn_t *c) {
  char buf[20];
	ssize_t ret;
	struct netaddr addr;

	while (true) {
		ret = udp_read_from(c, buf, sizeof(Payload), &addr);
    assert(ret == sizeof(Payload));
    
    const Payload *p = static_cast<const Payload *>((void*)buf);
    rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
    if (p->req_type == 11) {
      DoScan(db, readoptions);
      // DoScan(readoptions);
    }
    else if (p->req_type == 10) {
      // DoGet(readoptions, p->reqsize);
      DoGet(db, readoptions, keys[p->reqsize], keys_len[p->reqsize]);
    }
    else
      panic("bad req type %u", p->req_type);
    rocksdb_readoptions_destroy(readoptions);
    
    Payload rp = *p;
    rp.run_ns = 0;
    ret = udp_write_to(c, &rp, sizeof(Payload), &addr);
    assert(ret == sizeof(Payload));
	}
}

void Get5000() {
  char *err = NULL;
  uint64_t durations[5000];
  rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
  int cnt = 0;
  long long total = 0;
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
    if ( strncmp (returned_value, values[i], values_len[i]) != 0 ) {
      // unsigned char uif = _testui();
      // if (uif)
      //   _clui();
      printf("wrong value\n");
      printf("%s %s %d\n", returned_value, values[i], values_len[i]);
      // if (uif)
      //   _stui();
    }
    free(returned_value);
  }

  // unsigned char uif = _testui();
  // if (uif)
  //   _clui();
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
  // if (uif)
  //   _stui();
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

void GetScanInit() {
  unsigned int i = 0;

  uint64_t durations[1000];
  unsigned long long total = 0;
  for (i = 0; i < 1000; i++) {
    rocksdb_readoptions_t *readoptions = rocksdb_readoptions_create();
    uint64_t start = rdtscp(NULL);
    barrier();
    DoScan(db, readoptions);
    barrier();
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
    // DoGet(readoptions, j);
    DoGet(db, readoptions, keys[j], keys_len[j]);
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
}

typedef void (*bench_type)(void);

void MainHandler_local(void *arg) {
  srand(123);

  rt::WaitGroup wg(1);

  cycles_per_us = 2000;
  init_key_value();
  rocksdb_init();

  // const int task_num = 2;
  // std::string bench_name[task_num] = {"Get5000", "Get5000"};
  // bench_type bench_ptr[task_num] = {Get5000, Get5000};

  // const int task_num = 1;
  // std::string bench_name[task_num] = {"get_test"};
  // bench_type bench_ptr[task_num] = {get_test};

  // const int task_num = 1;
  // std::string bench_name[task_num] = {"scan_test"};
  // bench_type bench_ptr[task_num] = {scan_test};

  const int task_num = 2;
  std::string bench_name[task_num] = {"get_test", "scan_test"};
  bench_type bench_ptr[task_num] = {get_test, scan_test};

  int started = 0, finished = 0;
  for (int i = 0; i < task_num; ++i) {
		rt::Spawn([&, i]() {
      if (started == 0) {
         rt::UintrTimerStart();
      }

			started += 1;
     	if (started < task_num) {
        rt::Yield();
			}

      bench_ptr[i]();
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


void wramup() {
  const int task_num = 24;
  rt::WaitGroup wg(24);
  long long start = now();
  barrier();
  for (int i = 0; i < task_num; ++i) {
		rt::Spawn([&, i]() {
			// scan_test();
      hybrid_test();
      wg.Done();
		});
	}
  wg.Wait();
  barrier();
  long long end = now();
  printf("Wramup Ends: %.3f\n", 1.*(end - start) / 1e9);
}

void MainHandler(void *arg) {
  cycles_per_us = 2000;
  init_key_value();
  rocksdb_init();

  rt::UintrTimerStart();

  udpspawner_t *s;
  int ret = udp_create_spawner(listen_addr, HandleRequest, &s);
  if (ret) panic("ret %d", ret);

  rt::WaitGroup w(1);
  w.Wait();
}

unsigned num_port, num_conn;
void MainHandler_udpconn(void *arg) {
  cycles_per_us = 2000;
  init_key_value();
  rocksdb_init();
  
  rt::WaitGroup wg(1);
  rt::UintrTimerStart();
  
  // wramup();

  for (unsigned port = 0; port < num_port; ++port) {
    udpconn_t *c;
    ssize_t ret;
    listen_addr.port = 5000 + port;
    ret = udp_listen(listen_addr, &c);
    if (ret) {
      log_err("stat: udp_listen failed, ret = %ld", ret);
      return;
    }
    
<<<<<<< HEAD
    for (int i = 0; i < 32; ++i) {
=======
    for (unsigned i = 0; i < num_conn; ++i) {
>>>>>>> formal_apps
      rt::Spawn([&, c]() {
        HandleLoop(c);
      });
    }
  }

  wg.Wait();
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

  // open DB
  char *err = NULL;
  char DBPath[] = "/tmp/my_db";
  db = rocksdb_open(options, DBPath, &err);
  if (err) {
    log_err("Could not open RocksDB database: %s\n", err);
    exit(1);
  }
  log_info("Initialized RocksDB\n");

  PutInit();

  GetScanInit();
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "usage: [cfg_file] [mode=local|udp|udpconn]" << std::endl;
    return -EINVAL;
  }

  int ret;
  std::string mode = argv[2];
  if (mode == "local") {
    ret = runtime_init(argv[1], MainHandler_local, NULL);
  } else if (mode == "udp") {
    listen_addr.port = 5000;
    ret = runtime_init(argv[1], MainHandler, NULL);
  } else if (mode == "udpconn") {
    if (argc < 5) {
      std::cerr << "usage: [cfg_file] udpconn [num of ports] [num of connections]" << std::endl;
      return -EINVAL;
    }
    listen_addr.port = 5000;
    num_port = atoi(argv[3]);
    num_conn = atoi(argv[4]);
    ret = runtime_init(argv[1], MainHandler_udpconn, NULL);
  } else {
    std::cerr << "no mode given" << std::endl;
    return -EINVAL;
  }
  
  if (ret) {
    std::cerr << "failed to start runtime" << std::endl;
    return ret;
  }

  return 0;
}
// get: 0.715
// get concord: 0.721
// get concordfl: 0.790 