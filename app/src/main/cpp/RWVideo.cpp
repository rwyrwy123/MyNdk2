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
            continue;
        }
        if (avcodec_send_packet(video->avCodecContext, avPacket) != 0) {
            continue;
        }
        AVFrame *avFrame = av_frame_alloc();
        if (avFrame == NULL) {
            continue;
        }
        if (avcodec_receive_frame(video->avCodecContext, avFrame) != 0) {
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        if (avFrame->format = AV_PIX_FMT_YUV420P) {
            video->callback->yuv(video->avCodecContext->width, video->avCodecContext->height,
                                 avFrame->data[0],
                                 avFrame->data[1],
                                 avFrame->data[2]);
        } else {
            AVFrame *avFrame420 = av_frame_alloc();
            int num = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, video->avCodecContext->width,
                                               video->avCodecContext->height, 1);
            uint8_t *buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));
            av_image_fill_arrays(avFrame420->data, avFrame420->linesize,
                                 buffer,
                                 AV_PIX_FMT_YUV420P,
                                 video->avCodecContext->width,
                                 video->avCodecContext->height,
                                 1);
            SwsContext *swsContext = sws_getContext(video->avCodecContext->width,
                                                    video->avCodecContext->height,
                                                    video->avCodecContext->pix_fmt,
                                                    video->avCodecContext->width,
                                                    video->avCodecContext->height,
                                                    AV_PIX_FMT_YUV420P,SWS_BICUBIC,NULL,NULL,NULL);
            if (swsContext == NULL) {
                av_frame_free(&avFrame420);
                av_free(avFrame420);
                av_free(buffer);
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }
            sws_scale(swsContext,avFrame->data,avFrame->linesize,0,
                    avFrame->height,avFrame420->data,avFrame->linesize);
            video->callback->yuv(video->avCodecContext->width, video->avCodecContext->height,
                                 avFrame420->data[0],
                                 avFrame420->data[1],
                                 avFrame420->data[2]);
            av_frame_free(&avFrame420);
            av_free(avFrame420);
            av_free(buffer);
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
        }

    }
    pthread_detach(video->playthread);
}

void RWVideo::play() {
    pthread_create(&playthread, NULL, playThread, this);
}
