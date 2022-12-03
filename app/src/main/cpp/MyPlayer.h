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

    void setRenderFrameCallback(RenderFrameCallback callback);

    void _play();

private:
    char *path ;
    pthread_t pId;
    pthread_t pid_play;

    AVFormatContext *context = 0 ;

    AudioChannel *audioChannel = 0;
    VideoChannel *videoChannel = 0;
    JavaCallHelper *callHelper ;
    bool isPlaying; // 是否在播放

    RenderFrameCallback callback; //回调
};


#endif //NDKDEMO_MYPLAYER_H
