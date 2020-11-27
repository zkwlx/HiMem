//
// Created by zkw on 20-11-16.
//

#ifndef MEMORY_MMAP_TRACER_H
#define MEMORY_MMAP_TRACER_H

#include <unistd.h>
#include <string>

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
};

void tracerStart(char *dumpDir);

void tracerDestroy();

void tracerDestroy();

void postOnMmap(mmap_info *data);

void postOnMunmap(munmap_info *data);

void dumpToFile();

#endif //MEMORY_MMAP_TRACER_H
