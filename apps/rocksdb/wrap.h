#pragma once

#include <stdlib.h>

void* __wrap_malloc(size_t size);
void __wrap_free(void * ptr);
void* __wrap_calloc(size_t nmemb, size_t size);
void* __wrap_realloc(void *ptr, size_t size);

int __wrap_pthread_mutex_lock(pthread_mutex_t* mutex);
int __wrap_pthread_mutex_unlock(pthread_mutex_t* mutex);