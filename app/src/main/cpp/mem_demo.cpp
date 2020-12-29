#include <jni.h>
#include <string>
#include <sys/mman.h>
#include <cerrno>
#include <link.h>
#include "logger.h"

void fake() {
    LOGI(">>>>>>>>>>");
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_mem_MemTest_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    const int flags = MAP_PRIVATE | MAP_ANONYMOUS;
    const int protect = PROT_READ | PROT_WRITE;
    const int64_t size = 1024L;
    for (int64_t i = 0; i <= 60; i++) {
//        char *const space = static_cast<char *>(malloc(size));
        char *const space = static_cast<char *>(mmap(nullptr, size, protect, flags, -1, 0));
        if (space == MAP_FAILED) {
            LOGI("Mapped 空间分配失败： %zu-bytes，errno: %s", size, strerror(errno));
        } else {
            LOGI("Mapped 分配成功：%zu-bytes, address: %p, i:%d", size, space, i);
//            memset(space, 'F', 1024 * 1024);
        }
    }
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mem_MemTest_mmapSmall(JNIEnv *env, jobject thiz) {
    const int flags = MAP_PRIVATE | MAP_ANONYMOUS;
    const int protect = PROT_READ | PROT_WRITE;
    const int64_t size = 16 * 1024 * 1024L;
    fake();
    LOGI("!!!!>>>>>> %p", fake);
//    char *const space = static_cast<char *>(malloc(size));
    void *const space = mmap(nullptr, size, protect, flags, -1, 0);
    if (space == MAP_FAILED) {
        LOGI("Mapped 空间分配失败： %zu-bytes，errno: %s", size, strerror(errno));
    } else {
        LOGI("Mapped 分配成功：%zu-bytes, address: %p", size, space);
//        memset(space, 'S', size);
    }
    void *p = malloc(4 * 1024 * 1024);
    munmap(space, size);
    free(p);
}

static int callback(struct dl_phdr_info *info, size_t size, void *data) {
    int j;

    LOGI("name=%s address=%x, (%d segments)\n", info->dlpi_name, info->dlpi_addr, info->dlpi_phnum);

//        for (j = 0; j < info->dlpi_phnum; j++) {
//            LOGI("\t\t header %2d: address=%10p\n", j,
//                 (void *) (info->dlpi_addr + info->dlpi_phdr[j].p_vaddr));
//        }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mem_MemTest_munmapSmall(JNIEnv *env, jobject thiz) {
    dl_iterate_phdr(callback, nullptr);
}