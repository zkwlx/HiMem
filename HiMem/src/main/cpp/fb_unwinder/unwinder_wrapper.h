//
// Created by zkw on 20-11-18.
//

#ifndef MEMORY_UNWINDER_WRAPPER_H
#define MEMORY_UNWINDER_WRAPPER_H

auto unwind(unwind_callback_t _unwind_callback, void *_unwind_data) -> bool;

auto get_method_name(uintptr_t method) -> string_t;

auto get_declaring_class(uintptr_t method) -> uint32_t;

auto get_class_descriptor(uintptr_t cls) -> string_t;

auto get_method_trace_id(uintptr_t method) -> uint64_t;

#endif //MEMORY_UNWINDER_WRAPPER_H
