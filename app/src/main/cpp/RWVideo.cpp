//
// Created by Administrator on 3/10/2019.
//

#include "RWVideo.h"

RWVideo::RWVideo(RwCallback *callback, RWFFstate *fstate) {
    this->callback = callback;
    this->fstate = fstate;
    videoQuene = new RWAudioQuene();
    pthread_mutex_init(&seekMutex, NULL);
}

RWVideo::~RWVideo() {
    pthread_mutex_destroy(&seekMutex);
}

void *playThread(void *data) {
    RWVideo *video = static_cast<RWVideo *>(data);
    while (video->fstate != NULL && !video->fstate->exit) {

        if (video->fstate->seek) {
            av_usleep(1000 * 100);
            continue;
        }
        AVPacket *avPacket = av_packet_alloc();
        video->videoQuene->popAVPacket(avPacket);
        if (avPacket == NULL) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        pthread_mutex_lock(&video->seekMutex);
        if (avcodec_send_packet(video->avCodecContext, avPacket) != 0) {
            pthread_mutex_unlock(&video->seekMutex);
            continue;
        }
        AVFrame *avFrame = av_frame_alloc();
        if (avFrame == NULL) {
            pthread_mutex_unlock(&video->seekMutex);
            continue;
        }
        if (avcodec_receive_frame(video->avCodecContext, avFrame) != 0) {
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            pthread_mutex_unlock(&video->seekMutex);
            continue;
        }
        double diff = video->getCurrentTime(avFrame);
        LOGE("diff is %f", diff);
        if (avFrame->format != AV_PIX_FMT_YUV420P) {
            LOGE("当前视频是YUV420P格式");
            av_usleep(video->getDelayTime(diff) * 1000000);
            video->callback->yuv(
                    video->avCodecContext->width,
                    video->avCodecContext->height,
                    avFrame->data[0],
                    avFrame->data[1],
                    avFrame->data[2]);
        } else {
            LOGE("当前视频不是YUV420P格式");
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
                                                    AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL,
                                                    NULL);
            if (!swsContext) {
                av_frame_free(&avFrame420);
                av_free(avFrame420);
                av_free(buffer);
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&video->seekMutex);
                continue;
            }
            sws_scale(swsContext, avFrame->data, avFrame->linesize, 0,
                      avFrame->height, avFrame420->data, avFrame420->linesize);
            av_usleep(video->getDelayTime(diff) * 1000000);
            video->callback->yuv(video->avCodecContext->width, video->avCodecContext->height,
                                 avFrame420->data[0],
                                 avFrame420->data[1],
                                 avFrame420->data[2]);
            sws_freeContext(swsContext);
            av_frame_free(&avFrame420);
            av_free(avFrame420);
            av_free(buffer);
        }
        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame = NULL;
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
        pthread_mutex_unlock(&video->seekMutex);
    }
    pthread_exit(&video->playthread);
}

void RWVideo::play() {
    pthread_create(&playthread, NULL, playThread, this);
}

double RWVideo::getCurrentTime(AVFrame *avFrame) {
    double pts = av_frame_get_best_effort_timestamp(avFrame);
    if (pts == AV_NOPTS_VALUE) {
        pts = 0;
    }
    pts *= av_q2d(time_base);
    if (pts > 0) {
        clock = pts;
    }
    double diff = audio->currentDuration - clock;
    return diff;
}

double RWVideo::getDelayTime(double diff) {

    if (diff > 0.003) {
        delayTime = defaultDelayTime * 2 / 3;
        if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime * 2 / 3;
        } else if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        }
    } else if (diff < -0.003) {
        delayTime = delayTime * 3 / 2;
        if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime * 3 / 2;
        } else if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        }
    } else if (diff == 0.003) {

    }
    if (diff >= 0.5) {
        delayTime = 0;
    } else if (diff <= -0.5) {
        delayTime = defaultDelayTime * 2;
    }

    if (fabs(diff) >= 10) {
        delayTime = defaultDelayTime;
    }
    return delayTime;
}
