//
// Created by asus on 2019-03-04.
//
#include "RWAudio.h"


RWAudio::RWAudio(RWFFstate *fstate, int sample_rate, RwCallback *callback) {
    rwAudioQuene = new RWAudioQuene();
    this->fstate = fstate;
    this->sample_rate = sample_rate;
    this->callback = callback;
    outBuffer = static_cast<uint8_t *>(av_malloc(sample_rate * 2 * 2));

    soundTouch = new SoundTouch();
    sampleBuffer = static_cast<SAMPLETYPE *>(malloc(sample_rate * 2 * 2));
    soundTouch->setSampleRate(sample_rate);
    soundTouch->setChannels(2);
    soundTouch->setPitch(pitch);
    soundTouch->setTempo(speed);
    pthread_mutex_init(&seekMutex,NULL);
}

RWAudio::~RWAudio() {
    release();
    pthread_mutex_destroy(&seekMutex);
}

int RWAudio::convertTopcm(void **soundData) {
    int datasize = 0;
    while (fstate != NULL && !fstate->exit) {
        if (rwAudioQuene->getAVPacketSize() > 0) {
            AVPacket *avPacket = av_packet_alloc();
            if (avPacket == NULL) {
                if (LOG_DEBUG) {
                    LOGI("convertTopcm AVPacket create fail")
                }
                continue;
            }
            rwAudioQuene->popAVPacket(avPacket);
            pthread_mutex_lock(&seekMutex);
            if (avcodec_send_packet(avCodecContext, avPacket) != 0) {
                av_packet_free(&avPacket);
                free(avPacket);
                pthread_mutex_unlock(&seekMutex);
                continue;
            };

            AVFrame *avFrame = av_frame_alloc();
            if (avFrame == NULL) {
                if (LOG_DEBUG) {
                    LOGI("convertTopcm avFrame create fail")
                }
                pthread_mutex_unlock(&seekMutex);
                continue;
            }
            if (avcodec_receive_frame(avCodecContext, avFrame) != 0) {
                av_frame_free(&avFrame);
                free(avFrame);
                av_packet_free(&avPacket);
                free(avPacket);
                pthread_mutex_unlock(&seekMutex);
                continue;
            };

            if (avFrame->channels = 0 && avFrame->channel_layout != 0) {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            } else if (avFrame->channels != 0 && avFrame->channel_layout == 0) {
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            }
            SwrContext *swrContext;

            swrContext = swr_alloc_set_opts(NULL,
                                            AV_CH_LAYOUT_STEREO,
                                            AV_SAMPLE_FMT_S16,
                                            avFrame->sample_rate,
                                            avFrame->channel_layout,
                                            static_cast<AVSampleFormat>(avFrame->format),
                                            avFrame->sample_rate, NULL, NULL);

            if (!swrContext || swr_init(swrContext) < 0) {
                if (LOG_DEBUG) {
                    LOGI("swrContext create fail")
                }
                av_frame_free(&avFrame);
                free(avFrame);
                av_packet_free(&avPacket);
                free(avPacket);
                swr_free(&swrContext);
                pthread_mutex_unlock(&seekMutex);
                continue;
            }
            nb = swr_convert(swrContext,
                             &outBuffer, avFrame->nb_samples,
                             (const uint8_t **) avFrame->data,
                             avFrame->nb_samples);

            int channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            datasize = nb * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * channels;
            *soundData = outBuffer;
            currentDuration = avFrame->pts * av_q2d(time_base);
            if (currentDuration > duration) {
                currentDuration = duration;
            }
            av_frame_free(&avFrame);
            free(avFrame);
            av_packet_free(&avPacket);
            free(avPacket);
            swr_free(&swrContext);
            pthread_mutex_unlock(&seekMutex);
            break;
        }
    }
    return datasize;
}

int RWAudio::receiveSound() {

    while (fstate != NULL && !fstate->exit) {

        if(fstate->seek)
        {
            av_usleep(1000*100);
            continue;
        }

        soundPcm = NULL;
        if (finish) {
            finish = false;
            data_size = convertTopcm(reinterpret_cast<void **>(&soundPcm));
            if (data_size > 0) {
                for (int i = 0; i < data_size / 2 + 1; i++) {
                    sampleBuffer[i] = (soundPcm[i * 2] | (soundPcm[i * 2 + 1] << 8));
                }
                soundTouch->putSamples(sampleBuffer, nb);
                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
            } else {
                soundTouch->flush();
            }
        }
        if (num == 0) {
            finish = true;
            continue;
        } else {
            if (soundPcm == NULL) {
                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
                if (num == 0) {
                    finish = true;
                    continue;
                }
            }
            return num;
        }
    }
    return 0;
}

void bqCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    RWAudio *rwAudio = static_cast<RWAudio *>(context);
    int size = rwAudio->receiveSound();
