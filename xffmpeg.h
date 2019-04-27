#ifndef XFFMPEG_H
#define XFFMPEG_H
#include <iostream>
#include <QMutex>
extern "C" {
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}
class xffmpeg
{
public:
    static xffmpeg *Get() { //单例single
        static xffmpeg ff;
        return &ff;
    }
    bool open(const char *path);//打开视频文件open file
    void close();//关闭文件close file
    std::string get_error();//获取错误信息
    virtual  ~xffmpeg();
    long long total_sec = 0;//总时长total seconds


    AVPacket read();
    AVFrame *decode(const AVPacket *pkt);
    AVFrame *yuv = nullptr;
    unsigned int video_stream = 0;
    bool torgb(const AVFrame *yuv,uint8_t  out[],int outwidth,int outheight);
    SwsContext *sct = nullptr;//转码器上下文
    AVCodecContext *codec_ctx = nullptr;
protected:
    char errorbuff[1024];//打开时产生的错误信息
    xffmpeg();
    QMutex mutex;//互斥变量，多线程时避免同时间的读写
    AVFormatContext *avfc = nullptr;
};

#endif // XFFMPEG_H
