#include "xffmpeg.h"
#include <QDebug>
xffmpeg::xffmpeg()
{
    errorbuff[0] = '\0';
    //av_register_all();
}

xffmpeg::~xffmpeg()
{
}

bool xffmpeg::open(const char *path)
{
    close();//打开前先关闭清理
    mutex.lock();//上锁
    int re = avformat_open_input(&avfc,path,nullptr,nullptr);//打开解封装
    if (re != 0) {//打开解封装器错误时
        mutex.unlock();//失败，释放锁
        av_strerror(re,errorbuff,sizeof(errorbuff));
        qDebug() << "open file failed :" << path << "::" << errorbuff;//输出错误信息
        return false;
    }
   //解码器
    for (unsigned int i = 0;i < avfc->nb_streams;i++) {
        AVCodecParameters *avcp = avfc->streams[i]->codecpar;//解码上下文
        if (avcp->codec_type == AVMEDIA_TYPE_VIDEO) {//判断文件是否为视频
            video_stream = i;
            AVCodec *codec = avcodec_find_decoder(avcp->codec_id); //查找解码器
            if (!codec) {//未找到解码器
                mutex.unlock();
                qDebug() << "video code not find";
                return false;
            }
            AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);//需要使用avcodec_free_context释放
            avcodec_parameters_to_context(codec_ctx,avcp);//从avcodecparameters复制到avcodeccontext
            //视频的帧率应该从AVStream结构的avg_frame_rate成员获取，而非a'vCOdecContext结构体
            //如果avg_frame_rate为{0，1}，再尝试从r_frame_rate成员获取
            int err = avcodec_open2(codec_ctx,codec,nullptr);
        }
    }
    total_sec = avfc->duration / (AV_TIME_BASE);
    qDebug() << "file total seconds is " << total_sec /60 <<"--" << total_sec % 60;
    mutex.unlock();
    return true;
}

void xffmpeg::close()
{
    mutex.lock();//上锁，防止多线程中一个线程中关闭，另外一个线程在读取
    if (avfc) {avformat_close_input(&avfc);}
    if (yuv) av_frame_free(&yuv);//关闭时释放解码后的视频空间
    mutex.unlock();
}

std::string xffmpeg::get_error()
{
    mutex.lock();
    std::string re = this->errorbuff;
    mutex.unlock();
    return re;
}

AVPacket xffmpeg::read()
{
    AVPacket pkt;
    memset(&pkt,0,sizeof(AVPacket));
    mutex.lock();
    if (!avfc) {
        mutex.unlock();
        return pkt;
    }
    int err = av_read_frame(avfc,&pkt);
    if (err != 0) { //读取视频失败
        av_strerror(err,errorbuff,sizeof (errorbuff));
    }
    mutex.unlock();
    return pkt;
}

AVFrame * xffmpeg::decode(const AVPacket *pkt)
{
    mutex.lock();
    if (!avfc) {//若未打开视频
        mutex.unlock();
        return nullptr;
    }
    if (yuv == nullptr) {//申请解码的对象空间
        yuv = av_frame_alloc();
    }
    int re = avcodec_send_packet(avfc->streams[pkt->stream_index]->codec,pkt);
    if (re != 0) {
        mutex.unlock();
        return nullptr;
    }
    re = avcodec_receive_frame(avfc->streams[pkt->stream_index]->codec,yuv);//解码pkt
    if (re != 0) {
        mutex.unlock();
        return nullptr;
    }
    mutex.unlock();
    return yuv;
}

bool xffmpeg::torgb(const AVFrame *yuv, char *out, int outwidth, int outheight)
{
    mutex.lock();
    if (!avfc) {//未打开视频文件
        mutex.unlock();
        return false;
    }
    AVCodecParameters *videoCtx = avfc->streams[this->video_stream]->codecpar;
}
