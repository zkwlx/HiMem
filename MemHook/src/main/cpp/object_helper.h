//
// Created by createchance on 3/25/20.
//

#ifndef OBJECT_HELPER_H
#define OBJECT_HELPER_H

#include <jni.h>
#include <strings.h>

jint readInt(JNIEnv *env, jobject obj, const char *name, const char *sig);

void writeInt(JNIEnv *env, jobject obj, const char *name, const char *sig, __u32 data);

void writeLong(JNIEnv *env, jobject obj, const char *name, const char *sig, __u64 data);

void writeObj(JNIEnv *env, jobject obj, const char *name, const char *sig, jobject data);

#endif //OBJECT_HELPER_H
