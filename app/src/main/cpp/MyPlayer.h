//
// Created by bytedance on 2022/11/17.
//

#ifndef NDKDEMO_MYPLAYER_H
#define NDKDEMO_MYPLAYER_H
#include <string>
#include <jni.h>
#include "AudioChannel.h"
#include "VideoChannel.h"
#include "JavaCallHelper.h"

extern  "C" {
#include <libavformat/avformat.h>
}

class MyPlayer {

public:
    MyPlayer(const char *path,JavaCallHelper *callHelper);
    ~MyPlayer();

    void prepare();
    void _prepare();

    int start();


private:
    char *path ;
    pthread_t pId;
    AVFormatContext *context ;

    AudioChannel *audioChannel ;
    VideoChannel *videoChannel ;
    JavaCallHelper *callHelper ;

};


#endif //NDKDEMO_MYPLAYER_H
