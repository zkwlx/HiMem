//
// Created by zkw on 20-12-3.
//

#ifndef CLASS_LAYOUT_H
#define CLASS_LAYOUT_H

#include <pthread.h>

// pthread_internal_t 源码
// https://cs.android.com/android/platform/superproject/+/master:bionic/libc/bionic/pthread_internal.h;drc=master;bpv=0;bpt=1;l=65

class ClassLayout {
public:
    class ClassLayout *next;

    class ClassLayout *prev;

    pid_t tid;
public:
    pid_t cached_pid_;
public:
    pthread_attr_t attr;
};

#endif
