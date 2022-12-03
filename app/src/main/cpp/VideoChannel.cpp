//
// Created by bytedance on 2022/11/18.
//

#include "VideoChannel.h"

extern "C"{
#include <libavutil/imgutils.h>
}

VideoChannel::VideoChannel(int id, AVCodecContext *pContext) : BaseChannel(id, pContext) {
    frames.setReleaseCallback(releaseAvFrame);
}

VideoChannel::~VideoChannel() {
    frames.clear();
}

void* decode_task(void *args){
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->decode();
    return 0;
}
void* render_task(void *args){
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->render();
    return 0;
}

/**
 * 视频的解码 + 渲染
 */
void VideoChannel::play() {
    isPlaying = 1 ;

    //设置为工作状态
    frames.setWork(1);
    packets.setWork(1);

    // 1. 解码
    pthread_create(&pid_decode,0,decode_task, this);
    // 2. 渲染
    pthread_create(&pid_render,0,render_task, this);

}
/**
 * 解码，从队列取出avpacket，进行解码成avframe，再送进一个队列
 */
void VideoChannel::decode() {
    LOGE("VideoChannel: decode线程.....");
    AVPacket *packet = 0;
    while (isPlaying){
        int ret = packets.pop(packet);
        if (!isPlaying){  //pop可能是个耗时操作
            break;
        }
        // 取出失败
        if (!ret){
            continue;
        }
//        LOGE("videoChannel：_decode.....   从队列取出avpacket进行解码");
        //把包丢给解码器
        ret = avcodec_send_packet(avCodecContext,packet);
        releaseAvPacket(&packet);
        //重试
        if (ret != 0){
            break;
        }
        //代表了一个图像 (将这个图像先输出来)
        AVFrame *frame = av_frame_alloc();
        //从解码器中读取 解码后的数据包 AVFrame
        ret = avcodec_receive_frame(avCodecContext,frame);
        //需要更多的数据才能够进行解码
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if(ret != 0){
            break;
        }
        // 再开一个线程 进行播放
        frames.push(frame);

    }
    releaseAvPacket(&packet);
}
/**
 * 渲染， 从队列取出解码后的avframe，格式转换后，进行渲染
 */
void VideoChannel::render() {
    LOGE("VideoChannel: render线程.....");
//    int srcW, int srcH, enum AVPixelFormat srcFormat,
//    int dstW, int dstH, enum AVPixelFormat dstFormat,
//    int flags, SwsFilter *srcFilter,
//            SwsFilter *dstFilter, const double *param
    //目标： RGBA
    swsContext = sws_getContext(
            avCodecContext->width,avCodecContext->height,avCodecContext->pix_fmt,
            avCodecContext->width,avCodecContext->height,AV_PIX_FMT_RGBA,
            SWS_BILINEAR,0,0,0
                                );
    AVFrame* frame = 0;
    //指针数组
    uint8_t *dst_data[4];
    int dst_linesize[4];
    av_image_alloc(dst_data, dst_linesize,
                   avCodecContext->width, avCodecContext->height,AV_PIX_FMT_RGBA, 1);
    while (isPlaying){
        frames.pop(frame);
        if (!isPlaying){
            break;
        }
        //src_linesize: 表示每一行存放的 字节长度
        sws_scale(swsContext, reinterpret_cast<const uint8_t *const *>(frame->data),
                  frame->linesize, 0,
                  avCodecContext->height,
                  dst_data,
                  dst_linesize);
        //回调出去进行播放
        callback(dst_data[0],dst_linesize[0],avCodecContext->width, avCodecContext->height);
        releaseAvFrame(&frame) ;
    }
    av_freep(&dst_data[0]);
    releaseAvFrame(&frame) ;

}

void VideoChannel::setRenderFrameCallback(RenderFrameCallback callback) {
    this->callback = callback ;
}


