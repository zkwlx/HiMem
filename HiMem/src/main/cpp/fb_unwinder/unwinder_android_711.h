#ifndef MEMORY_UNWINDER_ANDROID_711_H
#define MEMORY_UNWINDER_ANDROID_711_H

auto unwind_711(unwind_callback_t _unwind_callback, void *_unwind_data) -> bool;

auto get_method_name_711(uintptr_t method) -> string_t;

auto get_declaring_class_711(uintptr_t method) -> uint32_t;

auto get_class_descriptor_711(uintptr_t cls) -> string_t;

auto get_method_trace_id_711(uintptr_t method) -> uint64_t;

#endif //MEMORY_UNWINDER_ANDROID_711_H