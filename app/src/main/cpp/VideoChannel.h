//
// Created by bytedance on 2022/11/18.
//

#ifndef DNPLAYER_VIDEOCHANNEL_H
#define DNPLAYER_VIDEOCHANNEL_H


#include "BaseChannel.h"
extern "C"{
#include "libswscale/swscale.h"
}

typedef void (*RenderFrameCallback)(uint8_t *,int,int,int);

class VideoChannel : public BaseChannel{
public:
    VideoChannel(int id, AVCodecContext *pContext);
    ~VideoChannel();

    void play();

    void decode();

    void render();

    void setRenderFrameCallback(RenderFrameCallback callback);

private:
    pthread_t pid_decode;
    pthread_t pid_render;

    SwsContext *swsContext=0;
    SafeQueue<AVFrame *> frames;

    RenderFrameCallback callback ;

};


#endif //DNPLAYER_VIDEOCHANNEL_H
