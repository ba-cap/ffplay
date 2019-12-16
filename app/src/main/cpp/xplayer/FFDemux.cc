//
//   author: patrick.dai
// datetime: 2019-12-02 12:12
//

extern "C" {
#include "libavformat/avformat.h"
}

#include <inttypes.h>

#include "FFDemux.h"
#include "XLog.h"

FFDemux::FFDemux()
{
    static bool isFirst = true;
    if(isFirst)
    {
        isFirst = false;

        // 注册所有的解封装器
        av_register_all();

        // 注册所有的解码器
        avcodec_register_all();

        // 初始化网络
        avformat_network_init();

        XLOGI("register ffmpeg");
    }

}

bool FFDemux::Open(const char *url)
{
    XLOGI("begin open file %s", url);

    int ret = avformat_open_input(&ic, url, nullptr, nullptr);
    if(ret != 0)
    {
        char buffer[1024] = {0x00};
        av_strerror(ret, buffer, sizeof(buffer));
        XLOGE("FFDemux open %s failed, reason: %s", url, buffer);
        return false;
    }
    XLOGI("FFDemux open %s success", url);

    // 读取文件信息
    ret = avformat_find_stream_info(ic, nullptr);
    if(ret != 0)
    {
        char buffer[1024] = {0x00};
        av_strerror(ret, buffer, sizeof(buffer));
        XLOGE("FFDemux find stream info failed, reason: %s", buffer);
        return false;
    }

    // AV_TIME_BASE 指的是 1 秒中有多少个时间单位
    // 这个值可能不一定有
    mTotalMillisecond = ic->duration / (AV_TIME_BASE / 1000);

    XLOGI("total duration: %dms", mTotalMillisecond );

    // 在 open 时调用一下
    getVideoParameter();
    getAudioParameter();

    return true;
}

XParameter FFDemux::getVideoParameter()
{
    if(!ic)
    {
        return XParameter();
    }

    // 获取视频流索引
    video_stream_idx = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if(video_stream_idx < 0)
    {
        XLOGE("ffmpeg find video index failed");
        return XParameter();
    }
    XParameter parameter;
    parameter.para = ic->streams[video_stream_idx]->codecpar;
    return parameter;
}

XParameter FFDemux::getAudioParameter()
{
    if(!ic)
    {
        return XParameter();
    }

    // 获取视频流索引
    audio_stream_idx  = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if(audio_stream_idx < 0)
    {
        XLOGE("ffmpeg find audio index failed");
        return XParameter();
    }
    XParameter parameter;
    parameter.para = ic->streams[audio_stream_idx]->codecpar;
    return parameter;
}

XData FFDemux::Read()
{
    if(!ic)
    {
        return XData();
    }

    XData data;

    AVPacket *pkt = av_packet_alloc();
    int ret = av_read_frame(ic, pkt);
    if(ret != 0)
    {
        av_packet_free(&pkt);
        return XData();
    }

    XLOGI("packet size is %d pts %ld", pkt->size, pkt->pts);
    data.data = (unsigned char *)pkt;
    data.size = pkt->size;

    // audio stream
    if(pkt->stream_index == audio_stream_idx)
    {
        data.type = DATA_AUDIO;
    }
    // video stream
    else if(pkt->stream_index == video_stream_idx)
    {
        data.type = DATA_VIDEO;
    }
    // neither video or audio
    // drop and destroy the data
    else
    {
        av_packet_free(&pkt);
        return XData();
    }

    return data;
}