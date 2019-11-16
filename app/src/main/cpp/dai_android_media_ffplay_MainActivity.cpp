
#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

#ifdef __cplusplus
}
#endif

#include <jni.h>
#include <string>

#include "ALogger.h"

static const char *tag = "MainActivity_JNI";

static double r2d(AVRational r)
{
    return (r.num == 0 || r.den == 0) ? 0.0 : (double)r.num / (double)r.den;
}

extern "C" JNIEXPORT jstring JNICALL
Java_dai_anroid_media_ffplay_MainActivity_getFfplayInfo(JNIEnv *env, jobject clazz)
{
    std::string str(avcodec_configuration());

    avformat_network_init();

    AVFormatContext *ic = nullptr;
    //const char path[] = "/storage/emulated/0/ffmpeg-test/1080.mp4";
    const char path[] = "/storage/emulated/0/ffmpeg-test/pptv_start_advert.mp4";

    int ret = avformat_open_input(&ic, path, nullptr, nullptr);
    if (ret != 0)
    {
        ALOGE(tag, "open file:%s failed, reason:%s.", path, av_err2str(ret));
        return env->NewStringUTF(str.c_str());
    }

    ret = avformat_find_stream_info(ic, nullptr);
    if (ret != 0)
    {
        ALOGE(tag, "find stream info failed, reason:%s.", av_err2str(ret));
    }

    ALOGD(tag, "open file : %s success.", path);
    ALOGD(tag, "duration  : %lld.", ic->duration);
    ALOGD(tag, "nb_streams: %d",    ic->nb_streams);

    int fps = 0;
    int videoStream = 0, audioStream = 0;

    for (int i = 0; i < ic->nb_streams; ++i)
    {
        AVStream *stream  = ic->streams[i];
        // video type
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            fps = r2d( stream->avg_frame_rate);
            ALOGD(tag, "this is video");
            ALOGD(tag, "FPS      = %d", fps);
            ALOGD(tag, "WIDTH    = %d", stream->codecpar->width);
            ALOGD(tag, "HEIGHT   = %d", stream->codecpar->height);
            ALOGD(tag, "CODECID  = %d", stream->codecpar->codec_id);
            ALOGD(tag, "PIXFORMAT= %d", stream->codecpar->format);
        }
        // audio type
        else if(stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStream = i;
            ALOGD(tag, "this is audio");
            ALOGD(tag, "sample_rate   = %d",   stream->codecpar->sample_rate);
            ALOGD(tag, "channel_layout= %lld", stream->codecpar->channel_layout);
            ALOGD(tag, "channels      = %d",   stream->codecpar->channels);
            ALOGD(tag, "sample_format = %d",   stream->codecpar->format);
        }
    }


    // get audio stream info
    // av_find_best_stream 相关参数说明:
    //    AVFormatContext *ic,    // 上下文
    //    enum AVMediaType type,  // 音频或者视频类型
    //    int wanted_stream_nb,   // 用不到可以传递 -1
    //    int related_stream,     // ffmpeg 有个节目的概念 ，一套视频中有多个节目, 目前该值用不到，使用 -1
    //    AVCodec **decoder_ret,  // 最终的解码器，也用不到 null
    //    int flags               // 官方说法还是没有用到，可能留给将来用, 默认可以填写 0
    audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    ALOGD(tag, "find best audio stream: %d", audioStream);



    // close the AVFormatContext
    avformat_close_input(&ic);

    return env->NewStringUTF(str.c_str());
}


