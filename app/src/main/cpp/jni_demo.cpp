
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavcodec/jni.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include <jni.h>
#include <string>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "ALogger.h"

static const char *tag = "demo_jni";

static double r2d(AVRational r)
{
    return (r.num == 0 || r.den == 0) ? 0.0 : (double) r.num / (double) r.den;
}

// 当前时间戳
static long long getCurrentMillisecond()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int sec = tv.tv_sec % 360000;
    long long t = sec * 1000 + tv.tv_usec / 1000;
    return t;
}

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *res)
{
    av_jni_set_java_vm(vm, 0);
    return JNI_VERSION_1_4;
}

static void getAllDecoderName(std::string& decoder)
{
    AVCodec *codec = av_codec_next(nullptr);
    while (nullptr != codec)
    {
        if (codec->decode != nullptr && codec->type == AVMEDIA_TYPE_VIDEO)
        {
            char szBuffer[128] = {0x00};
            snprintf(szBuffer, 127, "%s\n", codec->name);
            decoder += szBuffer;
        }
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_dai_android_media_ffplay_MainActivity_getFfplayInfo3(JNIEnv *env, jobject clazz)
{
    std::string hello = "Hello from C++ ";
    hello += avcodec_configuration();
    //初始化解封装
    av_register_all();
    //初始化网络
    avformat_network_init();

    avcodec_register_all();

    //打开文件
    AVFormatContext *ic = NULL;
    char path[] = "/storage/emulated/0/ffmpeg-test/1080.mp4";
    //char path[] = "/sdcard/video.flv";
    int re = avformat_open_input(&ic, path, 0, 0);
    if (re != 0)
    {
        ALOGW(tag, "avformat_open_input failed!:%s", av_err2str(re));
        return env->NewStringUTF(hello.c_str());
    }
    ALOGW(tag, "avformat_open_input %s success!", path);
    //获取流信息
    re = avformat_find_stream_info(ic, 0);
    if (re != 0)
    {
        ALOGW(tag, "avformat_find_stream_info failed!");
    }
    ALOGW(tag, "duration = %lld nb_streams = %d", ic->duration, ic->nb_streams);

    int fps = 0;
    int videoStream = 0;
    int audioStream = 1;

    for (int i = 0; i < ic->nb_streams; i++)
    {
        AVStream *as = ic->streams[i];
        if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            ALOGW(tag, "视频数据");
            videoStream = i;
            fps = r2d(as->avg_frame_rate);

            ALOGW(tag, "fps = %d,width=%d height=%d codeid=%d pixformat=%d", fps,
                  as->codecpar->width,
                  as->codecpar->height,
                  as->codecpar->codec_id,
                  as->codecpar->format
                 );
        }
        else if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            ALOGW(tag, "音频数据");
            audioStream = i;
            ALOGW(tag, "sample_rate=%d channels=%d sample_format=%d",
                  as->codecpar->sample_rate,
                  as->codecpar->channels,
                  as->codecpar->format
                 );
        }
    }
    //ic->streams[videoStream];
    //获取音频流信息
    audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    ALOGW(tag, "av_find_best_stream audioStream = %d", audioStream);
    //////////////////////////////////////////////////////////
    //打开视频解码器
    //软解码器
    AVCodec *codec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);
    //硬解码
    codec = avcodec_find_decoder_by_name("h264_mediacodec");
    if (!codec)
    {
        ALOGW(tag, "avcodec_find failed!");
        return env->NewStringUTF(hello.c_str());
    }
    //解码器初始化
    AVCodecContext *vc = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(vc, ic->streams[videoStream]->codecpar);

    vc->thread_count = 8;
    //打开解码器
    re = avcodec_open2(vc, 0, 0);
    //vc->time_base = ic->streams[videoStream]->time_base;
    ALOGW(tag, "vc timebase = %d/ %d", vc->time_base.num, vc->time_base.den);
    if (re != 0)
    {
        ALOGW(tag, "avcodec_open2 video failed!");
        return env->NewStringUTF(hello.c_str());
    }

    //////////////////////////////////////////////////////////
    //打开音频解码器
    //软解码器
    AVCodec *acodec = avcodec_find_decoder(ic->streams[audioStream]->codecpar->codec_id);
    //硬解码
    //codec = avcodec_find_decoder_by_name("h264_mediacodec");
    if (!acodec)
    {
        ALOGW(tag, "avcodec_find failed!");
        return env->NewStringUTF(hello.c_str());
    }
    //解码器初始化
    AVCodecContext *ac = avcodec_alloc_context3(acodec);
    avcodec_parameters_to_context(ac, ic->streams[audioStream]->codecpar);
    ac->thread_count = 8;
    //打开解码器
    re = avcodec_open2(ac, 0, 0);
    if (re != 0)
    {
        ALOGW(tag, "avcodec_open2  audio failed!");
        return env->NewStringUTF(hello.c_str());
    }
    //读取帧数据
    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    long long start = getCurrentMillisecond();
    int frameCount = 0;
    for (;;)
    {
        //超过三秒
        if (getCurrentMillisecond() - start >= 3000)
        {
            ALOGW(tag, "now decode fps is %d", frameCount / 3);
            start = getCurrentMillisecond();
            frameCount = 0;
        }

        int re = av_read_frame(ic, pkt);
        if (re != 0)
        {

            ALOGW(tag, "读取到结尾处!");
            int pos = 20 * r2d(ic->streams[videoStream]->time_base);

            break;
            //av_seek_frame(ic, videoStream, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME );
            //continue;
        }
        //只测试视频
        /*if(pkt->stream_index !=videoStream)
        {
            continue;
        }*/
        //LOGW("stream = %d size =%d pts=%lld flag=%d",
        //     pkt->stream_index,pkt->size,pkt->pts,pkt->flags
        //);

        AVCodecContext *cc = vc;
        if (pkt->stream_index == audioStream)
            cc = ac;

        //发送到线程中解码
        re = avcodec_send_packet(cc, pkt);
        //清理
        int p = pkt->pts;
        av_packet_unref(pkt);

        if (re != 0)
        {
            ALOGW(tag, "avcodec_send_packet failed!");
            continue;
        }
        for (;;)
        {
            re = avcodec_receive_frame(cc, frame);
            if (re != 0)
            {
                //LOGW("avcodec_receive_frame failed!");
                break;
            }
            //LOGW("avcodec_receive_frame %lld",frame->pts);
            //如果是视频帧
            if (cc == vc)
            {
                frameCount++;
            }

        }
        //////////////////////
    }



    //关闭上下文
    avformat_close_input(&ic);
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_dai_android_media_ffplay_MainActivity_getFfplayInfo4(JNIEnv *env, jobject clazz)
{
    std::string str(avcodec_configuration());

    av_register_all();
    avcodec_register_all();

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
    ALOGD(tag, "nb_streams: %d", ic->nb_streams);

    int fps = 0;
    int videoStream = 0, audioStream = 0;

    for (int i = 0; i < ic->nb_streams; ++i)
    {
        AVStream *stream = ic->streams[i];
        // video type
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            fps = r2d(stream->avg_frame_rate);
            ALOGD(tag, "this is video");
            ALOGD(tag, "FPS      = %d", fps);
            ALOGD(tag, "WIDTH    = %d", stream->codecpar->width);
            ALOGD(tag, "HEIGHT   = %d", stream->codecpar->height);
            ALOGD(tag, "CODECID  = %d", stream->codecpar->codec_id);
            ALOGD(tag, "PIXFORMAT= %d", stream->codecpar->format);
        }
        // audio type
        else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStream = i;
            ALOGD(tag, "this is audio");
            ALOGD(tag, "sample_rate   = %d", stream->codecpar->sample_rate);
            ALOGD(tag, "channel_layout= %llu", stream->codecpar->channel_layout);
            ALOGD(tag, "channels      = %d", stream->codecpar->channels);
            ALOGD(tag, "sample_format = %d", stream->codecpar->format);
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

    bool useSoftDecoder = false;
    // find the soft video decoder
    //
    AVCodec *videoCoder = nullptr;
    if (useSoftDecoder)
    {
        videoCoder = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);
        if (nullptr == videoCoder)
        {
            ALOGE(tag, "can't found the soft video decoder");
        }
    }
    else
    {
        videoCoder = avcodec_find_decoder_by_name("h264_mediacodec");
        if (nullptr == videoCoder)
        {
            std::string supportDecoder;
            getAllDecoderName(supportDecoder);
            ALOGE(tag, "can't found the hard video decoder, support:\n%s", supportDecoder.c_str());

            return env->NewStringUTF("Not found AVCodec named: 'h264_mediacodec'");
        }

        ALOGW(tag, "found AVCodec named: 'h264_mediacodec'");
    }

    // init the video decoder
    AVCodecContext *videoDecoderCtx = nullptr;
    if (nullptr != videoCoder)
    {
        videoDecoderCtx = avcodec_alloc_context3(videoCoder);
        videoDecoderCtx->thread_count = 4; // use 4 thread to decode

        avcodec_parameters_to_context(videoDecoderCtx, ic->streams[videoStream]->codecpar);

        ret = avcodec_open2(videoDecoderCtx, nullptr, nullptr);
        if (ret != 0)
        {
            ALOGE(tag, "open video decoder failed, reason:%s.", av_err2str(ret));
        }
    }

    // find and open audio decoder
    AVCodec *audioCoder = avcodec_find_decoder(ic->streams[audioStream]->codecpar->codec_id);
    if (nullptr == audioCoder)
    {
        ALOGE(tag, "can't found the soft audio decoder");
    }

    AVCodecContext *audioCodecCtx = nullptr;
    if (nullptr != audioCoder)
    {
        audioCodecCtx = avcodec_alloc_context3(audioCoder);
        audioCodecCtx->thread_count = 4; // use 4 thread to decode
        avcodec_parameters_to_context(audioCodecCtx, ic->streams[audioStream]->codecpar);

        ret = avcodec_open2(audioCodecCtx, nullptr, nullptr);
        if (ret != 0)
        {
            ALOGE(tag, "open audio decoder failed, reason:%s.", av_err2str(ret));
        }
    }

    // read the frame stream
    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    long long startTime = getCurrentMillisecond();
    int frameCount = 0;

    for (;;)
    {
        // 超过 1s 统计一次
        long long nowTime = getCurrentMillisecond();
        if (nowTime - startTime >= 1000)
        {
            startTime = nowTime;
            ALOGW(tag, "now decode fps is %d", frameCount);
            frameCount = 0;
        }

        int ret = av_read_frame(ic, pkt);
        if (ret != 0)
        {
            ALOGW(tag, "read the end of file");

            bool endFileExit = true;
            if (endFileExit)
            {
                break;
            }
            else
            {
                // seek to position 5s
                int pos = 5 * r2d(ic->streams[videoStream]->time_base);
                av_seek_frame(ic, videoStream, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
                continue;
            }
        }

        // ALOGD(tag, "stream:%d size:%d pts:%lld flag:%d", pkt->stream_index, pkt->size, pkt->pts, pkt->flags);

        // not video
        if (pkt->stream_index != videoStream)
        {
            continue;
        }

        ret = avcodec_send_packet(videoDecoderCtx, pkt);
        av_packet_unref(pkt);
        if (ret != 0)
        {
            ALOGE(tag, "send packet failed, reason: %s.", av_err2str(ret));
            continue;
        }

        for (;;)
        {
            ret = avcodec_receive_frame(videoDecoderCtx, frame);
            if (ret != 0)
            {
                // ALOGE(tag, "receive packet frame failed, reason: %s.", av_err2str(ret));
                break;
            }

            // ALOGD(tag, "receive frame pts=%lld.", frame->pts);

            frameCount++;
        }


        //

        // free the memory
        av_packet_unref(pkt);
    }


    // close the AVFormatContext
    avformat_close_input(&ic);

    return env->NewStringUTF(str.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_dai_android_media_ffplay_MainActivity_getFfplayInfo(JNIEnv *env, jobject clazz)
{
    std::string strVersion = "ffmpeg version information:\n";
    strVersion += avcodec_configuration();

    return env->NewStringUTF(strVersion.c_str());
}


static void print_av_stream_info(int type, const AVStream *stream )
{
    switch(type)
    {
        case AVMEDIA_TYPE_VIDEO:
            {
                ALOGI(tag, "video stream information:");
                ALOGI(tag, "     width: %d", stream->codecpar->width);
                ALOGI(tag, "    height: %d", stream->codecpar->height);
                ALOGI(tag, "  codec id: %d", stream->codecpar->codec_id);
                ALOGI(tag, "pix format: %d", stream->codecpar->format);
                break;
            }

        case AVMEDIA_TYPE_AUDIO:
            {
                ALOGI(tag, "audio stream information:");
                ALOGI(tag, "  sample rate: %d", stream->codecpar->sample_rate);
                ALOGI(tag, "     channels: %d", stream->codecpar->channels);
                ALOGI(tag, "sample format: %d", stream->codecpar->codec_id);
                break;
            }

        default:
            {
                ALOGW(tag, "unknown stream type: %d.", type);
                break;
            }
    }
}

extern "C" JNIEXPORT void JNICALL
Java_dai_android_media_ffplay_XGLSurfaceView_open_1player(
    JNIEnv *env, jobject clazz,
    jstring url, jobject surface)
{
    const char *path = env->GetStringUTFChars(url, nullptr);

    // register all demutex
    // this API disable in ffmpeg 4.2
    av_register_all();

    // init the network
    avformat_network_init();

    // register all codec
    avcodec_register_all();


    AVFormatContext *pFmtCtx = nullptr;
    int result = avformat_open_input(&pFmtCtx, path, nullptr, nullptr);
    if (0 != result )
    {
        ALOGE(tag, "ffmpeg open file:%s failed, reason:%s.", path, av_err2str(result));
        return;
    }
    ALOGD(tag, "ffmpeg open file:%s success.", path);


    // get the stream information
    result = avformat_find_stream_info(pFmtCtx, nullptr);
    if (0 != result)
    {
        ALOGW(tag, "ffmpeg find stream info failed, reason:%s.", av_err2str(result));
    }

    // print the stream base info
    ALOGD(tag, "duration: %lld,  streams: %d", pFmtCtx->duration, pFmtCtx->nb_streams);

    // base info
    int fps          = 0;
    int stream_video = 0;
    int stream_audio = 1;

    for(int i = 0; i < pFmtCtx->nb_streams; ++i)
    {
        AVStream *stream = pFmtCtx->streams[i];

        enum AVMediaType codec_type = stream->codecpar->codec_type;
        switch (codec_type)
        {
            case AVMEDIA_TYPE_VIDEO:
                {
                    stream_video = i;
                    break;
                }


            case AVMEDIA_TYPE_AUDIO:
                {
                    stream_audio = i;
                    break;
                }
        }
        print_av_stream_info(codec_type, stream);
    }


    // other way to use av_find_best_stream
    // we have find the audio stream, just tell you the function use:
    //// stream_audio = av_find_best_stream(pFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);


    //----------------------------------------------------------------------------------------------
    // video coder
    // first we use the hardware decoder
    AVCodec *codec_video = nullptr;
    codec_video = avcodec_find_decoder_by_name("h264_mediacodec");
    if(nullptr == codec_video)
    {
        ALOGE(tag, "can not find the video decoder named: 'h264_mediacodec', use soft decoder");
        // use the soft decoder
        codec_video = avcodec_find_decoder(pFmtCtx->streams[stream_video]->codecpar->codec_id);
    }

    if(nullptr == codec_video)
    {
        ALOGE(tag, "can not find any hardware or soft video decoder");
        return;
    }

    // init video decoder
    // open decoder
    AVCodecContext *ctx_video = avcodec_alloc_context3(codec_video);
    avcodec_parameters_to_context(ctx_video, pFmtCtx->streams[stream_video]->codecpar);
    ctx_video->thread_count = 4;

    result = avcodec_open2(ctx_video, nullptr, nullptr);
    if (0 != result)
    {
        ALOGE(tag, "open video coder failed, reason: %s.", av_err2str(result));
        return;
    }

    ALOGI(tag, "video time_base: %d-%d", ctx_video->time_base.num, ctx_video->time_base.den);


    //----------------------------------------------------------------------------------------------
    // audio coder (use the hardware fist)
    AVCodec *codec_audio = nullptr;
    /// codec_audio = avcodec_find_decoder_by_name("h264_mediacodec");
    // test show:
    // not support hardware for audio
    if(nullptr == codec_audio)
    {
        ALOGE(tag, "can not find the audio decoder named: 'h264_mediacodec', use soft decoder");
        codec_audio = avcodec_find_decoder(pFmtCtx->streams[stream_audio]->codecpar->codec_id);
    }
    if(nullptr == codec_audio)
    {
        ALOGE(tag, "can not find any hardware or soft audio decoder");
        return;
    }

    // audio decoder init
    AVCodecContext *ctx_audio = avcodec_alloc_context3(codec_audio);
    avcodec_parameters_to_context(ctx_audio, pFmtCtx->streams[stream_audio]->codecpar);
    ctx_audio->thread_count = 4;

    // open the decoder
    result = avcodec_open2(ctx_audio, nullptr, nullptr);
    if(0 != result)
    {
        ALOGE(tag, "open audio coder failed, reason: %s.", av_err2str(result));
        return;
    }


    // read the frame data
    AVPacket *pkt   = av_packet_alloc();
    AVFrame  *frame = av_frame_alloc();

    long long time_start  = getCurrentMillisecond();
    int       frame_count = 0;


    // init video sws pixel
    SwsContext *sws_ctx_video = nullptr;
    int out_width  = 1280;
    int out_height = 720;
    char *rgb = new char[1920 * 1080 * 4];


    // init audio sws context
    SwrContext *swr_ctx_audio = swr_alloc();
    swr_ctx_audio = swr_alloc_set_opts(
                        swr_ctx_audio,
                        av_get_default_channel_layout(2),
                        AV_SAMPLE_FMT_S16, ctx_audio->sample_rate,
                        av_get_default_channel_layout(ctx_audio->channels),
                        ctx_audio->sample_fmt, ctx_audio->sample_rate,
                        0, nullptr );
    result = swr_init(swr_ctx_audio);
    if(0 != result)
    {
        ALOGW(tag, "int swr failed, reason: %s", av_err2str(result));
    }
    char *pcm = new char[48000 * 4 * 2];


    // display window init
    ANativeWindow *native_win = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_setBuffersGeometry(native_win, out_width, out_height, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer win_buffer;

    // loop
    for( ; ; )
    {
        long long time_current = getCurrentMillisecond();
        if(time_current - time_start >= 3000)
        {
            ALOGD(tag, ">>>> decode fps is %d.", frame_count / 3);
            time_start  = time_current;
            frame_count = 0;
        }

        bool end_will_seek = false;

        result = av_read_frame(pFmtCtx, pkt);
        if (0 != result)
        {
            ALOGI(tag, ">>>> end tail of file");

            if(!end_will_seek)
            {
                break;
            }

            int pos = 20 * r2d(pFmtCtx->streams[stream_video]->time_base);
            av_seek_frame(pFmtCtx, stream_video, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            continue;
        }


        // base AVCodecContext
        AVCodecContext *ctx_codec_base = nullptr;
        if(pkt->stream_index == stream_video)
        {
            ctx_codec_base = ctx_video;
        }
        else if(pkt->stream_index == stream_audio)
        {
            ctx_codec_base = ctx_audio;
        }
        else
        {
            ALOGW(tag, "bad unknown package stream index = %d.", pkt->stream_index);
            continue;
        }


        // set data to decode
        result = avcodec_send_packet(ctx_codec_base, pkt);
        // clean up the package
        int p = pkt->pts;
        av_packet_unref(pkt);

        if(0 != result)
        {
            ALOGW(tag, ">>>> send package failed.");
            continue;
        }

        // inner loop
        for(;;)
        {
            result = avcodec_receive_frame(ctx_codec_base, frame);
            if( 0 != result)
            {
                break;
            }

            // video frame
            if(ctx_codec_base == ctx_video)
            {
                frame_count++;

                sws_ctx_video = sws_getCachedContext(
                                    sws_ctx_video,
                                    frame->width, frame->height,
                                    (AVPixelFormat)frame->format,
                                    out_width, out_height,
                                    AV_PIX_FMT_RGBA,
                                    SWS_FAST_BILINEAR,
                                    nullptr, nullptr, nullptr);
                if(!sws_ctx_video)
                {
                    ALOGW(tag, "sws obtain cached context failed");
                }
                else
                {
                    uint8_t *data[AV_NUM_DATA_POINTERS] = {0};
                    data[0]  = (uint8_t *)rgb;
                    int lines[AV_NUM_DATA_POINTERS] = {0};
                    lines[0] = out_width * 4;

                    int h = sws_scale(
                                sws_ctx_video, (const uint8_t **)frame->data,
                                frame->linesize, 0,
                                frame->height, data, lines);
                    ALOGI(tag, ">>>> scale %d", h);
                    if(h > 0)
                    {
                        ANativeWindow_lock(native_win, &win_buffer, 0);
                        uint8_t *dst = (uint8_t *)win_buffer.bits;
                        memcpy(dst, rgb, out_width * out_height * 4);
                        ANativeWindow_unlockAndPost(native_win);
                    }
                }
            }

            // audio frame
            if(ctx_codec_base == ctx_audio)
            {
                uint8_t *out[2] = {0};
                out[0] = (uint8_t *) pcm;

                int len = swr_convert(
                              swr_ctx_audio,
                              out,
                              frame->nb_samples,
                              (const uint8_t **)frame->data,
                              frame->nb_samples);
                ALOGI(tag, "swr convert: %d.", len);
            }
        }

    }

    // clean new memory
    delete[] rgb;
    delete[] pcm;

    avformat_close_input(&pFmtCtx);

    // release the Java String
    env->ReleaseStringUTFChars(url, path);
}

