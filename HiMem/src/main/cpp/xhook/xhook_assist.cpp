//
// Created by zkw on 20-12-4.
//

#include "xh_core.h"
#include "xhook_assist.h"
#include <string>
#include <link.h>
#include <cinttypes>
#include <set>
#include "../log.h"

static std::set<uintptr_t> hookedPtr;
static int doHook = FALSE;

extern "C"
int dl_callback(struct dl_phdr_info *info, size_t size, void *data) {
    // 通过 doHook 控制是否真正 hook，如果不 hook，只进行集合的插入操作，这样设计的原因请参考调用方的注释
    auto result = hookedPtr.insert(info->dlpi_addr);
    if (!result.second || doHook == FALSE) {
        // 如果重复 hook 或者指定不进行 hook，跳过
        return 0;
    }
    LOGI("dl hook name=%s address=%" PRIxPTR ", (%d segments)\n", info->dlpi_name,
         info->dlpi_addr, info->dlpi_phnum);
    xh_core_hook_for_iterate(info->dlpi_name, info->dlpi_addr);
    return 0;
}

extern "C"
void add_to_hooked_set(uintptr_t addr) {
    hookedPtr.insert(addr);
}

extern "C"
void set_callback_doHook(int hook) {
    doHook = hook;
}