//    int size = rwAudio->convertTopcm(reinterpret_cast<void **>(&rwAudio->soundPcm));
    if (size > 0) {
        (*rwAudio->bqPlayerBufferQueue)->Enqueue(rwAudio->bqPlayerBufferQueue,
                                                 rwAudio->sampleBuffer, size * 2 * 2);
//        (*rwAudio->bqPlayerBufferQueue)->Enqueue(rwAudio->bqPlayerBufferQueue,
//                                                 rwAudio->outBuffer, size);

        rwAudio->currentDuration += size / (rwAudio->sample_rate * 2 * 2);
        if (rwAudio->currentDuration - rwAudio->currentLast >= 0.1) {
            rwAudio->currentLast = rwAudio->currentDuration;
            rwAudio->callback->duration(SUB_THREAD, rwAudio->duration, rwAudio->currentDuration);
        }
    }
}

void *decodeThread(void *data) {
    RWAudio *rwAudio = static_cast<RWAudio *>(data);
    rwAudio->initOpenSL();
    pthread_exit(&rwAudio->playThread);
}

void RWAudio::playaudio() {
    pthread_create(&playThread, NULL, decodeThread, this);
}

void RWAudio::initOpenSL() {
    SLresult result;
    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        if (LOG_DEBUG) {
            LOGE("enginObject create fail");
        }
        return;
    }

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        if (LOG_DEBUG) {
            LOGE("enginObject realize fail");
        }
        return;
    }

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (SL_RESULT_SUCCESS != result) {
        if (LOG_DEBUG) {
            LOGE("engineEngine create fail");
        }
        return;
    }

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    if (SL_RESULT_SUCCESS != result) {
        if (LOG_DEBUG) {
            LOGE("outputMixObject create fail");
        }
        return;
    }

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        if (LOG_DEBUG) {
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
        (void) result;
    }

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                       2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2,
                                   getCurrentSampleRateForOpensles(sample_rate),
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                                   SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    const SLInterfaceID ids2[4] = {SL_IID_BUFFERQUEUE, SL_IID_MUTESOLO, SL_IID_VOLUME,
                                   SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/};
    const SLboolean req2[4] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ };

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc,
                                                &audioSnk,
                                                4, ids2, req2);
    if (SL_RESULT_SUCCESS == result) {
        (void) result;
    }
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS == result) {
        (void) result;
    }

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    if (SL_RESULT_SUCCESS == result) {
        (void) result;
    }

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);
    if (SL_RESULT_SUCCESS == result) {
        (void) result;
    }

    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_MUTESOLO, &muteSoloItf);
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &volumeItf);


    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqCallback, this);
    if (SL_RESULT_SUCCESS == result) {
        (void) result;
    }
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    bqCallback(bqPlayerBufferQueue, this);
}

void RWAudio::pause() {
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
}

void RWAudio::continuePlay() {
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    callback->pause(MAIN_THREAD);
}

void RWAudio::stop() {
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
}

int RWAudio::getCurrentSampleRateForOpensles(int sample_rate) {
    int rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

void RWAudio::release() {
    if (LOG_DEBUG) {
        LOGE("RWAudio release");
    }
    stop();
    if (rwAudioQuene != NULL) {
        rwAudioQuene->releaseAVPacket();
        delete rwAudioQuene;
        rwAudioQuene = NULL;
    }

    if (bqPlayerObject != NULL) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        muteSoloItf = NULL;
        volumeItf = NULL;
    }

    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

    if (outBuffer != NULL) {
        free(outBuffer);
        outBuffer = NULL;
    }

    if (avCodecContext != NULL) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        free(avCodecContext);
        avCodecContext = NULL;
    }
    if (avCodec != NULL) {
        free(avCodec);
        avCodec = NULL;
    }
    if (fstate != NULL) {
        fstate = NULL;
    }
    if (callback != NULL) {
        callback = NULL;
    }

}

void RWAudio::mutesolo(int solotype) {
    if (muteSoloItf != NULL) {
        if (solotype == 0)//right
        {
            (*muteSoloItf)->SetChannelMute(muteSoloItf, 1, false);
            (*muteSoloItf)->SetChannelMute(muteSoloItf, 0, true);
        } else if (solotype == 1)//left
        {
            (*muteSoloItf)->SetChannelMute(muteSoloItf, 1, true);
            (*muteSoloItf)->SetChannelMute(muteSoloItf, 0, false);
        } else if (solotype == 2)//center
        {
            (*muteSoloItf)->SetChannelMute(muteSoloItf, 1, false);
            (*muteSoloItf)->SetChannelMute(muteSoloItf, 0, false);
        }
    }
}

void RWAudio::pitchspeed(double pitch, double speed) {
    this->pitch = pitch;
    this->speed = speed;
    if (soundTouch != NULL){
        soundTouch->setTempo(speed);
        soundTouch->setPitch(pitch);
    }
}

void RWAudio::volume(int volume) {
    (*volumeItf)->SetVolumeLevel(volumeItf,(100 - volume) * -100);
}


