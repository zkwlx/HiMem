//
// Created by zkw on 20-11-18.
//

#include <jni.h>
#include <string>
#include "runtime.h"
#include "unwinder_wrapper.h"
#include "unwinder_android_900.h"

#include <sys/system_properties.h>
#include "../log.h"

std::string get_system_property(const char *key) {
    char prop_value[PROP_VALUE_MAX]{};
    if (__system_property_get(key, prop_value) > 0) {
        return std::string(prop_value);
    }
    return "";
}

static std::string os_version = get_system_property("ro.build.version.release");

auto unwind(unwind_callback_t _unwind_callback, void *_unwind_data) -> bool {
    bool result = false;
    LOGI("os version ============> %s", os_version.c_str());
    if (os_version == "9") {
        result = unwind_900(_unwind_callback, _unwind_data);
    } else if (os_version == "8.1.0") {
    } else if (os_version == "8.0.0") {
    } else if (os_version == "7.1.2") {
    } else if (os_version == "7.1.1") {
    } else if (os_version == "7.1.0") {
    } else if (os_version == "7.0.0") {
    } else if (os_version == "6.0.1") {
    } else if (os_version == "6.0.0") {
    }
    return result;
}

auto get_method_name(uintptr_t method) -> string_t {
    string_t result{};
    if (os_version == "9") {
        result = get_method_name_900(method);
    } else if (os_version == "8.1.0") {
    } else if (os_version == "8.0.0") {
    } else if (os_version == "7.1.2") {
    } else if (os_version == "7.1.1") {
    } else if (os_version == "7.1.0") {
    } else if (os_version == "7.0.0") {
    } else if (os_version == "6.0.1") {
    } else if (os_version == "6.0.0") {
    }
    return result;
}

auto get_declaring_class(uintptr_t method) -> uint32_t {
    uint32_t result = 0;
    if (os_version == "9") {
        result = get_declaring_class_900(method);
    } else if (os_version == "8.1.0") {
    } else if (os_version == "8.0.0") {
    } else if (os_version == "7.1.2") {
    } else if (os_version == "7.1.1") {
    } else if (os_version == "7.1.0") {
    } else if (os_version == "7.0.0") {
    } else if (os_version == "6.0.1") {
    } else if (os_version == "6.0.0") {
    }
    return result;
}

auto get_class_descriptor(uintptr_t cls) -> string_t {
    string_t result{};
    if (os_version == "9") {
        result = get_class_descriptor_900(cls);
    } else if (os_version == "8.1.0") {
    } else if (os_version == "8.0.0") {
    } else if (os_version == "7.1.2") {
    } else if (os_version == "7.1.1") {
    } else if (os_version == "7.1.0") {
    } else if (os_version == "7.0.0") {
    } else if (os_version == "6.0.1") {
    } else if (os_version == "6.0.0") {
    }
    return result;
}

auto get_method_trace_id(uintptr_t method) -> uint64_t {
    uint64_t result = 0;
    if (os_version == "9") {
        result = get_method_trace_id_900(method);
    } else if (os_version == "8.1.0") {
    } else if (os_version == "8.0.0") {
    } else if (os_version == "7.1.2") {
    } else if (os_version == "7.1.1") {
    } else if (os_version == "7.1.0") {
    } else if (os_version == "7.0.0") {
    } else if (os_version == "6.0.1") {
    } else if (os_version == "6.0.0") {
    }
    return result;
}