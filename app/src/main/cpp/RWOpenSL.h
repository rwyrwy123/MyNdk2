//
// Created by asus on 2019-03-05.
//

#ifndef MYNDK1_RWOPENSL_H
#define MYNDK1_RWOPENSL_H


#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "ffLog.h"
#include "jni.h"
#include "stdio.h"
#include "malloc.h"

class RWOpenSL {

public:
    const char* path;
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

     FILE *filePcm = NULL;
     uint8_t *buffer = NULL;
public:
    RWOpenSL(const char* path);
    ~RWOpenSL();
    void playPcm();


};


#endif //MYNDK1_RWOPENSL_H
