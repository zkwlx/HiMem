//
// Created by creat on 2020/3/24 0024.
//

#include <string>
#include <unistd.h>
#include <sys/mman.h>
#include <asm/unistd.h>
#include <pthread.h>
#include <bitset>
#include "mem_hook.h"
#include "mem_callback.h"
#include "mem_stack.h"
#include "fb_unwinder/runtime.h"
#include "utils/common_utils.h"

extern "C" {
#include "log.h"
#include "bhook/bytehook.h"
}

#if defined(__arm__)
#define PTHREAD_ATTR_OFFSET 16U
#elif defined(__aarch64__)
#define PTHREAD_ATTR_OFFSET 24U
#elif defined(__i386__)
#define PTHREAD_ATTR_OFFSET 16U
#elif defined(__x86_64__)
#define PTHREAD_ATTR_OFFSET 24U
#endif

#define MMAP_MODE 1
#define ALLOC_MODE 2

using namespace std;

int MODE = MMAP_MODE;

void *my_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    // 执行 stack 清理（不可省略），只需调用一次
    BYTEHOOK_STACK_SCOPE();

//    void *result = mmap(addr, length, prot, flags, fd, offset);
    void *result = BYTEHOOK_CALL_PREV(my_mmap, addr, length, prot, flags, fd, offset);
    callOnMmap(result, length, prot, flags, fd, offset);
    return result;
}

void *my_mmap64(void *addr, size_t length, int prot, int flags, int fd, off64_t offset) {
    // 执行 stack 清理（不可省略），只需调用一次
    BYTEHOOK_STACK_SCOPE();

//    void *result = mmap64(addr, length, prot, flags, fd, offset);
    void *result = BYTEHOOK_CALL_PREV(my_mmap64, addr, length, prot, flags, fd, offset);
    callOnMmap64(result, length, prot, flags, fd, offset);
    return result;
}

int my_munmap(void *addr, size_t length) {
    // 执行 stack 清理（不可省略），只需调用一次
    BYTEHOOK_STACK_SCOPE();

    callOnMunmap(addr, length);
//    int result = munmap(addr, length);
    int result = BYTEHOOK_CALL_PREV(my_munmap, addr, length);
    return result;
}

static pthread_attr_t *getAttrFromInternal() {
    // Android 9.0/8.1/8.0/7.1.2/7.1.1/7.0 结构一致
    //TODO 添加其他版本支持时需要适配，模拟的结构参考 class_layout.h 文件
    void *pthread_internal = __get_tls()[1];
    auto sp = (uintptr_t) pthread_internal;
    auto attr = sp + PTHREAD_ATTR_OFFSET;
    auto *attrP = (pthread_attr_t *) attr;
    return attrP;
}

void my_pthread_exit(void *return_value) {
    // 执行 stack 清理（不可省略），只需调用一次
    BYTEHOOK_STACK_SCOPE();

    pthread_attr_t *attr = getAttrFromInternal();
    if (attr->stack_size != 0) {
        callOnMunmap(attr->stack_base, attr->stack_size);
    }
    BYTEHOOK_CALL_PREV(my_pthread_exit, return_value);
//    pthread_exit(return_value);
}

void *my_malloc(size_t size) {
    // 执行 stack 清理（不可省略），只需调用一次
    BYTEHOOK_STACK_SCOPE();

//    void *result = malloc(size);
    void *result = BYTEHOOK_CALL_PREV(my_malloc, size);
    callOnMalloc(result, size);
    return result;
}

void *my_calloc(size_t count, size_t size) {
    // 执行 stack 清理（不可省略），只需调用一次
    BYTEHOOK_STACK_SCOPE();
//    void *result = calloc(count, size);
    void *result = BYTEHOOK_CALL_PREV(my_calloc, count, size);

    callOnMalloc(result, count * size);
    return result;
}

void *my_realloc(void *ptr, size_t size) {
    // 执行 stack 清理（不可省略），只需调用一次
    BYTEHOOK_STACK_SCOPE();

//    void *result = realloc(ptr, size);
    void *result = BYTEHOOK_CALL_PREV(my_realloc, ptr, size);
    callOnMalloc(result, size);
    return result;
}

void my_free(void *ptr) {
    // 执行 stack 清理（不可省略），只需调用一次
    BYTEHOOK_STACK_SCOPE();
    if (ptr) {
        callOnFree(ptr);
    }
//    free(ptr);
    BYTEHOOK_CALL_PREV(my_free, ptr);
}

