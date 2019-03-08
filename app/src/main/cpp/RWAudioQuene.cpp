//
// Created by asus on 2019-03-05.
//

#include "RWAudioQuene.h"

RWAudioQuene::RWAudioQuene() {
    pthread_mutex_init(&pthread_mutex, NULL);
    pthread_cond_init(&pthread_cond, NULL);
}

RWAudioQuene::~RWAudioQuene() {
    pthread_mutex_destroy(&pthread_mutex);
    pthread_cond_destroy(&pthread_cond);
}

void RWAudioQuene::popAVPacket(AVPacket *avPacket) {
    pthread_mutex_lock(&pthread_mutex);
    if (audioQuene.size() > 0) {

        AVPacket *packet = audioQuene.front();
        if (av_packet_ref(avPacket, packet) == 0) {
            audioQuene.pop();
        }
        av_packet_free(&packet);
        free(packet);
    } else {
        if (LOG_DEBUG) {
            LOGI("popAVPacket wait")
        }
        pthread_cond_wait(&pthread_cond, &pthread_mutex);
    }

    pthread_mutex_unlock(&pthread_mutex);
}

void RWAudioQuene::pushAVPacket(AVPacket *avPacket) {
    pthread_mutex_lock(&pthread_mutex);
    audioQuene.push(avPacket);
    pthread_cond_signal(&pthread_cond);
    pthread_mutex_unlock(&pthread_mutex);
}

int RWAudioQuene::getAVPacketSize() {
    int res = 0;
    pthread_mutex_lock(&pthread_mutex);
    res = audioQuene.size();
    pthread_mutex_unlock(&pthread_mutex);
    return res;
}

void RWAudioQuene::releaseAVPacket() {
    if(LOG_DEBUG)
    {
        LOGE("RWAudioQuene releaseAVPacket");
    }
    pthread_cond_signal(&pthread_cond);
    pthread_mutex_lock(&pthread_mutex);
    while (!audioQuene.empty()){
        AVPacket *packet = audioQuene.front();
        audioQuene.pop();
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }
    pthread_mutex_unlock(&pthread_mutex);
}
