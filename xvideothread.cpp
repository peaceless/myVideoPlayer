#include "xvideothread.h"
#include "xffmpeg.h"
#include "video_widget.h"
bool isexit = false;//线程未退出
xvideoThread::xvideoThread()
{

}
xvideoThread::~xvideoThread()
{

}

void xvideoThread::run()
{
    while (!isexit) {//线程未退出
        if (!xffmpeg::Get()->isPlay) {
            msleep(10);
            continue;
        }
        AVPacket pkt = xffmpeg::Get()->read();
        if (pkt.size <= 0) {//未打开视频
            msleep(10);
            continue;
        }
    if (pkt.stream_index != static_cast<int>(xffmpeg::Get()->video_stream)) {
        //不为视频
        av_packet_unref(&pkt);//不为视频时释放pkt
        continue;
    }
    yuv =  xffmpeg::Get()->decode(&pkt);
    av_packet_unref(&pkt);
    if (xffmpeg::Get()->fps > 0) {//控制解码进度
        msleep(static_cast<unsigned long>(1000/xffmpeg::Get()->fps) );}
    }
}
