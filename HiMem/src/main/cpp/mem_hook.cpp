//
// Created by creat on 2020/3/24 0024.
//

#include <string>
#include "mem_hook.h"
#include "mem_callback.h"
#include "runtime.h"
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

#define MMAP_MODE 0x1
#define ALLOC_MODE 0x2
#define MMAP_AND_ALLOC_MODE 0x4

using namespace std;

int MODE = MMAP_MODE;

void *my_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    // 执行 stack 清理（不可省略），只需调用一次
    BYTEHOOK_STACK_SCOPE();

    void *result = BYTEHOOK_CALL_PREV(my_mmap, addr, length, prot, flags, fd, offset);
    callOnMmap(result, length, prot, flags, fd, offset);
    return result;
}

void *my_mmap64(void *addr, size_t length, int prot, int flags, int fd, off64_t offset) {
    // 执行 stack 清理（不可省略），只需调用一次
    BYTEHOOK_STACK_SCOPE();

    void *result = BYTEHOOK_CALL_PREV(my_mmap64, addr, length, prot, flags, fd, offset);
    callOnMmap64(result, length, prot, flags, fd, offset);
    return result;
}

int my_munmap(void *addr, size_t length) {
    // 执行 stack 清理（不可省略），只需调用一次
    BYTEHOOK_STACK_SCOPE();

    callOnMunmap(addr, length);
    int result = BYTEHOOK_CALL_PREV(my_munmap, addr, length);
    return result;
}

static pthread_attr_t *getAttrFromInternal() {
    // Android 12 ~ 7.0 结构一致
    //TODO 添加其他版本支持时需要适配，模拟的结构参考 class_layout.h 文件
    void *pthread_internal = __get_tls()[TLS_SLOT_THREAD_ID];
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
}

void *my_malloc(size_t size) {
    // 执行 stack 清理（不可省略），只需调用一次
    BYTEHOOK_STACK_SCOPE();

    void *result = BYTEHOOK_CALL_PREV(my_malloc, size);
    callOnMalloc(result, size);
    return result;
}

void *my_calloc(size_t count, size_t size) {
    // 执行 stack 清理（不可省略），只需调用一次
    BYTEHOOK_STACK_SCOPE();
    void *result = BYTEHOOK_CALL_PREV(my_calloc, count, size);
    callOnMalloc(result, count * size);
    return result;
}

void *my_realloc(void *ptr, size_t size) {
    // 执行 stack 清理（不可省略），只需调用一次
    BYTEHOOK_STACK_SCOPE();
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
    BYTEHOOK_CALL_PREV(my_free, ptr);
}


/**
 * hook 的过滤函数，返回 true 则需要 hook，false 则跳过
 * @param caller_path_name
 * @param arg
 * @return
 **/
static bool allow_filter(const char *caller_path_name, void *arg) {
    (void) arg;

    if (nullptr == strstr(caller_path_name, "liblog.so") &&
        // hook libc.so 会导致递归调用最终 ANR
        nullptr == strstr(caller_path_name, "libc.so") &&
        // hook libhimem-native.so 会导致递归调用最终 ANR
        nullptr == strstr(caller_path_name, "libhimem-native.so") &&
        nullptr == strstr(caller_path_name, "libc++.so") &&
        // libGLES_mali.so 的内部会申请大量内存，但调用栈对分析问题没有帮助
        nullptr == strstr(caller_path_name, "libGLES_mali.so")
            ) {
        return true;
    } else
        return false;
}

static bytehook_stub_t pthread_exit_hook_stub = nullptr;

static void hook_for_pthread_exit() {
    pthread_exit_hook_stub = bytehook_hook_single("libc.so", nullptr, "pthread_exit",
                                                  (void *) my_pthread_exit, nullptr, nullptr);
}

static bytehook_stub_t mmap_hook_stub = nullptr;
static bytehook_stub_t mmap64_hook_stub = nullptr;
static bytehook_stub_t munmap_hook_stub = nullptr;

static void hook_for_mmap() {
    mmap_hook_stub = bytehook_hook_partial(allow_filter, nullptr, nullptr, "mmap", (void *) my_mmap,
                                           nullptr, nullptr);
    mmap64_hook_stub = bytehook_hook_partial(allow_filter, nullptr, nullptr, "mmap64",
                                             (void *) my_mmap64, nullptr, nullptr);
    munmap_hook_stub = bytehook_hook_partial(allow_filter, nullptr, nullptr, "munmap",
                                             (void *) my_munmap, nullptr, nullptr);
}

static bytehook_stub_t malloc_hook_stub = nullptr;
static bytehook_stub_t calloc_hook_stub = nullptr;
static bytehook_stub_t realloc_hook_stub = nullptr;
static bytehook_stub_t free_hook_stub = nullptr;

static void hook_for_alloc() {
    malloc_hook_stub = bytehook_hook_partial(allow_filter, nullptr, nullptr, "malloc",
                                             (void *) my_malloc, nullptr, nullptr);
    calloc_hook_stub = bytehook_hook_partial(allow_filter, nullptr, nullptr, "calloc",
                                             (void *) my_calloc, nullptr, nullptr);
    realloc_hook_stub = bytehook_hook_partial(allow_filter, nullptr, nullptr, "realloc",
                                              (void *) my_realloc, nullptr, nullptr);
    //TODO 去掉 free 的 hook，因为 free 调用过于频繁，会带来稳定性问题
    free_hook_stub = bytehook_hook_partial(allow_filter, nullptr, nullptr, "free", (void *) my_free,
                                           nullptr, nullptr);
}

void do_hook() {
    bytehook_init(BYTEHOOK_MODE_AUTOMATIC, true);
    if (MODE & MMAP_MODE || MODE & MMAP_AND_ALLOC_MODE) {
        hook_for_pthread_exit();
        hook_for_mmap();
    }
    if (MODE & ALLOC_MODE || MODE & MMAP_AND_ALLOC_MODE) {
        hook_for_alloc();
    }
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

