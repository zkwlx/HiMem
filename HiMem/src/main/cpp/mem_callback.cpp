//
// Created by zkw on 20-12-25.
//
#include <string>
#include <cerrno>
#include <set>
#include <pthread.h>

#include "mem_callback.h"
#include "mem_stack.h"
#include "mem_tracer.h"
#include "log.h"
#include "collection/MemoryCache.h"

using namespace std;

// 用于 mmap/munmap 去重（出于性能考虑不加锁，而是用线程私有数据，可能有失严谨）
thread_local MemoryCache *mmapAddressCache = new MemoryCache();
// 用于 xalloc/free 去重（出于性能考虑不加锁，而是用线程私有数据，可能有失严谨）
thread_local MemoryCache *allocAddressCache = new MemoryCache();
// 默认 1MB
uint MMAP_THRESHOLD = 1040384;
// malloc 阈值为 64KB
uint ALLOC_THRESHOLD = 64 * 1024;
// 是否在释放内存时获取堆栈，默认 false
bool obtainStackOnMunmap = false;

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

static string obtainStack() {
    // 如果获取不到 java stack，尝试获取 native stack
    string stack;
    obtainJavaStack(stack);
    if (stack.empty()) {
        obtainNativeStack(stack);
    }
    if (stack.empty()) {
        stack.append("stack unwind error").append(STACK_ELEMENT_DIV);
    }
    return stack;
}

static void onMmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    if (length < MMAP_THRESHOLD) {
        // 小内存分配，忽略
        return;
    }
    auto address = reinterpret_cast<uintptr_t>(addr);
    mmapAddressCache->insert(address);
//    auto result = mmapAddressSet.insert(address);
//    if (!result.second) {
    // 对同一个地址多次 mmap(可能因为 hook 过多导致)，跳过
//        return;
//    }
    // 尝试获取 JVM/Native 堆栈
    string stack = obtainStack();

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
    if (length < MMAP_THRESHOLD) {
        return;
    }
    auto address = reinterpret_cast<uintptr_t>(addr);
    bool success = mmapAddressCache->remove(address);
//    int success = mmapAddressSet.erase(address);
    if (!success) {
        // 对同一个地址多次 munmap(可能因为 hook 过多导致)，
        // 不过没必要跳过，因为有跨线程 munmap 的情况，比如 pthread 的创建和退出流程
    }
    string stack;
    if (obtainStackOnMunmap) {
        // 尝试获取 JVM/Native 堆栈
        stack = obtainStack();
    }
    munmap_info info{
            .address = address,
            .length = length,
            .stack = stack,
    };
    postOnMunmap(&info);
}

void callOnMalloc(void *addr, size_t size) {
    if (size < ALLOC_THRESHOLD) {
        // 小内存分配，忽略
        return;
    }
    auto address = reinterpret_cast<uintptr_t>(addr);
    allocAddressCache->insert(address);
    string stack = obtainStack();
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
    auto address = (uintptr_t) addr;
    bool success = allocAddressCache->remove(address);
    if (!success) {
        // 如果 free 的地址没有 alloc 过，忽略
        return;
    }
    free_info info{
            .address = address,
    };
    postOnFree(&info);
}
