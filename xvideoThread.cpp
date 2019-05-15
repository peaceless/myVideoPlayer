#include "xvideoThread.h"
#include "xffmpeg.h"
#include "video_widget.h"
#include "xaudio.h"
#include <QDebug>
#include <QList>
#include <QTime>
xvideoThread::xvideoThread()
{

}
xvideoThread::~xvideoThread()
{
    qDebug() << "excute ~xvideoThread";

}

int round(double fps,double pts)
{
    double x = pts / fps;
    qDebug() << "double x" <<x;
    return x-static_cast<int>(x) > 0.5 ? static_cast<int>(x+1):static_cast<int>(x);
}
void xvideoThread::run()
{
    bool diff = false;
    int decode_count = 1;
    int each = 0,deach = 0;
    QList<AVPacket> buffer[2];//0 for video,1for audio
    while (!isexit) {//线程未退出
        if (flush) {
            buffer[0].clear();
            buffer[1].clear();
            decode_count = 1;
            //video_count = 1;
            each = 0;
            deach = 0;
            flush = false;
            qDebug() << "flush all";
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
            if (buffer[0].size() > 0 || buffer[1].size() > 0) {
                //if buffer not empty
                // means video going to end
                break;
            }
            msleep(10);
            continue;
        }
//        qDebug() << "has read" << &pkt << pkt.size;
        if (pkt.stream_index == static_cast<int>(xffmpeg::Get()->video_stream)) {
            diff = false;
        }else if(pkt.stream_index == static_cast<int>(xffmpeg::Get()->audio_stream)) {
            //如果，缓冲区中有一秒音频
            diff = true;
            //当新读取的音频帧时间戳改变时 以秒为单位
            //从而计算每秒有多少帧音频帧
            if(!buffer[1].isEmpty()) {
//                qDebug() << "buffer for audio not null";
                if(xffmpeg::Get()->getPts(&pkt) == xffmpeg::Get()->getPts(&buffer[1].last()))
                {
                    decode_count ++;
//                qDebug() << "decode_count " << decode_count;
                } else {
                    qDebug() << xffmpeg::Get()->fps << decode_count;
                    each = round(xffmpeg::Get()->fps,decode_count);//static_cast<int>(decode_count / xffmpeg::Get()->fps);
                    //each 是每秒音频帧数量与视频帧数量的比值，即m解码多少音频帧解码一帧视频
//                qDebug() << "each is" << each;
                    if (each == 0) {
                        deach = round(decode_count,xffmpeg::Get()->fps);
//                    qDebug() << "deach is " << deach;
                    }
//reset decode_count
//                qDebug() << "reset decode_count";
                    decode_count = 1;
                }
            }
        } else {
            av_packet_unref(&pkt);
            continue;
        }
        buffer[diff].push_back(pkt);//将缓冲压入对应队列中
//        qDebug() << xffmpeg::Get()->getPts(&pkt) << "pkt pts is";
        if (buffer[0].size() > 0 && buffer[1].size() > 0) {
            qDebug() << "buffer has been over o";
            //两段缓冲区中都有一秒以上的缓冲
//            if(time.isNull()) time.start();
            int64_t p = xffmpeg::Get()->getPts(&buffer[0].last()) - xffmpeg::Get()->getPts(&buffer[0].front());
            int64_t q = xffmpeg::Get()->getPts(&buffer[1].last()) - xffmpeg::Get()->getPts((&buffer[1].front()));
            qDebug() << (p && q);
            if ( p && q) {
            //计算每秒有多少帧音频，从而每解码一定音频帧解码一帧视频
                //qDebug() << "buffer has been fulled  up";
                if (each != 0) {
                    for (int i = 0;i < each;i++) {
                        xffmpeg::Get()->decode1(&buffer[1].front());
                        buffer[1].pop_front();
                    }
                    xffmpeg::Get()->decode(&buffer[0].front());
                    buffer[0].pop_front();
                } else {
                    for (int i = 0;i<deach;i++) {
                        xffmpeg::Get()->decode(&buffer[0].front());
                        buffer[0].pop_front();
                    }
                    xffmpeg::Get()->decode1(&buffer[1].front());
                    buffer[1].pop_front();
                }
            }
          while(xffmpeg::Get()->getPts(&buffer[1].front()) > xffmpeg::Get()->getPts(&buffer[0].front())) {
                xffmpeg::Get()->decode(&buffer[0].front());
                buffer[0].pop_front();
            }
           while(xffmpeg::Get()->getPts(&buffer[0].front()) > xffmpeg::Get()->getPts(&buffer[1].front())){
               if (xaudio::get()->getFree() < 10000) {
                   msleep(1);
                   continue;
               }
               xffmpeg::Get()->decode1(&buffer[1].front());
               buffer[1].pop_front();
           }
        }
    }
    int i = buffer[0].size();
    int j = buffer[1].size();
    while (i) {
        qDebug() << xffmpeg::Get()->decode(&buffer[0].front());
        buffer[0].pop_front();
        i--;
        msleep(static_cast<unsigned long>(1000 / xffmpeg::Get()->fps));
    }
    qDebug() << xffmpeg::Get()->decode(nullptr);
    while(j) {
        if (xaudio::get()->getFree() < 10000) {
            msleep(1);
            continue;
        }
        qDebug() << xffmpeg::Get()->decode1(&buffer[1].front());
        buffer[1].pop_front();
        j--;
    }
   qDebug() << "thread destroyed";
}

