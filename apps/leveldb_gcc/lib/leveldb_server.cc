// Taken from https://github.com/emnl/leveldb-c-example/blob/master/leveldb_example.c
// Modfiied to use the concord-leveldb wrapper
extern "C" {

#include <runtime/runtime.h>
#include <runtime/udp.h>
#include <runtime/concord.h>
#include <base/log.h>
}

#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <x86intrin.h>
#include <time.h>
#include <sys/time.h>
#include <leveldb/c.h> 
#include "concord-leveldb.h"

#include "sync.h"
#include "uintr.h"
#include "thread.h"
#include "timer.h"

#define NUM_KEYS     15000
#define NUM_TRIALS    100

leveldb_t *db;
static netaddr listen_addr;

struct Payload {
  uint32_t id;
  uint32_t req_type;
  uint32_t reqsize;
  uint32_t run_ns;
};

static inline uint64_t get_ns()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_nsec;
}

char* keys[NUM_KEYS];
// char* values[NUM_KEYS];
size_t keys_len[NUM_KEYS];
// size_t values_len[NUM_KEYS];

char* gen_key(size_t id, size_t &keylen) {
  char* key = (char*) malloc(10);
  sprintf(key, "keyz%zu", id);
  keylen = strlen(key);
  return key;
}

// char* gen_value(size_t &valuelen) {
//   valuelen = rand() % 10 + 10;
//   char* value = (char*) malloc(valuelen + 10);
//   for (int i = 0; i < valuelen; i++) {
//     value[i] = 'a' + rand() % 26; 
//   }
//   value[valuelen] = '\0'; 
//   return value;
// }

void init_key_value() {
  for (size_t i = 0; i < NUM_KEYS; ++i) {
    keys[i] = gen_key(i, keys_len[i]);
    // values[i] = gen_value(values_len[i]);
  }
}

inline void DoGet(size_t kn) {
  leveldb_readoptions_t *roptions;
  roptions = leveldb_readoptions_create();
  size_t val_len;
  char *err = NULL;
  char* val = cncrd_leveldb_get(db, roptions, keys[kn], keys_len[kn], &val_len, &err);
  if (val == NULL || strncmp(keys[kn], val, val_len)) {
    printf("wrong: %s (%zu) %s (%zu)\n", keys[kn], keys_len[kn], val, val_len);
  }
  leveldb_readoptions_destroy(roptions);
}

inline void DoScan() {
  leveldb_readoptions_t *roptions;
  roptions = leveldb_readoptions_create();
  cncrd_leveldb_scan(db, roptions, NULL);  
  leveldb_readoptions_destroy(roptions);
  // for (int k = 0; k < 500000 * 3; ++k) {
  //   asm volatile("nop");
  // }
}

static void HandleRequest(udp_spawn_data *d) {
  const Payload *p = static_cast<const Payload *>(d->buf);
 
  if (p->req_type == 11)
    DoScan();
  else if (p->req_type == 10) {
    DoGet(p->reqsize);
  }
  else {
    panic("bad req type %u", p->req_type);
  }
  
  Payload rp = *p;
  rp.run_ns = 0;

  ssize_t wret = udp_respond(&rp, sizeof(rp), d);
  if (unlikely(wret <= 0)) panic("wret");
  udp_spawn_data_release(d->release_data);
}


void get_test() {
  leveldb_readoptions_t *roptions;
  roptions = leveldb_readoptions_create();
  size_t read_len;
  char *err = NULL;
  leveldb_free(err); err = NULL;
  for (int k = 0; k < NUM_TRIALS * 1000; k++) {
    // printf("k%d\n", k);
    char test_key[100];
    // size_t kn = rand() % NUM_KEYS;
    size_t kn = 1LL * k * k * 17 % NUM_KEYS;
    // sprintf(test_key,"keyz%zu", kn);
    // printf("read %zu %s (%zu)\n", kn, test_key, strlen(test_key));

    // unsigned char uif = _testui();
    // if (uif)
    //   _clui();

    char* val = cncrd_leveldb_get(db, roptions, keys[kn], keys_len[kn], &read_len, &err);

    // if (uif)
    //   _stui();
    if (val == NULL || strncmp(keys[kn], val, read_len)) {
      printf("%zu: %s (%zu) %s (%zu) err: %s\n", kn, keys[kn], keys_len[kn], val, read_len, err);
    }
  }
}

void scan_test() {
  leveldb_readoptions_t *roptions;
  roptions = leveldb_readoptions_create();
  for (int k = 0; k < NUM_TRIALS * 10; k++) {
    // cncrd_leveldb_scan(db, roptions, NULL);
    DoScan();
  }
}

void hybrid_test() {
  leveldb_readoptions_t *roptions;
  roptions = leveldb_readoptions_create();
  for (int k = 0; k < NUM_TRIALS * 100; k++) {
    if (rand() & 1) {
      cncrd_leveldb_scan(db, roptions, NULL);
    }
    else {
      char test_key[100];
      size_t read_len;
      char *err = NULL;
      size_t kn = 1LL * k * k * 17 % NUM_KEYS;
      // sprintf(test_key,"keyz%zu", kn);
      // printf("read %zu %s (%zu)\n", kn, test_key, strlen(test_key));
      // char* val = cncrd_leveldb_get(db, roptions, test_key, strlen(test_key), &read_len, &err);
      char* val = cncrd_leveldb_get(db, roptions, keys[kn], keys_len[kn], &read_len, &err);
      if (val == NULL || strncmp(keys[kn], val, read_len)) {
        printf("%zu: %s (%zu) %s (%zu)\n", kn, keys[kn], keys_len[kn], val, read_len);
      }
    }
  }
}

