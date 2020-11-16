//
// Created by zkw on 20-11-13.
//

#include <jni.h>
#include <string>
#include <pthread.h>

#include "../log.h"
#include "runtime.h"

uintptr_t get_art_thread() {
    return reinterpret_cast<uintptr_t>(getThreadInstance());
}

void *getThreadInstance() {
#if ANDROID_VERSION_NUM >= 700
    constexpr int kTlsSlotArtThreadSelf = 7;
    return __get_tls()[kTlsSlotArtThreadSelf];
#endif
}
