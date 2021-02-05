//
// Created by zkw on 20-11-16.
//

#ifndef MEMORY_MEM_TRACER_H
#define MEMORY_MEM_TRACER_H

#include <unistd.h>
#include <string>


struct malloc_info {
    uintptr_t address;
    size_t length;
    std::string stack;
};

struct free_info {
    uintptr_t address;
};

struct mmap_info {
    uintptr_t address;
    size_t length;
    int prot;
    int flag;
    int fd;
    std::string fdLink;
    off_t offset;
    std::string stack;
};

struct munmap_info {
    uintptr_t address;
    size_t length;
    std::string stack;
};

extern uint FLUSH_THRESHOLD;

void tracerStart(char *dumpDir);

void tracerDestroy();

void tracerDestroy();

void postOnMmap(mmap_info *data);

void postOnMunmap(munmap_info *data);

void postOnMalloc(malloc_info *data);

void postOnFree(free_info *data);

void flushToFile();

#endif //MEMORY_MEM_TRACER_H
