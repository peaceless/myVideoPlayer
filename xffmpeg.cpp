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

static double r2d(AVRational r);
long long xffmpeg::open(const char *path)
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
            fps = r2d(avfc->streams[i]->avg_frame_rate);
            AVCodec *codec = avcodec_find_decoder(avcp->codec_id); //查找解码器
            if (!codec) {//未找到解码器
                mutex.unlock();
                qDebug() << "video code not find";
                return false;
            }
            codec_ctx = avcodec_alloc_context3(codec);//需要使用avcodec_free_context释放
            avcodec_parameters_to_context(codec_ctx,avcp);//从avcodecparameters复制到avcodeccontext
            //视频的帧率应该从AVStream结构的avg_frame_rate成员获取，而非avCOdecContext结构体
            //如果avg_frame_rate为{0，1}，再尝试从r_frame_rate成员获取
            int err = avcodec_open2(codec_ctx,codec,nullptr);
            if (err != 0) { //未打开解码器
                mutex.unlock();
                char buf[1024] = {0};
                av_strerror(err,buf,sizeof(buf));
                qDebug() << buf;
                return -1;
            }
            qDebug() << "open codec success";
        }
    }
    total_sec = avfc->duration / (AV_TIME_BASE);
    qDebug() << "file total seconds is " << total_sec /60 <<"-" << total_sec % 60;
    mutex.unlock();
    return total_sec;
}
static double r2d(AVRational r)
{
    return r.den == 0 ? 0:static_cast<double>(r.num) / static_cast<double>(r.den);
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
//    AVCodecParameters *avcp = avfc->streams[pkt->stream_index]->codecpar;
//    AVCodecContext *avcc = avcodec_alloc_context3(nullptr);
//    avcodec_parameters_to_context(avcc,avcp);
    int re = avcodec_send_packet(codec_ctx,pkt);
    if (re != 0) {
        mutex.unlock();
        qDebug() << "send_packet wrong!!" << re;
        return nullptr;
    }
    re = avcodec_receive_frame(codec_ctx,yuv);//解码pkt
    if (re != 0) {
        mutex.unlock();
        return nullptr;
    }
    mutex.unlock();
    pts = static_cast<long long>(yuv->pts*r2d(avfc->streams[pkt->stream_index]->time_base));
    return yuv;
}

bool xffmpeg::torgb(const AVFrame *yuv, uint8_t out[], int outwidth, int outheight)
{
    mutex.lock();
    if (!avfc) {//未打开视频文件
        mutex.unlock();
        return false;
    }
    sct = sws_getCachedContext(sct,yuv->width,//初始化一个swscontext
                               yuv->height,
                               static_cast<AVPixelFormat>(yuv->format),//输入像素格式
                               outwidth,outheight,
                               AV_PIX_FMT_BGRA,//输出像素格式
                               SWS_FAST_BILINEAR,//转码的算法
                               nullptr,nullptr,nullptr);
    if (!sct) {
        mutex.unlock();
        return false;
    }
    uint8_t * data[AV_NUM_DATA_POINTERS] = {nullptr};
    data[0] = out;//第一位输出RGB
    int line_size[AV_NUM_DATA_POINTERS] = {0};

    line_size[0] = outwidth * 4;//一行的宽度，32位4个字节
    sws_scale(sct,yuv->data,//当前处理区域的每个通道数据指针
                      yuv->linesize,//每个通道行字节数
                      0,yuv->height,//原视频高度
                      data,//输出：：的每个通道数据指针
                      line_size//每个通道行字节数
                      );//开始转码
    mutex.unlock();
    return true;
}

bool xffmpeg::seek(float pos)
{
    mutex.lock();
    if (!avfc) {//未打开视频
        mutex.unlock();
        return false;
    }
    int64_t stamp = 0;
    stamp = static_cast<int64_t>(pos * avfc->streams[video_stream]->duration);//当前实际位置
    pts = static_cast<int64_t>(stamp * r2d(avfc->streams[video_stream]->time_base));
    int re = av_seek_frame(avfc,static_cast<int>(video_stream),stamp,
                           AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);//刷新缓冲
    avcodec_flush_buffers(codec_ctx);
    mutex.unlock();
    if (re > 0) return true;
    return false;
}
