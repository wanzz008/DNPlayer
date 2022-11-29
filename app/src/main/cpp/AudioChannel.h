//
// Created by bytedance on 2022/11/18.
//

#ifndef DNPLAYER_AUDIOCHANNEL_H
#define DNPLAYER_AUDIOCHANNEL_H


#include "BaseChannel.h"

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int id, AVCodecContext *pContext);
    void play();
};


#endif //DNPLAYER_AUDIOCHANNEL_H
