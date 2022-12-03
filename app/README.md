
https://blog.csdn.net/weixin_49406295/article/details/121483625

/* 结构体申明 */
/******************************************************************
* AVFormatContext               //加解封装，包含封装参数和AVStream
*    AVIOContext *pb;           //文件IO上下文
*    char filename[1024];       //文件名
*    unsigned int nb_streams;   //流元素个数，视频文件包含多少个流
*    AVStream **streams;        //音频和视频信息
*    int64_t duration;          //整个媒体文件长度
*    int64_t bit_rate;          //比特率，1s文件的大小
     ******************************************************************/

/****************************************************************************************
* AVStream                          //每一个流(音视频流)的参数信息，所有配置信息
*   AVCodecContext *codec;          //解封装和编码没做隔离，该参数已经过时，不使用
*   AVRational time_base;           //时间基数，用分数表示，为了保障精度
*   int64_t duration;               //时间段，该参数不一定有，以AVFormatContext里的为准
*   int64_t nb_frames;              //帧数
*   AVRational avg_frame_rate;      //帧率，用分数表示
*   AVCodecParameters *codecpar;    //音视频参数，用于替代AVCodecContext
    ****************************************************************************************/

/****************************************************************************************
* AVCodecParameters                 //音视频参数，存放在AVStream结构体里
*    enum AVMediaType codec_type;   //编码类型，音频还是视频
*    enum AVCodecID codec_id;       //编码格式
*    uint32_t codec_tag;            //编码器标签，用不到
*    int format;                    //像素格式或音频采样格式
*    int width; int height;         //音视频宽高
*    uint64_t channel_layout;       //声道
*    int channels; int sample_rate; int frame_size; //声道数，样本率，一帧音频大小
     ****************************************************************************************/

/****************************************************************************************
* AVPacket                     //具体的解封装完之后的数据包，用av_read_frame()函数读取数据包
* AVBufferRef *buf;         //指向的空间用来存储引用计数，代码中基本不用管
* int64_t pts;              //显示时间pts * (num / den) 乘以时间基数，值可能特别大
* int64_t dts;              //解码时间
* uint8_t *data; int size;  //数据缓存和数据大小，用ffmpeg提供的接口声请空间
     ****************************************************************************************/


原文链接：https://blog.csdn.net/irainsa/article/details/127676959
音频解码 + 播放：
1： 解码线程
    1.初始化FFmpge引擎、初始化链表、启动音频播放线程； 
    2.使用FFmpeg循环读取网络流中的数据包AVPacket，其数据格式为AAC(也可能为其他编码压缩格式)，然后再进行解码、重采样得到PCM数据； 
    3.将解码得到PCM数据插入到链表中，以等待播放线程读取播放；
    4.停止解码，释放资源。

2： 播放线程

    1.初始化播放链表； 
    2.启动初始化OPenSL ES线程，该线程主要是完成对OpenSL ES引擎的初始化，待初始化完毕后，会结束掉该线程。因为，OpenSL ES播放音频是通过回调函数的方式实现的，只需要循环读取PCM数据的线程即可； 
    3.循环读取PCM链表，并将读取的数据存储到播放链表中。
