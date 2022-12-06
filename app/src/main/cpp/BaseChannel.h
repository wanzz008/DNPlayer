//
// Created by bytedance on 2022/11/22.
//

#ifndef DNPLAYER_BASECHANNEL_H
#define DNPLAYER_BASECHANNEL_H

#include "safe_queue.h"
#include "macro.h"

extern "C" {
#include <libavcodec/avcodec.h>
};

class BaseChannel {
public:
    BaseChannel(int id, AVCodecContext *pContext, AVRational time_base) : channelId(id),avCodecContext(pContext),
                                                                          time_base(time_base){
        packets.setReleaseCallback(releaseAvPacket);
    }

    ~BaseChannel(){
        packets.clear();
    }

    /**
    * 释放 AVPacket
    * @param packet
    */
    static void releaseAvPacket(AVPacket** packet) {
        if (packet) {
            av_packet_free(packet);
            //为什么用指针的指针？
            // 指针的指针能够修改传递进来的指针的指向
            *packet = 0;
        }
    }

    /**
    * 释放 AVPacket
    * @param packet
    */
    static void releaseAvFrame(AVFrame** frame) {
        if (frame) {
            av_frame_free(frame);
            //为什么用指针的指针？
            // 指针的指针能够修改传递进来的指针的指向
            *frame = 0;
        }
    }


    //纯虚方法 相当于 抽象方法
    virtual void play() = 0;

    int channelId ;
    bool isPlaying;
    //编码数据包队列
    SafeQueue<AVPacket *> packets;
    //解码数据包队列
    SafeQueue<AVFrame *> frames;

    AVCodecContext *avCodecContext;

    AVRational time_base ;

    double audio_clock;
};


#endif //DNPLAYER_BASECHANNEL_H
