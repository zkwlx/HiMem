//
// Created by creat on 2020/3/24 0024.
//

#include "mem_hook.h"
#include <unistd.h>
#include <sys/mman.h>
#include "mem_native.h"

extern "C" {
#include "log.h"
#include "xhook/xhook.h"
}

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
    int result = munmap(addr, length);
    callOnMunmap(addr, length);
    return result;
}

void do_hook() {
    //追踪某些调用 (忽略 linker 和 linker64)
//    xhook_register("^/system/.*$", "mmap", my_mmap, NULL);
//    xhook_register("^/vendor/.*$", "mmap", my_mmap, NULL);
//    xhook_register("^/sys/kernel", "mmap", my_mmap, NULL);
//    xhook_register("^/vendor/.*$", "munmap", my_munmap, NULL);
//    xhook_register(".*", "mmap", (void *) my_mmap, NULL);
    xhook_register(".*", "mmap64", (void *) my_mmap64, NULL);
    xhook_register(".*", "munmap", (void *) my_munmap, NULL);
//    xhook_register(".*/libart.so$", "mmap", my_mmap, NULL);
//    xhook_register(".*/libc.so$", "mmap", my_mmap, NULL);
//    xhook_register(".*/libart.so$", "mmap64", my_mmap64, NULL);
//    xhook_register(".*/libc.so$", "mmap64", my_mmap64, NULL);
//    xhook_register(".*/libart.so$", "munmap", my_munmap, NULL);
//    xhook_register(".*/libc.so$", "munmap", my_munmap, NULL);
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
    xhook_ignore(".*/libhimem-native.so$", NULL);
    xhook_ignore(".*/liblog.so$", NULL);
    // 启动 hook 机制
    xhook_enable_debug(1);
    xhook_enable_sigsegv_protection(1);
    xhook_refresh(0);
}

void clear_hook() {
    xhook_clear();
    xhook_refresh(0);
}