void leveldb_init() {
  init_key_value();


  leveldb_options_t *options;
  leveldb_readoptions_t *roptions;
  leveldb_writeoptions_t *woptions;
  char *err = NULL;
  char *read;
  size_t read_len;

  /******************************************/
  /* OPEN */
  options = leveldb_options_create();
  leveldb_options_set_create_if_missing(options, 1);
  db = leveldb_open(options, "testdb", &err);
  if (err != NULL) {
    fprintf(stderr, "Open fail.\n");
    return;
  }

  /* reset error var */
  leveldb_free(err); err = NULL;


  /******************************************/
  /* WRITE */
  woptions = leveldb_writeoptions_create();
  cncrd_leveldb_put(db, woptions, "key", 3, "value", 5, &err);


  for (size_t i = 0; i < NUM_KEYS; i++)
  {
    // char key[100];
    // sprintf(key, "keyz%zu", i);
    // cncrd_leveldb_put(db, woptions, key, strlen(key), key, strlen(key), &err);
    cncrd_leveldb_put(db, woptions, keys[i], keys_len[i], keys[i], keys_len[i], &err);
    /* code */
  }
  if (err != NULL) {
    fprintf(stderr, "Write fail.\n");
    return;
  }
  leveldb_free(err); err = NULL;


  roptions = leveldb_readoptions_create();
  read = cncrd_leveldb_get(db, roptions, "key", 3, &read_len, &err);
  if (err != NULL) {
    fprintf(stderr, "Read fail.\n");
    return;
  }
  assert(strncmp(read, "value", 5) == 0);
  printf("Assert success || read: %s\n", read);   


  unsigned long long total_time = 0;  
  for (int k = 0; k < NUM_TRIALS; k++)
  {
    // char test_key[100];
    // size_t kn = rand() % NUM_KEYS;
    // sprintf(test_key, "keyz%zu", kn);
    // unsigned long long before = get_ns();
    // char* val = cncrd_leveldb_get(db, roptions, test_key, strlen(test_key), &read_len, &err);
    // if (strncmp(test_key, val, read_len)) {
    //   printf("%zu: %s (%zu) %s (%zu)\n", kn, test_key, strlen(test_key), val, read_len);
    // }
    // unsigned long long after = get_ns();
    // total_time += after - before;

    size_t kn = rand() % NUM_KEYS;
    unsigned long long before = get_ns();
    char* val = cncrd_leveldb_get(db, roptions, keys[kn], keys_len[kn], &read_len, &err);
    if (val == NULL || strncmp(keys[kn], val, read_len)) {
      printf("wrong: %s (%zu) %s (%zu)\n", keys[kn], keys_len[kn], val, read_len);
    }
    unsigned long long after = get_ns();
    total_time += after - before;
  }
  if (err != NULL) {
    fprintf(stderr, "Get fail.\n");
    return;
  }
  printf("Get: %llu ns\n", (total_time / NUM_TRIALS));
  leveldb_free(err); err = NULL;


  unsigned long long before = get_ns();
  for (int k = 0; k < NUM_TRIALS; k++) {
    DoScan();
    // cncrd_leveldb_scan(db, roptions, NULL);
  }
  unsigned long long after = get_ns();
  total_time = after - before;
  printf("Scan: %llu ns\n", (total_time / NUM_TRIALS));


  leveldb_options_destroy(options);
  leveldb_readoptions_destroy(roptions);
  leveldb_writeoptions_destroy(woptions);
  // leveldb_close(db);
  // leveldb_destroy_db(options, "testdb", &err);
  // leveldb_free(err); err = NULL;
}

void MainHandler_simple(void *arg) {
  leveldb_init();

  rt::WaitGroup wg(1);

  _stui();
  rt::UintrTimerStart();
  
  int started = 0, finished = 0, task_num = 10;
  for (int i = 0; i < task_num; ++i) {
		rt::Spawn([&, i]() {
			started += 1;

      // if (i & 1)
      // scan_test();
      // else 
      // get_test();
      // hybrid_test();
      scan_test();

      finished += 1;
			if (finished == task_num) {
				rt::UintrTimerEnd();
        wg.Done();
      }
		});
	}

  wg.Wait();
  rt::UintrTimerSummary();
}

void MainHandler(void *arg) {
  leveldb_init();
  
  rt::UintrTimerStart();
  
  udpspawner_t *s;
  int ret = udp_create_spawner(listen_addr, HandleRequest, &s);
  if (ret) panic("ret %d", ret);

  rt::WaitGroup w(1);
  w.Wait();
}


int main(int argc, char *argv[]) {

 //  leveldb_init();

  if (argc != 3) {
    std::cerr << "usage: [cfg_file] [portno]" << std::endl;
    return -EINVAL;
  }

  listen_addr.port = atoi(argv[2]);

  int ret = runtime_init(argv[1], MainHandler, NULL);
  if (ret) {
    std::cerr << "failed to start runtime" << std::endl;
    return ret;
  }
  // MainHandler(NULL);
  return 0;
}