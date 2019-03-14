//
// Created by asus on 2019-03-01.
//

#include "RwCallback.h"

RwCallback::RwCallback(JavaVM *vm, JNIEnv *env, jobject jobject1) {
    this->javaVM = vm;
    this->jniEnv = env;
    this->obj = jobject1;
    this->obj = env->NewGlobalRef(obj);
    jclass jcz = env->GetObjectClass(obj);
    if (!jcz){
        if(LOG_DEBUG)
        {
            LOGE("get jclass wrong");
        }
        return;
    }
    this->jmid_prepare = env->GetMethodID(jcz, "prepareCtJ", "()V");
    this->jmid_error = env->GetMethodID(jcz, "errorCtJ", "(ILjava/lang/String;)V");
    this->jmid_duration = env->GetMethodID(jcz, "durationCtJ", "(II)V");
    this->jmid_pause = env->GetMethodID(jcz, "pauseCtJ", "()V");
    this->jmid_stop = env->GetMethodID(jcz, "stopCtJ", "()V");
    this->jmid_yuv = env->GetMethodID(jcz, "yuvCtJ", "(II[B[B[B)V");
}


RwCallback::~RwCallback() {

}

void RwCallback::prepare(int type) {

    if(type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(obj,jmid_prepare);
    } else if (type == SUB_THREAD) {
        JNIEnv *jenv;

        if(LOG_DEBUG)
        {
            if (javaVM == NULL){
                LOGE("javaVM is empty");
            } else{
                LOGE("javaVM is not empty");
            }
        }
        if(javaVM->AttachCurrentThread(&jenv, NULL) != JNI_OK)
        {
            if(LOG_DEBUG)
            {
                LOGE("get child thread jnienv worng");
            }
            return;
        }
        jenv->CallVoidMethod(obj,jmid_prepare);
        javaVM->DetachCurrentThread();
    }
}

void RwCallback::error(int type, int code, char *msg) {

    if(type == MAIN_THREAD) {
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(obj,jmid_error,code,jmsg);
        jniEnv->DeleteLocalRef(jmsg);
    } else if (type == SUB_THREAD) {
        JNIEnv *jenv;
        if(javaVM->AttachCurrentThread(&jenv, NULL) != JNI_OK)
        {
            if(LOG_DEBUG)
            {
                LOGE("get child thread jnienv worng");
            }
            return;
        }
        jstring jmsg = jenv->NewStringUTF(msg);
        jenv->CallVoidMethod(obj,jmid_error,code,jmsg);
        jenv->DeleteLocalRef(jmsg);
        javaVM->DetachCurrentThread();
    }
}

void RwCallback::duration(int type,int duration,int currentDuration) {
    if(type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(obj,jmid_duration,duration,currentDuration);
    } else if (type == SUB_THREAD) {
        JNIEnv *jenv;
        if(javaVM->AttachCurrentThread(&jenv, NULL) != JNI_OK)
        {
            if(LOG_DEBUG)
            {
                LOGE("get child thread jnienv worng");
            }
            return;
        }
        jenv->CallVoidMethod(obj,jmid_duration,duration,currentDuration);
        javaVM->DetachCurrentThread();
    }
}

void RwCallback::stop(int type) {
    if(type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(obj,jmid_stop);
    } else if (type == SUB_THREAD) {
        JNIEnv *jenv;
        if(javaVM->AttachCurrentThread(&jenv, NULL) != JNI_OK)
        {
            if(LOG_DEBUG)
            {
                LOGE("get child thread jnienv worng");
            }
            return;
        }
        jenv->CallVoidMethod(obj,jmid_stop);
        javaVM->DetachCurrentThread();
    }
}

void RwCallback::pause(int type) {
    if(type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(obj,jmid_pause);
    } else if (type == SUB_THREAD) {
        JNIEnv *jenv;
        if(javaVM->AttachCurrentThread(&jenv, NULL) != JNI_OK)
        {
            if(LOG_DEBUG)
            {
                LOGE("get child thread jnienv worng");
            }
            return;
        }
        jenv->CallVoidMethod(obj,jmid_pause);
        javaVM->DetachCurrentThread();
    }
}

void RwCallback::yuv(int width, int height, uint8_t *y, uint8_t *u, uint8_t *v) {

    JNIEnv *jenv;
    if(javaVM->AttachCurrentThread(&jenv, NULL) != JNI_OK)
    {
        if(LOG_DEBUG)
        {
            LOGE("get child thread jnienv worng");
        }
        return;
    }

    jbyteArray yarray = jenv->NewByteArray(width*height);
    jenv->SetByteArrayRegion(yarray, 0,width * height, reinterpret_cast<jbyte *>(y));
    jbyteArray uarray = jenv->NewByteArray(width*height/4);
    jenv->SetByteArrayRegion(uarray, 0,width * height/4, reinterpret_cast<jbyte *>(u));
    jbyteArray varray = jenv->NewByteArray(width*height);
    jenv->SetByteArrayRegion(varray, 0,width * height/4, reinterpret_cast<jbyte *>(v));
    jenv->CallVoidMethod(obj, jmid_yuv, width, height, yarray, uarray, varray);
    jenv->DeleteLocalRef(yarray);
    jenv->DeleteLocalRef(uarray);
    jenv->DeleteLocalRef(varray);
    javaVM->DetachCurrentThread();

}
