#include <jni.h>
#include <string>
#include <sys/mman.h>
#include <cerrno>
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
    LOGI("!!!!>>>>>> %p", fake);
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
//    char *const space = static_cast<char *>(malloc(size));
    char *const space = static_cast<char *>(mmap(nullptr, size, protect, flags, -1, 0));
    if (space == MAP_FAILED) {
        LOGI("Mapped 空间分配失败： %zu-bytes，errno: %s", size, strerror(errno));
    } else {
//        LOGI("Mapped 分配成功：%zu-bytes, address: %p", size, space);
//        memset(space, 'S', size);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mem_MemTest_signalTest(JNIEnv *env, jobject thiz) {

}