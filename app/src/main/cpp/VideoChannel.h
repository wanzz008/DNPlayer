//
// Created by bytedance on 2022/11/18.
//

#ifndef DNPLAYER_VIDEOCHANNEL_H
#define DNPLAYER_VIDEOCHANNEL_H


#include "BaseChannel.h"
#include "AudioChannel.h"

extern "C"{
#include "libswscale/swscale.h"
}

typedef void (*RenderFrameCallback)(uint8_t *,int,int,int);

class VideoChannel : public BaseChannel{
public:
    VideoChannel(int id, AVCodecContext *pContext, int fps, AVRational rational);
    ~VideoChannel();

    void play();

    void decode();

    void render();

    void setRenderFrameCallback(RenderFrameCallback callback);

    void setAudioChannel(AudioChannel *audioChannel);

private:
    pthread_t pid_decode;
    pthread_t pid_render;

    SwsContext *swsContext=0;

    RenderFrameCallback callback ;

    int fps; //帧率

    AudioChannel *audioChannel = 0;
};


#endif //DNPLAYER_VIDEOCHANNEL_H
