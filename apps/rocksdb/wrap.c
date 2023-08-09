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
int __real_posix_memalign(void **ptr, size_t alignment, size_t size);
void* __real_aligned_alloc(size_t alignment, size_t size);

void* __real_memcpy(void* dest, const void *src, size_t num);
int __real_memcmp(const void * ptr1, const void * ptr2, size_t num);

int __real_strcmp(const char* str1, const char* str2);

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

    // printf("free: %p\n", ptr);
    __real_free(ptr);
    
    if (uif)
        _stui();
}

void *__wrap_calloc(size_t nmemb, size_t size) {
    unsigned char uif = _testui();
    if (uif)
        _clui();
        
    // printf("calloc\n");
    void *foo = __real_calloc(nmemb, size);
    
    if (uif)
        _stui();
    return foo;
}

void *__wrap_realloc(void *ptr, size_t size) {
    unsigned char uif = _testui();
    if (uif)
        _clui();

    // printf("realloc\n");
    void *foo = __real_realloc(ptr, size);
    
    if (uif)
        _stui();
    return foo;
}

int __wrap_posix_memalign(void **ptr, size_t alignment, size_t size) {
    unsigned char uif = _testui();
    if (uif)
        _clui();

    // printf("posix_memalign\n");
    int res = __real_posix_memalign(ptr, alignment, size);
    
    if (uif)
        _stui();
    return res;
}

void* __wrap_aligned_alloc(size_t alignment, size_t size) {
    unsigned char uif = _testui();
    if (uif)
        _clui();

    // printf("aligned_alloc\n");
    void* foo = __real_aligned_alloc(alignment, size);
    
    if (uif)
        _stui();
    return foo;
}

void* __wrap_memcpy(void* dest, const void *src, size_t num) {
    unsigned char uif = _testui();
    if (uif)
        _clui();

    // printf("memcpy\n");
    __real_memcpy(dest, src, num);
    
    if (uif)
        _stui();
}

int __wrap_memcmp(const void * ptr1, const void * ptr2, size_t num) {
    unsigned char uif = _testui();
    if (uif)
        _clui();

    // printf("__real_memcmp\n");
    int res = __real_memcmp(ptr1, ptr2, num);
    
    if (uif)
        _stui();
    return res;
}

int __wrap_strcmp(const char* str1, const char* str2) {
     unsigned char uif = _testui();
    if (uif)
        _clui();

    int res = __real_strcmp(str1, str2);
    
    if (uif)
        _stui();
    return res;
}
