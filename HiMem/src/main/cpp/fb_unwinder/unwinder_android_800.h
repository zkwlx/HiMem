#ifndef MEMORY_UNWINDER_ANDROID_800_H
#define MEMORY_UNWINDER_ANDROID_800_H

auto unwind_800(unwind_callback_t _unwind_callback, void *_unwind_data) -> bool;

auto get_method_name_800(uintptr_t method) -> string_t;

auto get_declaring_class_800(uintptr_t method) -> uint32_t;

auto get_class_descriptor_800(uintptr_t cls) -> string_t;

auto get_method_trace_id_800(uintptr_t method) -> uint64_t;

#endif //MEMORY_UNWINDER_ANDROID_800_H