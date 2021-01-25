//
// 用于测试 Block 堆栈获取的测试方法
// Created by zkw on 21-1-25.
//

#include <jni.h>
#include <csignal>
#include <csetjmp>
#include <string>


#include "log.h"
#include "mem_stack.h"

using namespace std;

static struct sigaction oldProfAction{};
pthread_t main_thread;

static void profSigHandler(int sig) {
    pthread_t current = pthread_self();
    if (current == main_thread) {
        string stack;
        obtainStack(stack);
        LOGI("stack------> %d", stack.size());
    } else {
        LOGE("thread error!! current: %ld, main: %ld", current, main_thread);
    }
}

static void initProfSigaction() {
    struct sigaction action{};
    action.sa_handler = profSigHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGPROF, &action, &oldProfAction) == -1) {
        LOGE("sigaction SIGPROF failed!");
    }
}

/**
 * 初始化 UI 线程，初始化 SIGPROF 信号处理函数
 * @param env
 * @param thiz
 */
void initForProf(JNIEnv *env, jobject thiz) {
    main_thread = pthread_self();
    initProfSigaction();
}

/**
 * 发送 SIGPROF 信号，获取主线程 Java 栈
 * @param env
 * @param thiz
 */
void sendSigprof(JNIEnv *env, jobject thiz) {
    pthread_kill(main_thread, SIGPROF);
}
