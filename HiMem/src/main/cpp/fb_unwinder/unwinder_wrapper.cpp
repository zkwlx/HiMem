//
// Created by zkw on 20-11-18.
//

#include <jni.h>
#include <string>
#include "runtime.h"
#include "unwinder_wrapper.h"
#include "unwinder_android_900.h"
#include "unwinder_android_810.h"
//#include "unwinder_android_800.h"

#include <sys/system_properties.h>
#include "../log.h"

inline bool is_900(const std::string &version) {
    // 版本号是否以 '9' 开头
    return version.rfind('9', 0) == 0;
}

inline bool is_810(const std::string &version) {
    return version.rfind("8.1", 0) == 0;
}

inline bool is_800(const std::string &version) {
    return version == "8" || version.rfind("8.0", 0) == 0;
}

std::string get_system_property(const char *key) {
    char prop_value[PROP_VALUE_MAX]{};
    if (__system_property_get(key, prop_value) > 0) {
        return std::string(prop_value);
    }
    return "";
}

static std::string os_version = get_system_property("ro.build.version.release");

auto unwind(unwind_callback_t _unwind_callback, void *_unwind_data) -> bool {
    static bool (*unwind_v)(unwind_callback_t, void *) = nullptr;
    bool result = false;

    if (unwind_v == nullptr) {
        if (is_900(os_version)) {
            unwind_v = unwind_900;
        } else if (is_810(os_version)) {
            unwind_v = unwind_810;
        } else if (os_version == "8.0.0") {
        } else if (os_version == "7.1.2") {
        } else if (os_version == "7.1.1") {
        } else if (os_version == "7.1.0") {
        } else if (os_version == "7.0.0") {
        } else if (os_version == "6.0.1") {
        } else if (os_version == "6.0.0") {
        }
    }
    if (unwind_v != nullptr) {
        result = unwind_v(_unwind_callback, _unwind_data);
    }
    return result;
}

auto get_method_name(uintptr_t method) -> string_t {
    static string_t (*get_method_name_v)(uintptr_t) = nullptr;
    string_t result{};
    if (get_method_name_v == nullptr) {
        if (is_900(os_version)) {
            get_method_name_v = get_method_name_900;
        } else if (is_810(os_version)) {
            get_method_name_v = get_method_name_810;
        } else if (os_version == "8.0.0") {
        } else if (os_version == "7.1.2") {
        } else if (os_version == "7.1.1") {
        } else if (os_version == "7.1.0") {
        } else if (os_version == "7.0.0") {
        } else if (os_version == "6.0.1") {
        } else if (os_version == "6.0.0") {
        }
    }
    if (get_method_name_v != nullptr) {
        result = get_method_name_v(method);
    }
    return result;
}

auto get_declaring_class(uintptr_t method) -> uint32_t {
    static uint32_t (*get_declaring_class_v)(uintptr_t) = nullptr;
    uint32_t result = 0;
    if (get_declaring_class_v == nullptr) {
        if (is_900(os_version)) {
            get_declaring_class_v = get_declaring_class_900;
        } else if (is_810(os_version)) {
            get_declaring_class_v = get_declaring_class_810;
        } else if (os_version == "8.0.0") {
        } else if (os_version == "7.1.2") {
        } else if (os_version == "7.1.1") {
        } else if (os_version == "7.1.0") {
        } else if (os_version == "7.0.0") {
        } else if (os_version == "6.0.1") {
        } else if (os_version == "6.0.0") {
        }
    }
    if (get_declaring_class_v != nullptr) {
        result = get_declaring_class_v(method);
    }
    return result;
}

auto get_class_descriptor(uintptr_t cls) -> string_t {
    static string_t (*get_class_descriptor_v)(uintptr_t) = nullptr;
    string_t result{};

    if (get_class_descriptor_v == nullptr) {
        if (is_900(os_version)) {
            get_class_descriptor_v = get_class_descriptor_900;
        } else if (is_810(os_version)) {
            get_class_descriptor_v = get_class_descriptor_810;
        } else if (os_version == "8.0.0") {
        } else if (os_version == "7.1.2") {
        } else if (os_version == "7.1.1") {
        } else if (os_version == "7.1.0") {
        } else if (os_version == "7.0.0") {
        } else if (os_version == "6.0.1") {
        } else if (os_version == "6.0.0") {
        }
    }
    if (get_class_descriptor_v != nullptr) {
        result = get_class_descriptor_v(cls);
    }
    return result;
}

auto get_method_trace_id(uintptr_t method) -> uint64_t {
    static uint64_t (*get_method_trace_id_v)(uintptr_t) = nullptr;
    uint64_t result = 0;

    if (get_method_trace_id_v == nullptr) {
        if (is_900(os_version)) {
            get_method_trace_id_v = get_method_trace_id_900;
        } else if (is_810(os_version)) {
            get_method_trace_id_v = get_method_trace_id_810;
        } else if (os_version == "8.0.0") {
        } else if (os_version == "7.1.2") {
        } else if (os_version == "7.1.1") {
        } else if (os_version == "7.1.0") {
        } else if (os_version == "7.0.0") {
        } else if (os_version == "6.0.1") {
        } else if (os_version == "6.0.0") {
        }
    }
    if (get_method_trace_id_v != nullptr) {
        result = get_method_trace_id_v(method);
    }
    return result;
}

