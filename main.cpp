#include "mainwindow.h"
#include "xffmpeg.h"
#include <QApplication>
int main(int argc, char *argv[])
{
   if (xffmpeg::Get()->open("E:\\test\\fortest.mp4")) {//是否成功打开
    qDebug() << "open success";
   } else {
    qDebug() << "open failed";
    getchar();
    return -1;
    }
   for(;;) {
        AVPacket pkt = xffmpeg::Get()->read();//每次获取一帧
        if (pkt.size == 0) break;
        qDebug() << "pts = " << pkt.pts;
        AVFrame *yuv = xffmpeg::Get()->decode(&pkt);//解码视频帧
        if (yuv) {
            qDebug() << "[D]";
        }
        av_packet_unref(&pkt);//重置pkt为0
   }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