static bytehook_stub_t pthread_exit_hook_stub = nullptr;
static bytehook_stub_t mmap_hook_stub = nullptr;
static bytehook_stub_t mmap64_hook_stub = nullptr;
static bytehook_stub_t munmap_hook_stub = nullptr;

static bool allow_filter(const char *caller_path_name, void *arg) {
    (void) arg;

    if (nullptr == strstr(caller_path_name, "liblog.so") &&
        nullptr == strstr(caller_path_name, "libc.so") &&
        nullptr == strstr(caller_path_name, "libhimem-native.so") &&
        nullptr == strstr(caller_path_name, "libbacktrace_libc++.so") &&
        nullptr == strstr(caller_path_name, "libbacktrace.so") &&
        nullptr == strstr(caller_path_name, "libcorkscrew.so") &&
        nullptr == strstr(caller_path_name, "libc++.so")) {
        return true;
    } else
        return false;
//    string packageName = getPackageName();
//    if (nullptr != strstr(caller_path_name, packageName.c_str()) &&
//        nullptr == strstr(caller_path_name, "libhimem-native.so")) {
//        LOGI("hooked so ========> %s", caller_path_name);
//        return true;
//    }else
//        return false;
}

static void hook_for_mmap() {
    bytehook_hook_single("libc.so", nullptr, "pthread_exit", (void *) my_pthread_exit, nullptr,
                         nullptr);
    bytehook_hook_partial(allow_filter, nullptr, nullptr, "mmap", (void *) my_mmap, nullptr,
                          nullptr);
    bytehook_hook_partial(allow_filter, nullptr, nullptr, "mmap64", (void *) my_mmap64,
                          nullptr, nullptr);
    bytehook_hook_partial(allow_filter, nullptr, nullptr, "munmap", (void *) my_munmap,
                          nullptr, nullptr);
}

static bytehook_stub_t malloc_hook_stub = nullptr;
static bytehook_stub_t calloc_hook_stub = nullptr;
static bytehook_stub_t realloc_hook_stub = nullptr;
static bytehook_stub_t free_hook_stub = nullptr;

static void hook_for_alloc() {
    bytehook_hook_partial(allow_filter, nullptr, nullptr, "malloc", (void *) my_malloc,
                          nullptr, nullptr);
    bytehook_hook_partial(allow_filter, nullptr, nullptr, "calloc", (void *) my_calloc,
                          nullptr, nullptr);
    bytehook_hook_partial(allow_filter, nullptr, nullptr, "realloc", (void *) my_realloc,
                          nullptr, nullptr);
    bytehook_hook_partial(allow_filter, nullptr, nullptr, "free", (void *) my_free,
                          nullptr, nullptr);
}

void do_hook() {
    bytehook_init(BYTEHOOK_MODE_AUTOMATIC, true);
    hook_for_mmap();
    hook_for_alloc();
//    if (MODE == MMAP_MODE) {
//        hook_for_mmap();
//        hook_for_alloc();
//    } else if (MODE == ALLOC_MODE) {
//        hook_for_alloc();
//    }

    // old_func 用法
//    void (*orig_printf)(const char*);
//    xhook_register(".so library", "printf", my_printf, (void*)&orig_printf);

    // 不要 hook 我们自己
//    xhook_ignore(".*/libhimem-native.so$", nullptr);
//    xhook_ignore(".*/liblog.so$", nullptr);
}

void clear_hook() {
    if (calloc_hook_stub != nullptr) {
        bytehook_unhook(calloc_hook_stub);
        calloc_hook_stub = nullptr;
    }
    if (free_hook_stub != nullptr) {
        bytehook_unhook(free_hook_stub);
        free_hook_stub = nullptr;
    }
    if (malloc_hook_stub != nullptr) {
        bytehook_unhook(malloc_hook_stub);
        malloc_hook_stub = nullptr;
    }
    if (realloc_hook_stub != nullptr) {
        bytehook_unhook(realloc_hook_stub);
        realloc_hook_stub = nullptr;
    }
    if (mmap64_hook_stub != nullptr) {
        bytehook_unhook(mmap64_hook_stub);
        mmap64_hook_stub = nullptr;
    }
    if (mmap_hook_stub != nullptr) {
        bytehook_unhook(mmap_hook_stub);
        mmap_hook_stub = nullptr;
    }
    if (munmap_hook_stub != nullptr) {
        bytehook_unhook(munmap_hook_stub);
        munmap_hook_stub = nullptr;
    }
    if (pthread_exit_hook_stub != nullptr) {
        bytehook_unhook(pthread_exit_hook_stub);
        pthread_exit_hook_stub = nullptr;
    }
}

