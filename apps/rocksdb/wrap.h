#pragma once

#include <stdlib.h>

void* __wrap_malloc(size_t size);
void __wrap_free(void * ptr);
void* __wrap_calloc(size_t nmemb, size_t size);
void* __wrap_realloc(void *ptr, size_t size);
int __wrap_posix_memalign(void **ptr, size_t alignment, size_t size);
void* __wrap_aligned_alloc(size_t alignment, size_t size);

void* __wrap_memcpy(void* dest, const void *src, size_t num);
int __wrap_memcmp(const void * ptr1, const void * ptr2, size_t num);

int __wrap_strcmp(const char* str1, const char* str2);
