//
// Created by createchance on 3/25/20.
//
#include "object_helper.h"

static inline jfieldID findField(JNIEnv *env, jobject obj, const char *name, const char *sig) {
    jclass cls = env->GetObjectClass(obj);
    if (cls == NULL) {
        return NULL;
    }
    return env->GetFieldID(cls, name, sig);
}

jint readInt(JNIEnv *env, jobject obj, const char *name, const char *sig) {
    jfieldID fid = findField(env, obj, name, sig);
    if (fid == NULL) {
        return -1;
    }
    return env->GetIntField(obj, fid);
}

void writeInt(JNIEnv *env, jobject obj, const char *name, const char *sig, __u32 data) {
    jfieldID fid = findField(env, obj, name, sig);
    if (fid == NULL) {
        return;
    }
    env->SetIntField(obj, fid, (jint) data);
}

void writeLong(JNIEnv *env, jobject obj, const char *name, const char *sig, __u64 data) {
    jfieldID fid = findField(env, obj, name, sig);
    if (fid == NULL) {
        return;
    }
    env->SetLongField(obj, fid, (jlong) data);
}

void writeObj(JNIEnv *env, jobject obj, const char *name, const char *sig, jobject data) {
    jfieldID fid = findField(env, obj, name, sig);
    if (fid == NULL) {
        return;
    }
    env->SetObjectField(obj, fid, data);
}
