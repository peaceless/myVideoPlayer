#include "xffmpeg.h"
#include "xaudio.h"
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
    re = avformat_find_stream_info(avfc,nullptr);
    if (re < 0) {
        qDebug() << "find stream info failed";
        mutex.unlock();
        return false;
    }
   //解码器
    for (unsigned int i = 0;i < avfc->nb_streams;i++) {
        AVCodecParameters *avcp = avfc->streams[i]->codecpar;//解码上下文
        qDebug() << "streams number "<< i;
        if (avcp->codec_type == AVMEDIA_TYPE_VIDEO) {//判断文件是否为视频
            video_stream = i;
            fps = r2d(avfc->streams[i]->avg_frame_rate);
            AVCodec *codec = avcodec_find_decoder(avcp->codec_id); //查找解码器
            if (!codec) {//未找到解码器
                mutex.unlock();
                qDebug() << "video code not find";
                return false;
            }
            codec_ctx = avcodec_alloc_context3(nullptr);//需要使用avcodec_free_context释放
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
        }else if(avcp->codec_type == AVMEDIA_TYPE_AUDIO) {//若为音频流
            qDebug() << "audio process " << i;
            audio_stream = i;//音频流编号
            AVCodec *codec = avcodec_find_decoder(avcp->codec_id);
            if(!codec) {
                mutex.unlock();
                qDebug() << "audio decoder not find";
            }
            codec_ctx_audio = avcodec_alloc_context3(nullptr);
            avcodec_parameters_to_context(codec_ctx_audio,avcp);
            if (avcodec_open2(codec_ctx_audio,codec,nullptr) != 0) {
                mutex.unlock();
                qDebug() << "decoder failed";
                return -1;
            }
            qDebug() << "open audio decoder success" << codec_ctx_audio->codec_id;
            this->sampleRate = codec_ctx_audio->sample_rate;
            this->channel = codec_ctx_audio->channels;
            switch(codec_ctx_audio->sample_fmt) {
            case AV_SAMPLE_FMT_S16:
                this->sampleSize = 16;
                break;
            case AV_SAMPLE_FMT_S32:
                this->sampleSize = 32;
                break;
            case AV_SAMPLE_FMT_FLTP:
                this->sampleSize = 32;
                break;
            default:
                qDebug() << "not on ouur" << codec_ctx_audio->sample_fmt << avcp->sample_rate << avcp->channels;
                break;
            }
            qDebug() << "audio sampleRate :" << sampleRate <<"sampleSize :" <<sampleSize
                     << "channel:" <<channel;
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
    if (pcm) av_frame_free(&pcm);
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

long long xffmpeg::decode1(const AVPacket *pkt)
{
    mutex.lock();
    long long time = -1;
    if(!avfc) {
        mutex.unlock();
        return -1;
    }
    //AVFrame * temp = av_frame_alloc();
    if (pcm == nullptr) {
        pcm = av_frame_alloc();
    }
    bool need_new;
    int re = avcodec_send_packet(codec_ctx_audio,pkt);
    if (re != 0){
        qDebug() << "send audio packet wrong";
        mutex.unlock();
        return -1;
    }
    while(1) {
        need_new = false;
        qDebug() << "try to receive a audio frame";
        int re = avcodec_receive_frame(codec_ctx_audio,pcm);
        if (re != 0){
            if (re == AVERROR(EAGAIN)) {
                need_new = true;
                break;
            }
        }else {
            qDebug() << "get a audio frame" <<pcm << pcm->channels <<
                        pcm->nb_samples;
            int bufsize = av_samples_get_buffer_size(nullptr,xffmpeg::Get()->pcm->channels,
                                                      xffmpeg::Get()->pcm->nb_samples,
                                                      AV_SAMPLE_FMT_S16,0);
             qDebug() << "this step";
             if (bufsize >=0 ){
                 qDebug() << "bufsize"<< bufsize;
                 char *buf = new char[bufsize];
             int len = toPcm(buf);
             xaudio::get()->write(buf,len);
             time = static_cast<long long>(pcm->pts * r2d(avfc->streams[pkt->stream_index]->time_base));
             qDebug() << "audio time is" << time;
             delete[] buf;
             }
            continue;
        }
    }
    mutex.unlock();
    return time;
}
int64_t xffmpeg::getPts(const AVPacket *pkt)
{
    return  static_cast<int64_t>(pkt->pts * r2d(avfc->streams[pkt->stream_index]->time_base));
}
long long xffmpeg::decode(const AVPacket *pkt)
{
    mutex.lock();
    if (!avfc) {//若未打开视频
        mutex.unlock();
        return -1;
    }
    if (yuv == nullptr) {//申请解码的对象空间
        yuv = av_frame_alloc();
    }
    qDebug() << codec_ctx->codec_id << "codec_id is ";
    qDebug() << "pkt size is " << pkt->size;
    int re = avcodec_send_packet(codec_ctx,pkt);
    if (re != 0) {
        mutex.unlock();
        qDebug() << "send_packet wrong!!" << re;
        return -1;
    }
    re = avcodec_receive_frame(codec_ctx,yuv);//解码pkt
    if (re != 0) {
        mutex.unlock();
        return -1;
    }
    mutex.unlock();
    long long p = static_cast<long long>(yuv->pts*r2d(avfc->streams[pkt->stream_index]->time_base));
    return p;
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

int xffmpeg::toPcm(char out[])
{
    qDebug() << "to pacm";
    if (!avfc || !pcm || !out) {
        return 0;
    }
    if (aCtx == nullptr) {
        aCtx = swr_alloc();
        swr_alloc_set_opts(aCtx,static_cast<int64_t>(codec_ctx_audio->channel_layout),
                           AV_SAMPLE_FMT_S16,
                           codec_ctx_audio->sample_rate,
                           codec_ctx_audio->channels,
                           static_cast<AVSampleFormat>(codec_ctx_audio->sample_fmt),
                           codec_ctx_audio->sample_rate,0,nullptr);
        swr_init(aCtx);
    }
    uint8_t *data[1] = {nullptr};
    data[0] = (uint8_t *)out;
    qDebug() << "pcm->nb_sample" << pcm->data[0];
    int len = swr_convert(aCtx,
                          data,
                          10000,
                          (const uint8_t **)pcm->data,
                          pcm->nb_samples);
    if (len <= 0){
        qDebug() << len << "si nel";
        return 0;
    }
    int outsize = av_samples_get_buffer_size(nullptr,codec_ctx_audio->channels,pcm->nb_samples,
                                             AV_SAMPLE_FMT_S16,0);
    return outsize;
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
    avcodec_flush_buffers(codec_ctx_audio);
    mutex.unlock();
    if (re > 0) return true;
    return false;
}
