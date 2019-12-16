//
//   author: patrick.dai
// datetime: 2019-12-02 19:28
//

#include "FFDecode.h"
#include "XLog.h"

extern "C" {
#include "libavcodec/avcodec.h"
}



bool FFDecode::open(XParameter parameter)
{
    if(!parameter.para)
    {
        return false;
    }

    AVCodecParameters *p = parameter.para;

    // 1. 查找解码器
    AVCodec *av_codec = avcodec_find_decoder(p->codec_id);
    if(!av_codec)
    {
        XLOGE("ffmepg find decoder %d failed.", p->codec_id);
        return false;
    }

    XLOGI("ffmpeg find decoder %d success.", p->codec_id);

    // 2 创建解码上下文  复制参数
    codec = avcodec_alloc_context3(av_codec);
    avcodec_parameters_to_context( codec, p);
    codec->thread_count = 8;

    // 3 打开解码器
    int ret = avcodec_open2(codec, nullptr, nullptr);
    if(ret != 0)
    {
        char buffer[1024] = {0};
        av_strerror(ret, buffer, sizeof(buffer) - 1);
        XLOGE("ffmepg codec open2 failed: %s", buffer);
        return false;
    }

    XLOGI("ffmpeg code open2 success");

    // video type
    if(codec->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        this->type = data_type::DATA_VIDEO;
    }
    // audio type
    else if(codec->codec_type == AVMEDIA_TYPE_AUDIO)
    {
        this->type = data_type::DATA_AUDIO;
    }
    // drop unknown type
    else
    {
        this->type = data_type::UNKNOWN;
    }

    return true;
}


bool FFDecode::send_packet(XData pkt)
{
    if(!codec)
    {
        return false;
    }

    if(pkt.size <= 0 || !pkt.data)
    {
        return false;
    }

    int ret = avcodec_send_packet(codec, (AVPacket *)pkt.data);
    // send failed
    if(ret != 0)
    {
        return false;
    }

    return true;
}

XData FFDecode::receive_frame()
{
    if(!codec)
    {
        return XData();
    }

    if(!frame)
    {
        frame = av_frame_alloc();
    }

    int ret = avcodec_receive_frame(codec, frame);
    // decode failed
    if(ret != 0)
    {
        return XData();
    }

    // decode success
    // TODO
    XData data;
    data.data = (unsigned char *)frame;
    // video data
    if(codec->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        data.size += frame->linesize[0];
        data.size += frame->linesize[1];
        data.size += frame->linesize[2];

        data.width  = frame->width;
        data.height = frame->height;
    }
    // audio data
    else if(codec->codec_type == AVMEDIA_TYPE_AUDIO)
    {
        // 样本字节数 * 单通道样本数 * 通道数
        data.size = av_get_bytes_per_sample((AVSampleFormat)frame->format ) * frame->nb_samples * 2;
    }

    memcpy(data.datas, frame->data, sizeof(data.datas));

    return data;
}

