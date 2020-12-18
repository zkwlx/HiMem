//
// Created by creat on 2020/3/24 0024.
//

#include <string>
#include <unistd.h>
#include <sys/mman.h>
#include <asm/unistd.h>
#include <pthread.h>
#include "mem_hook.h"
#include "mem.h"
#include "mem_stack.h"
#include "fb_unwinder/runtime.h"

extern "C" {
#include "log.h"
#include "xhook/xhook.h"
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

void set_hook_debug(int enable) {
    xhook_enable_debug(enable);
    // debug 阶段关闭 segv 保护，以便发现问题，release 的时候需要开启，减少 hook 带来的线上崩溃
    xhook_enable_sigsegv_protection(enable ? 0 : 1);
}

void *my_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    void *result = mmap(addr, length, prot, flags, fd, offset);
    callOnMmap(result, length, prot, flags, fd, offset);
    return result;
}

void *my_mmap64(void *addr, size_t length, int prot, int flags, int fd, off64_t offset) {
    void *result = mmap64(addr, length, prot, flags, fd, offset);
    callOnMmap64(result, length, prot, flags, fd, offset);
    return result;
}

int my_munmap(void *addr, size_t length) {
    // Android8.1 上的munmap操作后面不能有其他调用，否则段错误，不知为何
    callOnMunmap(addr, length);
    int result = munmap(addr, length);
    return result;
}

static pthread_attr_t *getAttrFromInternal() {
    // Android 9.0/8.1.0 结构一致
    //TODO 添加其他版本支持时需要适配，模拟的结构参考 class_layout.h 文件
    void *pthread_internal = __get_tls()[1];
    auto sp = (uintptr_t) pthread_internal;
    auto attr = sp + PTHREAD_ATTR_OFFSET;
    auto *attrP = (pthread_attr_t *) attr;
    return attrP;
}

void my_pthread_exit(void *return_value) {
    pthread_attr_t *attr = getAttrFromInternal();
    if (attr->stack_size != 0) {
        callOnMunmap(attr->stack_base, attr->stack_size);
    }
    pthread_exit(return_value);
}

void do_hook() {
    //追踪某些调用 (忽略 linker 和 linker64)
    xhook_register(".*\\.so$", "mmap", (void *) my_mmap, nullptr);
    xhook_register(".*\\.so$", "mmap64", (void *) my_mmap64, nullptr);
    xhook_register(".*\\.so$", "munmap", (void *) my_munmap, nullptr);
    xhook_register(".*/libc.so$", "pthread_exit", (void *) my_pthread_exit, nullptr);
    xhook_ignore(".*/linker$", "mmap");
    xhook_ignore(".*/linker$", "mmap64");
    xhook_ignore(".*/linker$", "munmap");
    xhook_ignore(".*/linker64$", "mmap");
    xhook_ignore(".*/linker64$", "mmap64");
    xhook_ignore(".*/linker64$", "munmap");

    // old_func 用法
//    void (*orig_printf)(const char*);
//    xhook_register(".so library", "printf", my_printf, (void*)&orig_printf);

    // 不要 hook 我们自己
    xhook_ignore(".*/libhimem-native.so$", nullptr);
    xhook_ignore(".*/liblog.so$", nullptr);
    // 启动 hook 机制
    //TODO 关闭debug
    xhook_enable_debug(1);
    xhook_enable_sigsegv_protection(1);
    xhook_refresh(0);
}

int rehook_for_iterate() {
    return xhook_refresh_for_iterate();
}

void clear_hook() {
    xhook_clear();
    xhook_refresh(0);
}

