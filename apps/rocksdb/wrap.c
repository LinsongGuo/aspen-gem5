#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <pthread.h>
#include <x86intrin.h>
#include "wrap.h"

void* __real_malloc(size_t size);
void __real_free(void * ptr);
void* __real_calloc(size_t nmemb, size_t size);
void* __real_realloc(void *ptr, size_t size);
int __real_pthread_mutex_lock(pthread_mutex_t* mutex);
int __real_pthread_mutex_unlock(pthread_mutex_t* mutex);

void *__wrap_malloc(size_t size) {
    unsigned char uif = _testui();
    if (uif)
        _clui();
  
    // printf("malloc\n");
    void *p = __real_malloc(size);

    if (uif)
        _stui();
    return p;
}

void __wrap_free(void *ptr) {
    unsigned char uif = _testui();
    if (uif)
        _clui();

    // printf("free\n");
    __real_free(ptr);
    
    if (uif)
        _stui();
}

void *__wrap_calloc(size_t nmemb, size_t size) {
    unsigned char uif = _testui();
    if (uif)
        _clui();
        
    // printf("calloc\n");
    void * foo = __real_calloc(nmemb, size);
    
    if (uif)
        _stui();
    return foo;
}

void *__wrap_realloc(void *ptr, size_t size) {
    unsigned char uif = _testui();
    if (uif)
        _clui();

    // printf("realloc\n");
    void * foo = __real_realloc(ptr, size);
    
    if (uif)
        _stui();
    return foo;
}

int __wrap_pthread_mutex_lock(pthread_mutex_t* mutex) {
    unsigned char uif = _testui();
    // if (uif)
        _clui();

    // printf("uif: %d\n", uif);

    int res = __real_pthread_mutex_lock(mutex);

    if (uif)
        _stui();
    return res;
}

int __wrap_pthread_mutex_unlock(pthread_mutex_t* mutex) {
    unsigned char uif = _testui();
    // if (uif)
        _clui();

    // printf("unlock uif: %d\n", uif);

    int res = __real_pthread_mutex_unlock(mutex);

    if (uif)
        _stui();
    return res;
}
