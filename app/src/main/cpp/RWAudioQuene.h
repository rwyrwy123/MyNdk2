//
// Created by asus on 2019-03-05.
//

#ifndef MYNDK1_RWAUDIOQUENE_H
#define MYNDK1_RWAUDIOQUENE_H

#include "queue"
#include "pthread.h"
#include "ffLog.h"
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"
extern "C"{
#include "libavformat/avformat.h"
};

class RWAudioQuene {

public:
    std::queue<AVPacket *> audioQuene;
    pthread_mutex_t pthread_mutex;
    pthread_cond_t pthread_cond;
public:
    RWAudioQuene();
    ~RWAudioQuene();
    void popAVPacket(AVPacket *avPacket);
    void pushAVPacket(AVPacket *avPacket);
    int getAVPacketSize();

    void releaseAVPacket();
};


#endif //MYNDK1_RWAUDIOQUENE_H
