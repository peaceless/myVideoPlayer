#include "xvideoThread.h"
#include "xffmpeg.h"
#include "video_widget.h"
#include "xaudio.h"
#include <QDebug>
#include <QList>
xvideoThread::xvideoThread()
{

}
xvideoThread::~xvideoThread()
{
    qDebug() << "excute ~xvideoThread";

}

void xvideoThread::run()
{
    bool diff = false;
    bool decode_flag;
    QList<AVPacket> buffer[2];//0 for video,1for audio
    while (!isexit) {//线程未退出
        decode_flag =false;
        if (flush) {
            buffer[0].clear();
            buffer[1].clear();
            flush = false;
            continue;
        }
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
        if (pkt.size <= 0) {//未打开视频
            msleep(10);
            continue;
        }
        if (pkt.stream_index == static_cast<int>(xffmpeg::Get()->audio_stream)) {
            diff = true;
        }else if(pkt.stream_index == static_cast<int>(xffmpeg::Get()->video_stream)) diff = false;
        else {
            av_packet_unref(&pkt);
            continue;}
        buffer[diff].push_back(pkt);//将缓冲压入对应队列中
        if (buffer[0].size() > 0 && buffer[1].size() > 0) {
            qDebug() << "buffer has been over o";
            if (xffmpeg::Get()->getPts(&pkt) - xffmpeg::Get()->getPts(&buffer[0].front()) > 0){
                qDebug() << "has buffered";
                xffmpeg::Get()->decode1(&buffer[1].front());
                xffmpeg::Get()->decode(&buffer[0].front());
                buffer[0].pop_front();
                buffer[1].pop_front();
            }
        }
//        if (pkt.stream_index == static_cast<int>(xffmpeg::Get()->audio_stream)) {
//            //为音频
//            xffmpeg::Get()->decode1(&pkt);
//            av_packet_unref(&pkt);//不为视频时释放pkt
//            continue;
//        }else continue;
//       qDebug() <<"get here" << pkt.stream_index;
//        xffmpeg::Get()->decode(&pkt);
//        av_packet_unref(&pkt);
//        if (xffmpeg::Get()->fps > 0) {//控制解码进度
//            msleep(static_cast<unsigned long>(1000/xffmpeg::Get()->fps) );
//        }
    }
    qDebug() << "thread destroyed";
}
