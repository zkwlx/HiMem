//
// Created by zkw on 20-12-25.
//
#include <string>
#include <cerrno>
#include <set>

#include "mem_callback.h"
#include "fb_unwinder/runtime.h"
#include "mem_stack.h"
#include "mem_tracer.h"


using namespace std;

// 用于 mmap/munmap 去重（处于性能考虑不加锁，而是用线程私有数据，可能有失严谨，无关紧要）
thread_local set<uintptr_t> addressSet;
// 默认 1MB
uint SIZE_THRESHOLD = 1040384;

static string fdToLink(int fd) {
    if (fd > 0) {
        string fdPath = "/proc/self/fd/";
        fdPath.append(to_string(fd));
        char file_path[1024] = {'\0'};
        if (readlink(fdPath.c_str(), file_path, sizeof(file_path) - 1) == -1) {
            return string("error: ").append(strerror(errno));
        } else {
            return string(file_path);
        }
    } else {
        return "";
    }
}

static void onMmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    if (length < SIZE_THRESHOLD) {
        // 小内存分配，忽略
        return;
    }
    auto address = reinterpret_cast<uintptr_t>(addr);
    auto result = addressSet.insert(address);
    if (!result.second) {
        // 对同一个地址多次 mmap(可能因为 hook 过多导致)，跳过
        return;
    }
    // 尝试获取 JVM/Native 堆栈
    //TODO 考虑是否同时支持 JVM/Native 堆栈
    string stack;
    if (!obtainStack(stack) || stack.empty())
        obtainNativeStack(stack);

    if (stack.empty())
        stack.append("stack unwind error").append(STACK_ELEMENT_DIV);

    // fd 解析为映射的文件 path
    string fdLink = fdToLink(fd);

    mmap_info info{
            .address = address,
            .length = length,
            .prot = prot,
            .flag = flags,
            .fd = fd,
            .fdLink = fdLink,
            .offset = offset,
            .stack = stack,
    };
    postOnMmap(&info);
}

void callOnMmap64(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    onMmap(addr, length, prot, flags, fd, offset);
}

void callOnMmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    onMmap(addr, length, prot, flags, fd, offset);
}

void callOnMunmap(void *addr, size_t length) {
    if (length < SIZE_THRESHOLD) {
        return;
    }
    auto address = reinterpret_cast<uintptr_t>(addr);
    int success = addressSet.erase(address);
    if (!success) {
        // 对同一个地址多次 munmap(可能因为 hook 过多导致)，
        // 不过没必要跳过，因为有跨线程 munmap 的情况，比如 pthread 的创建和退出流程
    }
    munmap_info info{
            .address = address,
            .length = length,
    };
    postOnMunmap(&info);
}

void callOnMalloc(void *addr, size_t size) {
    if (size < SIZE_THRESHOLD) {
        // 小内存分配，忽略
        return;
    }
    auto address = reinterpret_cast<uintptr_t>(addr);
    auto result = addressSet.insert(address);
    if (!result.second) {
        // 对同一个地址多次 malloc (可能因为 hook 过多导致)，跳过
        return;
    }
    // 尝试获取 Native 堆栈。alloc 系列监控暂不获取 JVM 栈
    //TODO 当 JVM/Native 栈可同时支持时再打开 JVM 栈
    string stack;
    obtainNativeStack(stack);
    if (stack.empty())
        stack.append("stack unwind error").append(STACK_ELEMENT_DIV);

    malloc_info info{
            .address = address,
            .length = size,
            .stack = stack,
    };
    postOnMalloc(&info);
}

void callOnFree(void *addr) {
    auto address = reinterpret_cast<uintptr_t>(addr);
    int success = addressSet.erase(address);
    if (!success) {
        // 暂时 malloc/free 一对一
        return;
    }
    free_info info{
            .address = address,
    };
    postOnFree(&info);
}
