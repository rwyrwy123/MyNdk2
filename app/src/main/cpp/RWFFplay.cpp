//
// Created by asus on 2019-03-01.
//

#include "RWFFplay.h"


RWFFplay::RWFFplay(const char *path, RwCallback *prepareLs,RWFFstate *rwfFstate) {
    this->path = path;
    this->fstate = rwfFstate;
    this->callback = prepareLs;

//    if (prepareLs->javaVM == NULL){
//        LOGE("javaVM is empty");
//    } else{
//        LOGE("javaVM is not empty");
//    }

    exit = false;
    pthread_mutex_init(&init_mutex,NULL);
    pthread_mutex_init(&seek_mutex,NULL);
}

RWFFplay::~RWFFplay() {
    pthread_mutex_destroy(&init_mutex);
    pthread_mutex_destroy(&seek_mutex);
}

void *preparedThreadMethod(void *data) {
    RWFFplay *rwfFplay = static_cast<RWFFplay *>(data);
    rwfFplay->prepareThread();
    pthread_exit(&rwfFplay->preparedThread);
}

void RWFFplay::prepare() {
    preparedThread = pthread_create(&preparedThread, NULL, preparedThreadMethod, this);
}

int interupt(void *ctx){
    RWFFplay *fFmpeg = (RWFFplay *) ctx;
    if(fFmpeg->fstate->exit)
    {
        return AVERROR_EOF;
    }
    return 0;
}

void RWFFplay::prepareThread() {

    pthread_mutex_lock(&init_mutex);
    av_register_all();
    avformat_network_init();
    int rest;
    avFormatContext = avformat_alloc_context();
    if (avFormatContext == NULL) {
        if (LOG_DEBUG) {
            LOGE("avFormatContext create fail");
        }
        callback->error(SUB_THREAD,1001,"avFormatContext create fail");
        return;
    }
    avFormatContext->interrupt_callback.callback = interupt;
    avFormatContext->interrupt_callback.opaque = this;
    rest = avformat_open_input(&avFormatContext, path, NULL, NULL);
    if (rest != 0) {
        if (LOG_DEBUG) {
            LOGE("avformatcontxt open fail");
        }
        callback->error(SUB_THREAD,1002,"avformatcontxt open fail");
        return;
    }
    rest = avformat_find_stream_info(avFormatContext, NULL);
    if (rest < 0) {
        if (LOG_DEBUG) {
            LOGE("stream can not find");
        }
        return;
    }

    for (int i = 0; i < avFormatContext->nb_streams; i++) {
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
//            rwAudio = new RWAudio();
            if (rwAudio == NULL) {
                rwAudio = new RWAudio(fstate,avFormatContext->streams[i]->codecpar->sample_rate,callback);
                rwAudio->streamIndex = i;
                rwAudio->avCodecParameters = avFormatContext->streams[i]->codecpar;
                rwAudio->duration = avFormatContext->duration / AV_TIME_BASE;
                duration = rwAudio->duration;
                rwAudio->time_base = avFormatContext->streams[i]->time_base;
            }
        }
    }

    if (rwAudio == NULL) {
        if (LOG_DEBUG) {
            LOGE("audio can not find");
        }
        return;
    }

    AVCodec *dec = avcodec_find_decoder(rwAudio->avCodecParameters->codec_id);
    if (dec == NULL) {
        if (LOG_DEBUG) {
            LOGE("decoder can not find");
        }
        return;
    }
    rwAudio->avCodecContext = avcodec_alloc_context3(dec);
    if (rwAudio->avCodecContext == NULL) {
        if (LOG_DEBUG) {
            LOGE("avCodecContext can not create");
        }
        return;
    }
    rest = avcodec_parameters_to_context(rwAudio->avCodecContext, rwAudio->avCodecParameters);
    if (rest < 0) {
        if (LOG_DEBUG) {
            LOGE("parameters can not to condecContext");
        }
        return;
    }
    rest = avcodec_open2(rwAudio->avCodecContext, rwAudio->avCodec, NULL);
    if (rest != 0) {
        if (LOG_DEBUG) {
            LOGE("avcodec_open to fail");
        }
        return;
    }
    if (callback != NULL) {
//
        if (callback->javaVM == NULL){
            LOGE("javaVM is empty");
        } else{
            LOGE("javaVM is not empty");
        }

        callback->prepare(SUB_THREAD);
    }

    pthread_mutex_unlock(&init_mutex);
}

