//
// Created by zkw on 20-12-4.
//

#ifndef MEMORY_XHOOK_ASSIST_H
#define MEMORY_XHOOK_ASSIST_H
#ifdef __cplusplus
extern "C" {
#endif

#define TRUE 1
#define FALSE 0

/**
 * 遍历 so 的回调，每个 info 包含这个 so 的路径、基地址等信息
 * @param info  包含这个 so 的路径、基地址等信息
 * @param size info 结构体的大小
 * @param data 自定义参数，当前代表是否进行 hook 操作（0 代表否，1 代表是）
 * @return
 */
int dl_callback(struct dl_phdr_info *info, size_t size, void *data);

/**
 * 将地址添加到 hooked 的集合中，避免重复 hook（调用的地方加了锁，所以多线程安全）
 * @param addr
 */
void add_to_hooked_set(uintptr_t addr);

/**
 * 设置 so 遍历的 callback 是否 hook（调用的地方加了锁，所以多线程安全）
 * @param hook
 */
void set_callback_doHook(int hook);

#ifdef __cplusplus
}
#endif

#endif //MEMORY_XHOOK_ASSIST_H
