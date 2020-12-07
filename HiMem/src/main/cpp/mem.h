//
// Created by zkw on 20-11-11.
//

#ifndef MEMORY_MEM_H
#define MEMORY_MEM_H

#include <malloc.h>

void callOnMmap64(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

void callOnMmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

void callOnMunmap(void *addr, size_t length);

#endif //MEMORY_MEM_H
