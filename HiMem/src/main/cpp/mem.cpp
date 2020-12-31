#include <jni.h>
#include <csignal>
#include <csetjmp>
#include "log.h"

#include "mem_hook.h"
#include "mem_tracer.h"
#include "mem_stack.h"
#include "mem_callback.h"

using namespace std;

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

static struct sigaction oldAction{};

static void sigHandler(int sig) {
    if (canJump) {
        siglongjmp(jumpEnv, 1);
    } else {
        LOGE("Don't jump, because not obtainStack()!");
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

void
init(JNIEnv *env, jobject thiz, jstring dumpDir, jlong mmapSizeThreshold, jlong flushThreshold, int mode) {
    SIZE_THRESHOLD = mmapSizeThreshold;
    FLUSH_THRESHOLD = flushThreshold;
    MODE = mode;
    // set global env
    setEnv(env, thiz);
    char *dumpDirChar = const_cast<char *>(env->GetStringUTFChars(dumpDir, JNI_FALSE));
    tracerStart(dumpDirChar);
    env->ReleaseStringUTFChars(dumpDir, dumpDirChar);

    initSigaction();

    do_hook();
}

void refreshHookForDl(JNIEnv *env, jobject thiz) {
    rehook_for_iterate();
}

void deInit(JNIEnv *env, jobject thiz) {
    clear_hook();
    tracerDestroy();
    clearEnv(env);
}

/**
 * 只是为了模拟 pthread_internal_t 结构的内存布局，不做实际用途
 */
void forTestClassLayout() {
    ClassLayout layout{};
    size_t next_offset = offsetof(ClassLayout, next);
    size_t prev_offset = offsetof(ClassLayout, prev);
    size_t tid_offset = offsetof(ClassLayout, tid);
    size_t cache_pid_offset = offsetof(ClassLayout, cached_pid_);
    size_t attr_pid_offset = offsetof(ClassLayout, attr);
    // 根据打印出来的 attr 的偏移，进行相加读取
    LOGI("ClassLayout:\nnext:%d, prev:%d, tid:%d, cachePid:%d, attr:%d", next_offset, prev_offset,
         tid_offset, cache_pid_offset, attr_pid_offset);
    layout.tid = 10086;
    void *p = &layout;
    uintptr_t sp = (uintptr_t) p;
    uintptr_t tid = sp + 8U;// 其实就是 tid_offset
    const int *tidP = (int *) tid;
    // 如果tid的值时10086说明读取成功
    LOGI("==================> tid:%d ", *tidP);
}

void memFlush(JNIEnv *env, jobject thiz) {
    flushToFile();
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
        {"setDebug",         "(I)V",                    (void *) setDebug},
        {"init",             "(Ljava/lang/String;JJI)V", (void *) init},
        {"deInit",           "()V",                     (void *) deInit},
        {"memFlush",         "()V",                     (void *) memFlush},
        {"refreshHookForDl", "()V",                     (void *) refreshHookForDl},
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
