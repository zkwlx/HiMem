#include <jni.h>
#include <string>
#include <sys/mman.h>
#include <cerrno>
#include "log.h"

#include <unistd.h>
#include <malloc.h>
#include <map>

#include "mem_hook.h"
#include "mem_native.h"
#include "fb_unwinder/runtime.h"
#include "mmap_tracer.h"
#include "mem_stack.h"

extern "C" {
#include "log.h"
}

#define SIZE_THRESHOLD 1024
//#define SIZE_THRESHOLD 1040384

JavaVM *g_vm = nullptr;
jobject g_obj = nullptr;

void setEnv(JNIEnv *env, jobject obj) {
    // 保存全局 vm 引用，以便在子线程中使用
    env->GetJavaVM(&g_vm);
    // 保存全局的 java jni 对象引用
    g_obj = env->NewGlobalRef(obj);
}

void clearEnv(JNIEnv *env) {
    env->DeleteGlobalRef(g_obj);
}

void setDebug(JNIEnv *env, jobject thiz, jint enable) {
    set_hook_debug(enable);
}

void init(JNIEnv *env, jobject thiz, jstring dumpDir) {
    // set global env
    setEnv(env, thiz);
    char *dumpDirChar = const_cast<char *>(env->GetStringUTFChars(dumpDir, JNI_FALSE));
    tracerStart(dumpDirChar);
    env->ReleaseStringUTFChars(dumpDir, dumpDirChar);
    do_hook();
}

void deInit(JNIEnv *env, jobject thiz) {
    clear_hook();
    tracerDestroy();
    clearEnv(env);
}

void memDump(JNIEnv *env, jobject thiz) {
    dumpToFile();
}

void onMmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    if (length < SIZE_THRESHOLD) {
        // 小内存分配，忽略
        return;
    }
    JNIEnv *env;
    if (g_vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK || env == nullptr) {
        // 非 JVM 触发的分配，警告
        LOGE("MMAP GetEnv Fail!!!>> addr: %p, length: %u", addr, length);
    }
    std::string stack;
    obtainStack(stack);

    mmap_info info{
            .address = reinterpret_cast<uintptr_t>(addr),
            .length = length,
            .prot = prot,
            .flag = flags,
            .fd = fd,
            .offset = offset,
            .stack = stack,
    };
    postOnMmap(&info);
}

void callOnMmap64(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    LOGI("mmap64 ====> addr:%p, addr_long:%ld, length:%d, prot:%d, flags:%d, fd:%d, offset:%lld",
         addr, addr, length, prot, flags, fd, offset);
    onMmap(addr, length, prot, flags, fd, offset);
}

void callOnMmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    LOGI("mmap ====> addr:%p, addr_long:%ld, length:%d, prot:%d, flags:%d, fd:%d, offset:%lld",
         addr, addr, length, prot, flags, fd, offset);
    onMmap(addr, length, prot, flags, fd, offset);
}

void callOnMunmap(void *addr, size_t length) {
    LOGI("munmmap ----> addr:%p, length:%d", addr, length);
    if (length < SIZE_THRESHOLD) {
        return;
    }
    JNIEnv *env;
    if (g_vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK || env == nullptr) {
        LOGE("MUNMAP GetEnv Fail!!!>> addr: %p, length: %ld", addr, length);
    }

    munmap_info info{
            .address = reinterpret_cast<uintptr_t>(addr),
            .length = length,
    };
    postOnMunmap(&info);
}

void callJava(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    JNIEnv *env;
    jmethodID mid;
    jclass clazz;
    if (g_vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return;
    }
    // find class
    clazz = env->GetObjectClass(g_obj);
    if (clazz == nullptr) {
        return;
    }
    mid = env->GetMethodID(clazz, "onMmap", "(JJIIIJ)V");
    if (mid != nullptr) {
        env->CallVoidMethod(g_obj, mid, addr, length, prot, flags, fd, offset);
    }
}

static JNINativeMethod methods[] = {
        {"setDebug", "(I)V",                  (void *) setDebug},
        {"init",     "(Ljava/lang/String;)V", (void *) init},
        {"deInit",   "()V",                   (void *) deInit},
        {"memDump",  "()V",                   (void *) memDump},
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    jclass clazz = env->FindClass("com/himem/HiMemNative");
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0]));
    return JNI_VERSION_1_6;
}
