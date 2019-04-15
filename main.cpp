#include "mainwindow.h"
#include <QApplication>
int main(int argc, char *argv[])
{
    av_register_all(); //注册ffmpeg的库

    char *path = "E:\test\[Nekomoenai.sub][Kimi no Suizou wo Tabetai][Movie][1080p][CHS].mp4";
    AVFormatContext *ic = NULL;//解封装上下文
    int re = avformat_open_input(&ic,path,0,0);
    if (re != 0) {//打开错误时
        char buf[1024] = {0};
        av_strerror(re,buf,sizeof(buf));
        printf("open %s failed :%s\n",path,buf);
        getchar();
        return -1;
    }
    int total_sec = ic->duration / (AV_TIME_BASE);//获取视频时常
    printf("file totalsec is %d-%d\n",total_sec/60,total_sec%60);
    int video_stream = 0;
    for (int i = 0;i < ic->nb_streams;i++) {
        AVCodecContext *enc = ic->streams[i]->codec;//解码上下文
        if (enc->codec_type == AVMEDIA_TYPE_VIDEO) {//判断是否为视频
            video_stream = i;
            AVCodec *codec = avcodec_find_decoder(enc->codec_id);//查找解码器
            if (!codec) {//未找到解码器
                printf("video code not find\n");
                return -1;
            }
            int err = avcodec_open2(enc,codec,NULL);//打开解码器
            if (err != 0) {//未打开解码器
                char buf[1024] = {0};
                av_strerror(err,buf,sizeof(buf));
                printf(buf);
                return -2;
            }
            printf("open codec success!\n");
        }
    }
    //读取并分析视频包
    AVFrame *yuv = av_frame_alloc();
    for (;;) { //读取视频中的所有帧
        AVPacket pkt;
        re = av_read_frame(ic,&pkt);//读取解封装后的数据
        if (re != 0) break;
        if (pkt.stream_index != video_stream) continue;
        int pts = pkt.pts * av_q2d(ic->streams[pkt.stream_index]->time_base) * 1000;
        int got_picture = 0;
        int re = avcodec_decode_video2(re,yuv,&got_picture,&pkt)
        printf("pts = %d",pkt.pts);//打印pts，显示时间戳，用于同步
        av_packet_unref(&pkt);
    }
    avformat_close_input(&ic);//释放解封装器的空间，以防空间被快速消耗
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
