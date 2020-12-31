//
// Created by zkw on 20-11-18.
//

#include <string>
#include <csignal>
#include "fb_unwinder/runtime.h"
#include "fb_unwinder/unwinder_wrapper.h"
#include "mem_stack.h"

extern "C" {
#include "log.h"
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

bool unwind_cb(uintptr_t frame, void *data) {
    auto *ud = reinterpret_cast<unwinder_data *>(data);
    if (ud->depth >= ud->max_depth) {
        // stack overflow, stop the traversal
        return false;
    }
    ud->frames[ud->depth] = get_method_trace_id(frame);
    //TODO 这里是否可以优化为需要的时候再解析
    if (ud->method_names != nullptr && ud->class_descriptors != nullptr) {
        auto declaring_class = get_declaring_class(frame);
        auto class_string_t = get_class_descriptor(declaring_class);
        string_t method_string_t = get_method_name(frame);
        ud->method_names[ud->depth] = method_string_t.data;
        ud->class_descriptors[ud->depth] = class_string_t.data;
        if (class_string_t.data == nullptr) {
            LOGE("class descriptors data is null!!!!!!!!!!!");
        }
    }
    ++ud->depth;
    return true;
}

bool obtainStack(std::string &stack) {
    int64_t frames[MAX_STACK_DEPTH]; // frame pointer addresses
    char const *method_names[MAX_STACK_DEPTH];
    char const *class_descriptors[MAX_STACK_DEPTH];
    memset(method_names, 0, sizeof(method_names));
    memset(class_descriptors, 0, sizeof(class_descriptors));
    unwinder_data data{
            .ucontext = nullptr,
            .frames = frames,
            .method_names = method_names,
            .class_descriptors = class_descriptors,
            .depth = 0,
            .max_depth = MAX_STACK_DEPTH,
    };
    canJump = true;
    bool result;
    if (sigsetjmp(jumpEnv, 1) == 0) {
        if (unwind(&unwind_cb, &data)) {
            for (int i = 0; i < data.depth; i++) {
                std::string desc = data.class_descriptors[i];
                stack.append(desc.substr(1, desc.size() - 2))
                        .append(".").append(data.method_names[i]).append(STACK_ELEMENT_DIV);
            }
            result = true;
        } else {
            LOGI("unwind failed!!!!");
            result = false;
        }
    } else {// jump
        LOGI("SIGSEGV!! skip obtainStack() !!");
        result = false;
    }
    canJump = false;
    return result;

}

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

void obtainNativeStack(std::string &stack) {
    //TODO 是否需要注册 sigjmp
    void *buffer[STACK_SIZE];
    int frames_size = capture_backtrace(buffer, STACK_SIZE);
    for (int i = 0; i < frames_size; i++) {
        Dl_info info;
        const void *addr = buffer[i];
        if (dladdr(addr, &info) && info.dli_fname) {
            stack.append(info.dli_fname).append("(");
            std::string func_name = info.dli_sname == nullptr ? "None" : info.dli_sname;
            stack.append(func_name).append(")").append(STACK_ELEMENT_DIV);
        }
    }
}