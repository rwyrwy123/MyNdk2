//
// Created by asus on 2019-03-04.
//

#ifndef MYNDK1_RWAUDIO_H
#define MYNDK1_RWAUDIO_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
};

#include "RWAudioQuene.h"
#include "ffLog.h"
#include "RWFFstate.h"
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"
#include "RwCallback.h"

class RWAudio {

public:
    int streamIndex;
    AVCodecContext *avCodecContext = NULL;
    AVCodec *avCodec = NULL;
    AVCodecParameters *avCodecParameters = NULL;
    RWAudioQuene *rwAudioQuene = NULL;
    RwCallback *callback = NULL;
    int sample_rate;

    RWFFstate *fstate = NULL;
    uint8_t *outBuffer = NULL;
    pthread_t playThread = NULL;

    // engine interfaces
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine;

// output mix interfaces
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings =
            SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

// buffer queue player interfaces
    SLObjectItf bqPlayerObject = NULL;
    SLPlayItf bqPlayerPlay;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
    SLMuteSoloItf muteSoloItf;

    AVRational time_base;
    double duration = 0;
    double currentDuration = 0 ;
    double currentLast = 0 ;
    bool seek = false;

public:
    RWAudio(RWFFstate *fstate,int sample_rate,RwCallback *callback);
    ~RWAudio();
    void playaudio();
    void initOpenSL();
    int convertTopcm();
    void pause();
    void continuePlay();
    void stop();
    void release();
    int getCurrentSampleRateForOpensles(int sample_rate);

    void mutesolo(int solotype);
};


#endif //MYNDK1_RWAUDIO_H
