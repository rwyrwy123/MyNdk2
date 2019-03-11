//
// Created by asus on 2019-03-01.
//

#ifndef MYNDK1_RWFFplay_H
#define MYNDK1_RWFFplay_H

extern "C"{
#include "libavformat/avformat.h"
#include "libavutil/time.h"
};

#include "RwCallback.h"
#include "RWAudio.h"
#include "pthread.h"
#include "RWFFstate.h"
#include "RWVideo.h"

class RWFFplay {

public:
    const char *path;
    AVFormatContext *avFormatContext ;
    RwCallback *callback ;
    RWAudio *rwAudio = NULL;
    RWVideo *rwVideo = NULL;
    pthread_t preparedThread;
    pthread_t startThread;
    int rest;
    RWFFstate *fstate = NULL;
    int exit = false; // 多次点击停止
    pthread_mutex_t init_mutex;
    double duration = 0;
    pthread_mutex_t seek_mutex;

public:
    RWFFplay(const char *path,RwCallback *prepareLs,RWFFstate *rwfFstate);
    ~RWFFplay();
    void prepare();
    void prepareThread();
    void start();
    void startThreadRun();
    void pause();
    void continuePlay();
    void stop();
    void release();
    void seek(int64_t perscent);
    void mutesolo(int solotype);
    void pitchspeed(double pitch, double speed);
    void volume(int volume);
    void initAvpacket(AVCodecParameters *parameters,AVCodecContext **avCodecContext);
};


#endif //MYNDK1_RWFFplay_H
