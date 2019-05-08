#include "xvideothread.h"
#include "xffmpeg.h"
#include "video_widget.h"
#include "xaudio.h"
#include <QDebug>
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
        int free = xaudio::get()->getFree();
        if (free < 10000) {
            msleep(1);
            continue;
        }
        AVPacket pkt = xffmpeg::Get()->read();
    qDebug() << "ready for video";
        if (pkt.size <= 0) {//未打开视频
            msleep(10);
            continue;
        }
    if (pkt.stream_index != static_cast<int>(xffmpeg::Get()->video_stream)) {
        //为音频
//        xffmpeg::Get()->decode(&pkt);
        qDebug() << xffmpeg::Get()->pcm << xffmpeg::Get()->yuv;
        xffmpeg::Get()->decode1(&pkt);
          av_packet_unref(&pkt);//不为视频时释放pkt
          qDebug() << "why why why";
       //   if (xffmpeg::Get()->pcm == nullptr) {continue;}
       // int bufsize = av_samples_get_buffer_size(nullptr,xffmpeg::Get()->pcm->channels,
       //                                          xffmpeg::Get()->pcm->nb_samples,
       //                                          AV_SAMPLE_FMT_S16,0);
       // qDebug() << "this step";
       // if (bufsize >=0 ){
       //     qDebug() << "bufsize"<< bufsize;
       //     char *buf = new char[bufsize];
       // int len = xffmpeg::Get()->toPcm(buf);
       // xaudio::get()->write(buf,len);
       // delete[] buf;
       // }
        continue;
    }
    qDebug() <<"get here" << pkt.stream_index;
    xffmpeg::Get()->decode(&pkt);
    av_packet_unref(&pkt);
    if (xffmpeg::Get()->fps > 0) {//控制解码进度
        msleep(static_cast<unsigned long>(1000/xffmpeg::Get()->fps) );}
    }
}
