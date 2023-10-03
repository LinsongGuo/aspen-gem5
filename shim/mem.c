#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <pthread.h>
#include <x86intrin.h>

#include "common.h"

#include <runtime/smalloc.h>

void *__real_malloc(size_t size);
void __real_free(void *ptr);
void *__real_calloc(size_t nmemb, size_t size);
void *__real_realloc(void *ptr, size_t size);
int __real_posix_memalign(void **ptr, size_t alignment, size_t size);
void *__real_aligned_alloc(size_t alignment, size_t size);

#ifndef UNSAFE_PREEMPT_SIMDREG
void *__real_memcpy(void *dest, const void *src, size_t n);
int __real_memcmp(const void *s1, const void *s2, size_t n);
void *__real_memmove(void *dest, const void *src, size_t n);
void *__real_memset(void *s, int c, size_t n);
int __real_strcmp(const char *s1, const char *s2);
int __real_strncmp(const char* str1, const char* str2, size_t num);
#endif 

void *__wrap_malloc(size_t size) {
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(shim_active()))
        enter_non_reentrance();
#endif

    // printf("malloc\n");
    void *p = NULL;
    p = __real_malloc(size);
    
    // if (likely(shim_active())) {
    //     p = smalloc(size);
    //     printf("smalloc: %p\n", p);
    // } else {
    //     p = __real_malloc(size);
    //     printf("__real_malloc: %p\n", p);
    // }
    // void *p = __real_malloc(size);

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(shim_active()))
        exit_non_reentrance();
#endif

    return p;
}

void __wrap_free(void *ptr) {
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(shim_active()))
        enter_non_reentrance();
#endif

    __real_free(ptr);
    
    // printf("free: %p\n", ptr);
    // if (likely(shim_active())) {
    //     // printf("sfree: %p\n", ptr);
    //     sfree(ptr);
    // } else {
    //     // printf("__real_free: %p\n", ptr);
    //     __real_free(ptr);
    // }
    // __real_free(ptr);
    
#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(shim_active()))
        exit_non_reentrance();
#endif
}

void *__wrap_calloc(size_t nmemb, size_t size) {
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(shim_active()))
        enter_non_reentrance();
#endif
        
    void *foo = __real_calloc(nmemb, size);
    
#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(shim_active()))
        exit_non_reentrance();
#endif

    return foo;
}

void *__wrap_realloc(void *ptr, size_t size) {
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(shim_active()))
        enter_non_reentrance();
#endif

    void *foo = __real_realloc(ptr, size);
    
#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(shim_active()))
        exit_non_reentrance();
#endif

    return foo;
}

int __wrap_posix_memalign(void **ptr, size_t alignment, size_t size) {
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(shim_active()))
        enter_non_reentrance();
#endif

    int res = __real_posix_memalign(ptr, alignment, size);
    
#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(shim_active()))
        exit_non_reentrance();
#endif

    return res;
}

void* __wrap_aligned_alloc(size_t alignment, size_t size) {
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(shim_active()))
        enter_non_reentrance();
#endif

    void *res = __real_aligned_alloc(alignment, size);
    
#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(shim_active()))
        exit_non_reentrance();
#endif

    return res;
}

#if defined(UNSAFE_PREEMPT_CLUI) || defined(UNSAFE_PREEMPT_FLAG)
void* __wrap_memcpy(void *dest, const void *src, size_t n) {
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG)
    if (likely(shim_active()))
        enter_non_reentrance();
#endif
    
    // printf("__wrap_memcpy\n");
    void *res = __real_memcpy(dest, src, n);
    
#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG)
    if (likely(shim_active()))
        exit_non_reentrance();
#endif

    return res;
}

int __wrap_memcmp(const void *s1, const void *s2, size_t n) {
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG)
    if (likely(shim_active()))
        enter_non_reentrance();
#endif

    // printf("__wrap_memcmp\n");
    int res = __real_memcmp(s1, s2, n);
    
#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG)
    if (likely(shim_active()))
        exit_non_reentrance();
#endif

    return res;
}

void *__wrap_memmove(void *dest, const void *src, size_t n) {
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG)
    if (likely(shim_active()))
        enter_non_reentrance();
#endif

    // printf("__wrap_memmove\n");
    void *res = __real_memmove(dest, src, n);
    
#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG)
    if (likely(shim_active()))
        exit_non_reentrance();
#endif

    return res;
}

void *__wrap_memset(void *s, int c, size_t n) {
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG)
    if (likely(shim_active()))
        enter_non_reentrance();
#endif

    // printf("__wrap_memset\n");
    void *res = __real_memset(s, c, n);

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG)
    if (likely(shim_active()))
        exit_non_reentrance();
#endif

    return res;
}

int __wrap_strcmp(const char* str1, const char* str2) {
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG)
    if (likely(shim_active()))
        enter_non_reentrance();
#endif

    // printf("__wrap_strcmp\n");
    int res = __real_strcmp(str1, str2);
    
#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG)
    if (likely(shim_active()))
        exit_non_reentrance();
#endif

    return res;
}

int __wrap_strncmp(const char* str1, const char* str2, size_t num) {
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG)
    if (likely(shim_active()))
        enter_non_reentrance();
#endif
    
    // printf("__real_strncmp\n");
    int res = __real_strncmp(str1, str2, num);
    
#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG)
    if (likely(shim_active()))
        exit_non_reentrance();
#endif

    return res;
}
#endif