void RWFFplay::startThreadRun() {
    if (rwAudio == NULL) {
        return;
    }

    // 获取数据
    rwAudio->playaudio();
    // 入队
    while (fstate != NULL && !fstate->exit) {

        if (rwAudio->seek){
            continue;
        }
        if (rwAudio->rwAudioQuene->getAVPacketSize() > 40){
            continue;
        }

        AVPacket *avPacket = av_packet_alloc();
        if (avPacket == NULL) {
            break;
        }
        pthread_mutex_lock(&seek_mutex);
        int ret = av_read_frame(avFormatContext, avPacket);
        pthread_mutex_unlock(&seek_mutex);
        if (ret == 0) {
            if (avPacket->stream_index == rwAudio->streamIndex) {
                rwAudio->rwAudioQuene->pushAVPacket(avPacket);
            }
        } else {
            av_packet_free(&avPacket);
            free(avPacket);
            if (rwAudio ->rwAudioQuene->getAVPacketSize() >0){
                continue;
            } else{
                fstate->exit = true;
                break;
            }
        }
    }
}

void *startThreadMethod(void *data) {
    RWFFplay *rwfFplay = static_cast<RWFFplay *>(data);
    rwfFplay->startThreadRun();
    pthread_exit(&rwfFplay->startThread);
}

void RWFFplay::start() {
    startThread = pthread_create(&startThread, NULL, startThreadMethod, this);
}

void RWFFplay::pause() {
    if (rwAudio != NULL){
        rwAudio->pause();
    }
}

void RWFFplay::continuePlay() {
    if (rwAudio != NULL){
        rwAudio->continuePlay();
    }
}

void RWFFplay::stop() {
    if (rwAudio != NULL){
        rwAudio->stop();
    }
}

void RWFFplay::release() {
    if(LOG_DEBUG)
    {
        LOGE("RWFFplay release");
    }

    if (fstate != NULL && fstate->exit){
        return;
    }
    fstate->exit = true;
    pthread_mutex_lock(&init_mutex);
//    int count = 0;
    while (!exit){
//        if (count > 1000){
//            exit = true;
//        }
        av_usleep(1000 * 10);//暂停10毫秒
        exit = true;
    }
    if(rwAudio != NULL)
    {
        delete(rwAudio);
        rwAudio = NULL;
    }

    if(avFormatContext != NULL)
    {
        avformat_close_input(&avFormatContext);
        avformat_free_context(avFormatContext);
        avFormatContext = NULL;
    }

    if(callback != NULL)
    {
        callback = NULL;
    }
    if(fstate != NULL)
    {
        fstate = NULL;
    }
    if(LOG_DEBUG)
    {
        LOGE("release done");
    }
    pthread_mutex_unlock(&init_mutex);
}

void RWFFplay::seek(int64_t perscent) {

    if (duration <= 0){
        return;
    }
    if (perscent > 0 &&perscent <= duration){
        if (rwAudio != NULL){
            rwAudio->seek = true;
            rwAudio->rwAudioQuene->releaseAVPacket();
            rwAudio->currentDuration = 0;
            rwAudio->currentLast = 0;
            pthread_mutex_lock(&seek_mutex);
            int64_t time =  perscent * AV_TIME_BASE;
            avformat_seek_file(avFormatContext,-1,INT64_MIN,time,INT64_MAX,0);
            pthread_mutex_unlock(&seek_mutex);
            rwAudio->seek = false;
        }
    }

}

void RWFFplay::mutesolo(int solotype) {
    if (rwAudio != NULL){
        rwAudio->mutesolo(solotype);
    }
}

void RWFFplay::pitchspeed(double pitch, double speed) {
    if (rwAudio != NULL){
        rwAudio->pitchspeed(pitch,speed);
    }
}

void RWFFplay::volume(int volume) {
    if (rwAudio != NULL){
        rwAudio->volume(volume);
    }
}


