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

    if (sigsetjmp(jumpEnv, 1) == 0) {
        canJump = true;
        if (unwind(&unwind_cb, &data)) {
            for (int i = 0; i < data.depth; i++) {
                std::string desc = data.class_descriptors[i];
                stack.append(desc.substr(1, desc.size() - 2))
                        .append(".").append(data.method_names[i]).append(STACK_ELEMENT_DIV);
            }
            LOGI("unwind success!!!:%d", stack.length());
            return true;
        } else {
            LOGI("unwind failed!!!!");
            return false;
        }
    } else {// jump
        LOGI("SIGSEGV!! skip ================!!");
        stack.append("stack unwind error").append(STACK_ELEMENT_DIV);
        return false;
    }


}