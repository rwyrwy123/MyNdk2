//
// Created by Administrator on 3/10/2019.
//

#ifndef MYNDK2_RWVIDEO_H
#define MYNDK2_RWVIDEO_H

#include "RwCallback.h"
#include "RWFFstate.h"
#include "RWAudioQuene.h"
#include "RWAudio.h"

extern "C" {

#include <libavcodec/avcodec.h>
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
};

class RWVideo {

public:
    int streamIndex;
    AVCodecContext *avCodecContext = NULL;
    AVCodecParameters *avCodecParameters = NULL;
    AVRational time_base;
    RWAudioQuene *videoQuene = NULL;
    RwCallback *callback = NULL;
    RWFFstate *fstate = NULL;
    pthread_t playthread;
    RWAudio *audio = NULL;
    int clock = 0;
    double defaultDelayTime = 0.04;
    double delayTime = 0;
    pthread_mutex_t seekMutex;

public:
    RWVideo(RwCallback *callback,RWFFstate *fstate);
    ~RWVideo();
    void play();
    double getCurrentTime(AVFrame *avFrame);
    double getDelayTime(double diff);
};


#endif //MYNDK2_RWVIDEO_H
