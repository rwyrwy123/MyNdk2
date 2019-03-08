//
// Created by asus on 2019-03-05.
//

#include "RWOpenSL.h"

RWOpenSL::RWOpenSL(const char *path) {
    this->path = path;
    filePcm = fopen(path,"r");
    buffer = static_cast<uint8_t *>(malloc(44100 * 2 * 2));
}

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context){

    RWOpenSL *rwOpenSL = static_cast<RWOpenSL *>(context);
    if (rwOpenSL->filePcm == NULL){
        if (LOG_DEBUG){
            LOGD("filePcm no exit")
        }
    }
    while (!feof(rwOpenSL->filePcm)){
        fread(rwOpenSL->buffer,44100*2*2,1,rwOpenSL->filePcm);
        if(rwOpenSL->buffer == NULL)
        {
            LOGI("%s", "read end");
            break;
        } else{
            LOGI("%s", "reading");
        }
        break;
    }
    if (NULL != rwOpenSL->buffer) {
        SLresult result;
        result = (*rwOpenSL->bqPlayerBufferQueue)->Enqueue(rwOpenSL->bqPlayerBufferQueue, rwOpenSL->buffer, 44100 * 2 * 2);
    }
}

void RWOpenSL::playPcm() {
    SLresult result;
    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result){
        if (LOG_DEBUG){
            LOGE("enginObject create fail");
        }
        return;
    }

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result){
        if (LOG_DEBUG){
            LOGE("enginObject realize fail");
        }
        return;
    }

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (SL_RESULT_SUCCESS != result){
        if (LOG_DEBUG){
            LOGE("engineEngine create fail");
        }
        return;
    }

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    if (SL_RESULT_SUCCESS != result){
        if (LOG_DEBUG){
            LOGE("outputMixObject create fail");
        }
        return;
    }

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result){
        if (LOG_DEBUG){
            LOGE("outputMixObject Realize fail");
        }
        return;
    }

    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    const SLInterfaceID ids2[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/};
    const SLboolean req2[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ };

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
                                                3, ids2, req2);
    if (SL_RESULT_SUCCESS == result) {
        (void)result;
    }
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS == result) {
        (void)result;
    }

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    if (SL_RESULT_SUCCESS == result) {
        (void)result;
    }

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);
    if (SL_RESULT_SUCCESS == result) {
        (void)result;
    }

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);
    if (SL_RESULT_SUCCESS == result) {
        (void)result;
    }
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    bqPlayerCallback(bqPlayerBufferQueue, this);
}


RWOpenSL::~RWOpenSL() {

}
