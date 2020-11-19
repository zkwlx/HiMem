//
// Created by zkw on 20-11-18.
//

#include <jni.h>
#include "runtime.h"
#include "unwinder_wrapper.h"

#if defined(__aarch64__)
#include "arm64/unwinder_android_900.h"
#elif defined(__arm__)
#include "arm/unwinder_android_900.h"
#elif defined(__i386__)

#include "x86/unwinder_android_900.h"

#elif defined(__x86_64__)
#include "x86_64/unwinder_android_900.h"
#else
#error unsupported architecture
#endif

auto unwind(unwind_callback_t _unwind_callback, void *_unwind_data) -> bool {
    return unwind_900(_unwind_callback, _unwind_data);
}

auto get_method_name(uintptr_t method) -> string_t {
    string_t result{};
    result = get_method_name_900(method);
    return result;
}

auto get_declaring_class(uintptr_t method) -> uint32_t {
    uint32_t result;
    result = get_declaring_class_900(method);
    return result;
}

auto get_class_descriptor(uintptr_t cls) -> string_t {
    string_t result{};
    result = get_class_descriptor_900(cls);
    return result;
}

auto get_method_trace_id(uintptr_t method) -> uint64_t {
    uint64_t result;
    result = get_method_trace_id_900(method);
    return result;
}