//
// Created by zkw on 20-11-18.
//

#ifndef MEMORY_MEM_STACK_H
#define MEMORY_MEM_STACK_H

#include <csetjmp>

#define STACK_ELEMENT_DIV_CHAR '|'
#define STACK_ELEMENT_DIV "|"

extern __thread jmp_buf jumpEnv;
extern __thread bool canJump;

void initJavaStackDumper();

/**
 * 获取当前线程 Java 调用栈
 * @param stack 调用栈结果
 * @param dump_locks 是否记录 lock monitor 的状态
 */
void obtainJavaStack(std::string &stack, bool dump_locks = false);

void obtainNativeStack(std::string &stack);

#endif //MEMORY_MEM_STACK_H
