//
// Created by bytedance on 2022/11/17.
//

#include "MyPlayer.h"
#include <string>
#include <string.h>
#include <jni.h>
#include "macro.h"


MyPlayer::MyPlayer(const char *path_, JavaCallHelper *callHelper) {

//    this->path = path_;
    this->callHelper = callHelper;

    this->path = new char[strlen(path_) + 1];
//    //防止 dataSource参数 指向的内存被释放
    strcpy(this->path,path_);

}

MyPlayer::~MyPlayer() {
    delete path;
    path = 0;
    delete callHelper ;
}

void *prepare_task(void *args) {
    MyPlayer *player = static_cast<MyPlayer *>(args);
    player->_prepare();
    return 0;
}

void MyPlayer::prepare() {
    pthread_create(&pId, 0, prepare_task, this);
}
/**
 * 准备工作： 解封装
 */
void MyPlayer::_prepare() {
    avformat_network_init();
    //1、打开媒体地址(文件地址、直播地址)
    int ret = avformat_open_input(&context, path, 0, 0);
    LOGE("打开媒体地址为: %s",path);
    if (ret != 0) {
        LOGE("打开媒体失败:%s",av_err2str(ret));
        callHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        return;
    }
    //2、查找媒体中的 音视频流 (给 contxt里的 streams等成员赋)
    ret = avformat_find_stream_info(context, 0);
    if (ret < 0) {
        LOGE("查找流失败:%s",av_err2str(ret));
        callHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        return;
    }
    //nb_streams :几个流(几段视频/音频)
    for (int i = 0; i < context->nb_streams; ++i) {
        //可能代表是一个视频 也可能代表是一个音频
        AVStream *stream = context->streams[i];
        //包含了 解码 这段流 的各种参数信息(宽、高、码率、帧率)
        AVCodecParameters *parameters = stream->codecpar;

        //无论视频还是音频都需要干的一些事情（获得解码器）
        // 1、通过 当前流 使用的 编码方式，查找解码器
        AVCodec *codec = avcodec_find_decoder(parameters->codec_id);
        if (codec == NULL) {
            LOGE("查找解码器失败:%s", av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            return;
        }

        //2、获得解码器上下文
        AVCodecContext *avCodecContext = avcodec_alloc_context3(codec);
        if (avCodecContext == NULL) {
            LOGE("创建解码上下文失败:%s", av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }
        //3、设置上下文内的一些参数 (context->width)
        //        avCodecContext->width = parameters->width;
        //        avCodecContext->height = parameters->height;
        ret = avcodec_parameters_to_context(avCodecContext, parameters);
        if (ret < 0) {
            LOGE("设置解码上下文参数失败:%s", av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }
        // 4、打开解码器
        ret = avcodec_open2(avCodecContext, codec, 0);
        if (ret != 0) {
            LOGE("打开解码器失败:%s",av_err2str(ret));
            callHelper->onError(THREAD_CHILD,FFMPEG_OPEN_DECODER_FAIL);
        }
        if (parameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioChannel = new AudioChannel(i, avCodecContext);
            LOGE("audioChannel音频轨道---------");
        } else if (parameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoChannel = new VideoChannel(i, avCodecContext);
            videoChannel->setRenderFrameCallback(callback);
            LOGE("videoChannel视频轨道---------");
        }

    }

    //没有音视频  (很少见)
    if (!audioChannel && !videoChannel) {
        LOGE("没有音视频");
        callHelper->onError(THREAD_CHILD,FFMPEG_NOMEDIA);
        return;
    }

    // 准备完了 通知java 你随时可以开始播放
    callHelper->onPrepare(THREAD_CHILD);

}

void *play(void *args) {
    MyPlayer *player = static_cast<MyPlayer *>(args);
    player->_play();
    return 0;
}

/**
 * 解码
 * @return
 */
int MyPlayer::start() {
    isPlaying = 1 ;

    if (videoChannel){
        LOGE("start..... videoChannel不为空");
        videoChannel->play();
    } else{
        LOGE("start..... videoChannel为空");
    }
    // 读取数据，填充avpacket队列
    pthread_create(&pid_play,0,play,this);


    return 0;
}

/**
 * 专门读取数据包
 */
void MyPlayer::_play() {
    LOGE("_play 读取avpacket数据并送进队列.....");
    //1、读取媒体数据包(音视频数据包)
    int ret ;
    while (isPlaying){
        AVPacket *packet = av_packet_alloc();
        ret = av_read_frame(context,packet);
        if (ret == 0){
            if (audioChannel && packet->stream_index == audioChannel->channelId){
//                LOGE("_play.....audioChannel----向队列输送avpacket");
            } else if(videoChannel && packet->stream_index == videoChannel->channelId){
//                LOGE("_play.....videoChannel----向队列输送avpacket");
                videoChannel->packets.push(packet);
            }
        }else if (ret == AVERROR_EOF) {
            //读取完成 但是可能还没播放完

        } else {
            //
        }
    }
}


void MyPlayer::setRenderFrameCallback(RenderFrameCallback callback) {
    this->callback = callback ;
}



