//
// Created by Administrator on 3/10/2019.
//

#include "RWVideo.h"

RWVideo::RWVideo(RwCallback *callback, RWFFstate *fstate) {
    this->callback = callback;
    this->fstate = fstate;
    videoQuene = new RWAudioQuene();
}

RWVideo::~RWVideo() {

}

void *playThread(void *data) {
    RWVideo *video = static_cast<RWVideo *>(data);
    while (video->fstate != NULL && !video->fstate->exit) {
        AVPacket *avPacket = av_packet_alloc();
        video->videoQuene->popAVPacket(avPacket);
        if (avPacket == NULL) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
        }
        LOGE("video decode");
    }
    pthread_detach(video->playthread);
}

void RWVideo::play() {
    pthread_create(&playthread, NULL, playThread, this);
}
