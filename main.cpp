#include "mainwindow.h"
#include "xffmpeg.h"
#include <QApplication>
#include <QLabel>
#include <video_widget.h>
int main(int argc, char *argv[])
{
   // uint8_t *rgb = new uint8_t[800*600*4];
  // if (xffmpeg::Get()->open("E:\\test\\fortest.mp4")) {//是否成功打开
    //qDebug() << "open success";
   //} else {
    //qDebug() << "open failed";
    //getchar();
    //return -1;
    //}
//   for(;;) {
//        AVPacket pkt = xffmpeg::Get()->read();//每次获取一帧
//        if (pkt.size == 0) break;
//        qDebug() << "pts = " << pkt.pts;
//
//        if (pkt.stream_index != static_cast<int>(xffmpeg::Get()->video_stream)){//若不为视频
//            av_packet_unref(&pkt);//重置pkt
//            continue;
//        }
//        AVFrame *yuv = xffmpeg::Get()->decode(&pkt);//解码视频帧
//        if (yuv) {
//            qDebug() << "[D]";
//            xffmpeg::Get()->torgb(yuv,rgb,800,600);
//        }
//        av_packet_unref(&pkt);//重置pkt为0
//   }

    QApplication a(argc, argv);
    MainWindow w;
    video_widget v_w;
    v_w.show();
    w.show();

    return a.exec();
}
