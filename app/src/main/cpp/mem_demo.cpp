#include <jni.h>
#include <string>
#include <sys/mman.h>
#include <cerrno>
#include <link.h>
#include "logger.h"

void fake() {
    LOGI(">>>>>>>>>>");
}

void *space = nullptr;

extern "C" JNIEXPORT jstring JNICALL
Java_com_mem_MemTest_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    const int64_t size = 80 * 1024 * 1024L;

    if (space == nullptr) {
        const int flags = MAP_PRIVATE | MAP_ANONYMOUS;
        const int protect = PROT_READ | PROT_WRITE;
//        space = static_cast<char *>(mmap(nullptr, size, protect, flags, -1, 0));
        space = malloc(size);
        if (space == MAP_FAILED) {
            LOGI("Mapped 空间分配失败： %zu-bytes，errno: %s", size, strerror(errno));
        } else {
            LOGI("Mapped 分配成功：%zu-bytes, address: %p", size, space);
            memset(space, 'F', 30 * 1024 * 1024);
        }
    } else {
//        munmap(space, size);
        free(space);
        LOGI("Mapped 空间释放： %zu-bytes", size);
        space = nullptr;
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

void log(char *line) {
//    LOGI("%s", line);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mem_MemTest_asprintfTest(JNIEnv *env, jobject thiz) {
    struct timeval stamp{};
    gettimeofday(&stamp, nullptr);
    long long time = stamp.tv_usec;
    for (int i = 0; i < 100000; i++) {
        char *content;
        asprintf(&content, "%s[]%d[]%d[]%s\n", "asprintf", i, i * 10, "testtttttt");
        log(content);
        free(content);
    }
    gettimeofday(&stamp, nullptr);
    time = stamp.tv_usec - time;
    LOGI("===================asprintf: %lld", time);
}

using namespace std;

extern "C"
JNIEXPORT void JNICALL
Java_com_mem_MemTest_stringTest(JNIEnv *env, jobject thiz) {
    struct timeval stamp{};
    gettimeofday(&stamp, nullptr);
    long long time = stamp.tv_usec;
    for (int i = 0; i < 100000; i++) {
        string content =
                "string[]" + to_string(i) + "[]" + to_string(i * 10) + "[]" + "ttesttttttt\n";
        log((char *) content.c_str());
    }
    gettimeofday(&stamp, nullptr);
    time = stamp.tv_usec - time;
    LOGI("===================string: %lld", time);
}