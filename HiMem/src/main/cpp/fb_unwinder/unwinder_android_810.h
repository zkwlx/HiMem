#ifndef MEMORY_UNWINDER_ANDROID_810_H
#define MEMORY_UNWINDER_ANDROID_810_H

auto unwind_810(unwind_callback_t _unwind_callback, void *_unwind_data) -> bool;

auto get_method_name_810(uintptr_t method) -> string_t;

auto get_declaring_class_810(uintptr_t method) -> uint32_t;

auto get_class_descriptor_810(uintptr_t cls) -> string_t;

auto get_method_trace_id_810(uintptr_t method) -> uint64_t;

#endif //MEMORY_UNWINDER_ANDROID_810_H