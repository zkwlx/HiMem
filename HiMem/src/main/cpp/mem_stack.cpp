//
// Created by zkw on 20-11-18.
//

#include <string>
#include <csignal>
#include "mem_stack.h"
#include "mem_hook.h"

extern "C" {
#include "log.h"
#include "./xunwind/xunwind.h"
}

#define MAX_STACK_DEPTH 512

/**
 * SIGSEGV 信号的跳转恢复点。需要考虑多线程情况
 */
__thread jmp_buf jumpEnv;
/**
 * 设置好 SIGSEGV 跳转点时置位 true。需要考虑多线程情况
 */
__thread bool canJump = false;

//=============================== native stack ====================================
#include <unwind.h>
#include <dlfcn.h>
#include <cinttypes>


struct backtrace_state_t {
    void **current;
    void **end;
};

// 栈起始的偏移量
#define STACK_OFFSET 4
// 栈从偏移处开始的最大行数
#define STACK_SIZE 15

static __thread int lineCount;

static _Unwind_Reason_Code unwind_callback(struct _Unwind_Context *context, void *arg) {
    lineCount++;
    if (lineCount > (STACK_OFFSET + STACK_SIZE)) {
        // 暂时只保留 5 行栈信息
        return _URC_NORMAL_STOP;
    }
    if (lineCount > STACK_OFFSET) {
        // 从栈的第 STACK_OFFSET+1 行开始解析，跳过前 STACK_OFFSET 行（因为是 himem 内部调用栈）
        auto *state = (backtrace_state_t *) arg;
        _Unwind_Word pc = _Unwind_GetIP(context);
        if (pc) {
            if (state->current == state->end) {
                return _URC_END_OF_STACK;
            } else {
                *state->current++ = (void *) pc;
            }
        }
    }
    return _URC_NO_REASON;
}

static size_t capture_backtrace(void **buffer, size_t max) {
    lineCount = 0;
    backtrace_state_t state = {buffer, buffer + max};
    _Unwind_Backtrace(unwind_callback, &state);
    return state.current - buffer;
}

static uintptr_t g_frames[128];
static size_t g_frames_sz = 0;

void obtainNativeStack(std::string &stack) {

    if (MODE == 0) {
        //TODO CFI
        stack = xunwind_cfi_get(XUNWIND_CURRENT_PROCESS, XUNWIND_CURRENT_THREAD, nullptr,
                                nullptr);
    } else if (MODE == 1) {
        //TODO EH
        size_t eh_frames_size = xunwind_eh_unwind(g_frames, sizeof(g_frames) / sizeof(g_frames[0]),
                                                  nullptr);
        __atomic_store_n(&g_frames_sz, eh_frames_size, __ATOMIC_SEQ_CST);
        stack = xunwind_frames_get(g_frames, g_frames_sz, nullptr);
    } else if (MODE == 2) {
        //TODO FP
        size_t fp_frames_size = xunwind_fp_unwind(g_frames, sizeof(g_frames) / sizeof(g_frames[0]),
                                                  nullptr);
        __atomic_store_n(&g_frames_sz, fp_frames_size, __ATOMIC_SEQ_CST);
        stack = xunwind_frames_get(g_frames, g_frames_sz, nullptr);
    }



//    //TODO 是否需要注册 sigjmp
//    void *buffer[STACK_SIZE];
//    int frames_size = capture_backtrace(buffer, STACK_SIZE);
//    for (int i = 0; i < frames_size; i++) {
//        Dl_info info;
//        const void *addr = buffer[i];
//        if (dladdr(addr, &info) && info.dli_fname) {
//            stack.append(info.dli_fname).append("(");
//            std::string func_name = info.dli_sname == nullptr ? "None" : info.dli_sname;
//            stack.append(func_name).append(")").append(STACK_ELEMENT_DIV);
//        }
//    }
}