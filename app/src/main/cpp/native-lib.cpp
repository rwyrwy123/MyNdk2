#include <jni.h>
#include <string>
#include "ffLog.h"

extern "C"
{
#include "libavformat/avformat.h"
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_myndk1_myndk_MyNDKF_testHasFF(JNIEnv *env, jobject instance) {

    av_register_all();
    AVCodec *c_temp = av_codec_next(NULL);
    while (c_temp != NULL) {
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                LOGI("[Video]:%s", c_temp->name);
                break;
            case AVMEDIA_TYPE_AUDIO:
                LOGI("[Audio]:%s", c_temp->name);
                break;
            default:
                LOGI("[Other]:%s", c_temp->name);
                break;
        }
        c_temp = c_temp->next;
    }
}


#include "RWFFplay.h"
#include "RwCallback.h"
#include "RWOpenSL.h"

RWFFplay *pffplay = NULL;
JavaVM *javaVM;
RwCallback *prepareLs = NULL;
RWOpenSL *rwOpenSL = NULL;
RWFFstate *fstate = NULL;

extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;
    if (javaVM->GetEnv((void **) (&env), JNI_VERSION_1_4) == JNI_OK) {
        return JNI_VERSION_1_4;
    }
    return result;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_myndk1_ff_FFNdk_prepare(JNIEnv *env, jobject instance, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    if (path == NULL) {
        if (LOG_DEBUG) {
            LOGE("文件为空");
        }
        return;
    }
    if (pffplay == NULL) {
        prepareLs = new RwCallback(javaVM, env, instance);
//        if (javaVM == NULL){
//            LOGE("javaVM is empty");
//        } else{
//            LOGE("javaVM is not empty");
//        }
        fstate = new RWFFstate();
        pffplay = new RWFFplay(path, prepareLs,fstate);
    }
    pffplay->prepare();

}extern "C"
JNIEXPORT void JNICALL
Java_com_example_myndk1_ff_FFNdk_playPcm(JNIEnv *env, jobject instance, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    if (rwOpenSL == NULL) {
        rwOpenSL = new RWOpenSL(path);
    }
    rwOpenSL->playPcm();
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_myndk1_ff_FFNdk_start__(JNIEnv *env, jobject instance) {
    if (pffplay == NULL) {
        if (LOG_DEBUG) {
            LOGI("pffplay is empty")
        }
        return;
    }
    pffplay->start();
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_myndk1_ff_FFNdk_pause(JNIEnv *env, jobject instance) {
    if (pffplay != NULL) {
        pffplay->pause();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_example_myndk1_ff_FFNdk_stop(JNIEnv *env, jobject instance) {

    if (pffplay != NULL) {
        pffplay->release();
        delete (pffplay);
        pffplay = NULL;
        if( prepareLs != NULL)
        {
            prepareLs->stop(MAIN_THREAD);
//            delete(prepareLs);
            prepareLs = NULL;
        }
        if(fstate != NULL)
        {
//            delete(fstate);
            fstate = NULL;
        }
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_example_myndk1_ff_FFNdk_continuedplay(JNIEnv *env, jobject instance) {

    if (pffplay != NULL) {
        pffplay->continuePlay();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_example_myndk1_ff_FFNdk_next(JNIEnv *env, jobject instance, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);

    // TODO

    env->ReleaseStringUTFChars(path_, path);
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_myndk1_ff_FFNdk_seek(JNIEnv *env, jobject instance, jint perscent) {

    // TODO
    if (pffplay != NULL){
        pffplay->seek(perscent);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_myndk1_ff_FFNdk_mutesolo(JNIEnv *env, jobject instance, jint solotype) {

    // TODO
    if (pffplay != NULL){
        pffplay->mutesolo(solotype);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_myndk1_ff_FFNdk_pitchspeed(JNIEnv *env, jobject instance, jdouble pitch,
                                            jdouble speed) {

    // TODO
    if (pffplay != NULL){
        pffplay->pitchspeed(pitch,speed);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_myndk1_ff_FFNdk_volume(JNIEnv *env, jobject instance, jint volume) {

    // TODO
    if (pffplay != NULL){
        pffplay->volume(volume);
    }
}