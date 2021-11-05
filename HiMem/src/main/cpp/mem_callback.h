//
// Created by zkw on 20-12-25.
//

#ifndef MEMORY_MEM_CALLBACK_H
#define MEMORY_MEM_CALLBACK_H


#include <malloc.h>

extern uint MMAP_THRESHOLD;
extern uint ALLOC_THRESHOLD;

extern bool obtainStackOnMunmap;

void callOnMmap64(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

void callOnMmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

void callOnMunmap(void *addr, size_t length);

void callOnMalloc(void *addr, size_t size);

void callOnFree(void *addr);

#endif //MEMORY_MEM_CALLBACK_H
