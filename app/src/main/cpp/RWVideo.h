//
// Created by Administrator on 3/10/2019.
//

#ifndef MYNDK2_RWVIDEO_H
#define MYNDK2_RWVIDEO_H

#include "RwCallback.h"
#include "RWFFstate.h"
#include "RWAudioQuene.h"

extern "C" {

#include <libavcodec/avcodec.h>
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
public:
    RWVideo(RwCallback *callback,RWFFstate *fstate);
    ~RWVideo();
    void play();
};


#endif //MYNDK2_RWVIDEO_H
