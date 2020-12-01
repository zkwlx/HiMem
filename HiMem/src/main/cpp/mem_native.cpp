#include <jni.h>
#include <string>
#include <sys/mman.h>
#include <cerrno>
#include <csignal>
#include <csetjmp>
#include "log.h"

#include <set>

#include "mem_hook.h"
#include "mem_native.h"
#include "fb_unwinder/runtime.h"
#include "mmap_tracer.h"
#include "mem_stack.h"

extern "C" {
#include "log.h"
}

using namespace std;

JavaVM *g_vm = nullptr;
jobject g_obj = nullptr;

// 默认 1MB
static uint64_t SIZE_THRESHOLD = 1040384;

// 用于 mmap/munmap 去重
thread_local set<uintptr_t> addressSet;

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

static struct sigaction oldAction{};

void sigHandler(int sig) {
    if (canJump) {
        siglongjmp(jumpEnv, 1);
    } else {
        LOGE("can't jump, because not invoke sigsetjmp!");
        oldAction.sa_handler(sig);
    }
}

static void initSigaction() {
    struct sigaction action{};
    action.sa_handler = sigHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGSEGV, &action, &oldAction) == -1) {
        LOGE("sigaction SIGSEGV failed!");
    }
}

void init(JNIEnv *env, jobject thiz, jstring dumpDir, jlong mmapSizeThreshold) {
    SIZE_THRESHOLD = mmapSizeThreshold;
    // set global env
    setEnv(env, thiz);
    char *dumpDirChar = const_cast<char *>(env->GetStringUTFChars(dumpDir, JNI_FALSE));
    tracerStart(dumpDirChar);
    env->ReleaseStringUTFChars(dumpDir, dumpDirChar);

    initSigaction();

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

string fdToLink(int fd) {
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

void onMmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
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
    // 尝试获取 JVM 堆栈
    string stack;
    obtainStack(stack);
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
        // 对同一个地址多次 munmap(可能因为 hook 过多导致)，跳过
        return;
    }
    munmap_info info{
            .address = address,
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
        {"setDebug", "(I)V",                   (void *) setDebug},
        {"init",     "(Ljava/lang/String;J)V", (void *) init},
        {"deInit",   "()V",                    (void *) deInit},
        {"memDump",  "()V",                    (void *) memDump},
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
