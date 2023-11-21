#include "concord-rocksdb.h"

inline void DoGet(rocksdb_t *db, rocksdb_readoptions_t *readoptions, char* key, int key_len) { 
  char* err = NULL;
  size_t valuelen = 0;
  char *returned_value =
    rocksdb_get(db, readoptions, key, key_len, &valuelen, &err);
  // assert(!err);
  // assert(returned_value != NULL && strncmp (returned_value, key, valuelen) == 0);
  if (err != NULL || returned_value == NULL || strncmp (returned_value, key, valuelen) != 0) {
    printf("wrong value: %s\n", err);
  }
  free(returned_value);
}


inline void DoScan(rocksdb_t *db, rocksdb_readoptions_t *readoptions) {
  const char *retr_key;
  size_t klen;
  rocksdb_iterator_t *iter = rocksdb_create_iterator(db, readoptions);
  rocksdb_iter_seek_to_first(iter);
  while (rocksdb_iter_valid(iter)) {
    retr_key = rocksdb_iter_key(iter, &klen);
    rocksdb_iter_next(iter);
  } 
  rocksdb_iter_destroy(iter);
}
