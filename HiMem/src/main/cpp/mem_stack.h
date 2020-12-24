//
// Created by zkw on 20-11-18.
//

#ifndef MEMORY_MEM_STACK_H
#define MEMORY_MEM_STACK_H

#include <csetjmp>

#define STACK_ELEMENT_DIV "|"

extern __thread jmp_buf jumpEnv;
extern __thread bool canJump;

struct unwinder_data {
    ucontext_t *ucontext;
    int64_t *frames;
    char const **method_names;
    char const **class_descriptors;
    uint16_t depth;
    uint16_t max_depth;
};

bool obtainStack(std::string &stack);

void obtainNativeStack(std::string &stack);

#endif //MEMORY_MEM_STACK_H
