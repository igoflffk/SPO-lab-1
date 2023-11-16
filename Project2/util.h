#pragma once

#include    <stdbool.h>

void* myMalloc(size_t size);

#define myAlloc(T) ((T*)myMalloc(sizeof(T)))
#define myAllocArray(T,n) ((T*)myMalloc(sizeof(T)*(n)))
#define myAllocWithArray(T,E,n) ((T*)myMalloc(sizeof(T)+sizeof(E)*(n)))

#define UNUSED(x) ((void)&x)

typedef int (*intFunc2)(void*, void*);
typedef void (*action2)(void*, void*);
typedef void (*action3)(void*, void*, void*);
typedef bool (*boolFunc2)(void*, void*);

char* formatString(const char* fmt, ...);
char* substring(const char* str, int position, int length);
