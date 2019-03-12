//
// Created by asus on 2019-03-01.
//

#ifndef MYNDK1_PREPARELISTENER_H
#define MYNDK1_PREPARELISTENER_H

#include "ffLog.h"
#include <jni.h>
#define SUB_THREAD 0
#define MAIN_THREAD 1

class RwCallback {

public:
    JavaVM *javaVM;
    JNIEnv *jniEnv;
    jobject obj;
    jmethodID jmid_prepare;
    jmethodID jmid_error;
    jmethodID jmid_duration;
    jmethodID jmid_pause;
    jmethodID jmid_stop;
    jmethodID jmid_yuv;
public:
    RwCallback(JavaVM *vm,JNIEnv *env,jobject obj);
    ~RwCallback();
    void prepare(int type);
    void error(int type,int code,char *msg);
    void duration(int type,int duration,int currentDuration);
    void stop(int type);
    void pause(int type);
    void yuv(int width,int height,uint8_t *y,uint8_t *u,uint8_t *v);
};


#endif //MYNDK1_PREPARELISTENER_H
