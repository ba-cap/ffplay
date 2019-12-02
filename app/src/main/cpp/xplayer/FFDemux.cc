//
//   author: patrick.dai
// datetime: 2019-12-02 12:12
//

extern "C" {
#include "libavformat/avformat.h"
}

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

    XLOGI("total duration: %s ms", mTotalMillisecond );

    return true;
}

XParameter FFDemux::getParameter()
{
    if(!ic)
    {
        return XParameter();
    }

    // 获取视频流索引
    int video_idx = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if(video_idx < 0)
    {
        XLOGE("ffmpeg find video index failed");
        return XParameter();
    }
    XParameter parameter;
    parameter.para = ic->streams[video_idx]->codecpar;
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

    return data;
}