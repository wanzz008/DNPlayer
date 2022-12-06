//
// Created by bytedance on 2022/11/18.
//

#ifndef DNPLAYER_AUDIOCHANNEL_H
#define DNPLAYER_AUDIOCHANNEL_H


#include "BaseChannel.h"
extern  "C"{
#include <libswresample/swresample.h>
};

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int id, AVCodecContext *pContext, AVRational time_base);
    ~AudioChannel();
    void play();

    void decode();

    void _play();

    int getPcm();

public:
    uint8_t *data = 0;
    int out_channels;
    int out_samplesize;
    int out_sample_rate;

private:

    //重采样
    SwrContext *swrContext = 0;

    pthread_t  pid_audio_decode;
    pthread_t  pid_audio_play;

    /**
     * OpenSL ES
     */
    // 引擎与引擎接口
    SLObjectItf engineObject = 0;
    SLEngineItf engineInterface = 0;
    //混音器
    SLObjectItf outputMixObject = 0;
    //播放器
    SLObjectItf bqPlayerObject = 0;
    //播放器接口
    SLPlayItf bqPlayerInterface = 0;

    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueueInterface =0;

};


#endif //DNPLAYER_AUDIOCHANNEL_H
