//
// Created by zkw on 20-11-13.
//

#ifndef MEMORY_UNWINDER_H
#define MEMORY_UNWINDER_H

typedef bool (*unwind_callback_t)(uintptr_t, void *);

struct unwinder_data {
    ucontext_t *ucontext;
    int64_t *frames;
    char const **method_names;
    char const **class_descriptors;
    uint16_t depth;
    uint16_t max_depth;
};

bool unwind(unwind_callback_t _unwind_callback, void *_unwind_data);

string_t get_method_name(uintptr_t method);

uint32_t get_declaring_class(uintptr_t method);

string_t get_class_descriptor(uintptr_t cls);

uint64_t get_method_trace_id(uintptr_t method);

bool stack();

#endif //MEMORY_UNWINDER_H
