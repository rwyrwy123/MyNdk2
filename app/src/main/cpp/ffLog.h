//
// Created by asus on 2019-03-01.
//

#ifndef MYNDK1_FFLOG_H
#define MYNDK1_FFLOG_H

#include "android/log.h"

#define LOG_DEBUG true

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"rwy",FORMAT,##__VA_ARGS__);
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"rwy",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"rwy",FORMAT,##__VA_ARGS__);


#endif //MYNDK1_FFLOG_H
