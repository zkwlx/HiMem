//
// Created by zkw on 20-12-3.
//

#ifndef CLASS_LAYOUT_H
#define CLASS_LAYOUT_H

#include <pthread.h>

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
