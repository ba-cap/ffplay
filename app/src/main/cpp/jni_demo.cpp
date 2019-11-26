
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

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "ALogger.h"

#include "demo.h"

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

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *res)
{
    av_jni_set_java_vm(vm, 0);
    return JNI_VERSION_1_4;
}


extern "C" JNIEXPORT jstring JNICALL
Java_dai_android_media_ffplay_MainActivity_getFfplayInfo(JNIEnv *env, jobject clazz)
{
    std::string strVersion = "ffmpeg version information:\n";
    strVersion += avcodec_configuration();

    return env->NewStringUTF(strVersion.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_dai_android_media_ffplay_MainActivity_playAudio(
    JNIEnv *env, jobject thiz, jobject manager, jstring name)
{
    const char *path = env->GetStringUTFChars(name, nullptr);
    ALOGD(tag, "read asset file from %s.", path);

    size_t  buffer_size = 0;
    u_char *file_buffer = nullptr;
    int read_bytes      = 0;


    AAssetManager *assetManager = AAssetManager_fromJava(env, manager);
    AAsset        *asset        = nullptr;
    if(nullptr == assetManager)
    {
        goto END_AND_RELEASE;
    }

    asset = AAssetManager_open(assetManager, path, AASSET_MODE_UNKNOWN);
    if(nullptr == asset)
    {
        ALOGW(tag, "open asset file %s failed.", path);
        goto END_AND_RELEASE;
    }

    // the file is short, so we read all file to memory
    buffer_size = (size_t)AAsset_getLength(asset);
    ALOGD(tag, "asset file size: %d", buffer_size);
    file_buffer = new u_char[buffer_size];
    read_bytes  = AAsset_read(asset, file_buffer, buffer_size);

    play_audio_pcm(file_buffer, (size_t)read_bytes);

END_AND_RELEASE:
    if(nullptr != assetManager)
    {
        assetManager = nullptr;
    }

    if(nullptr != asset)
    {
        AAsset_close(asset);
        asset = nullptr;
    }

    if(nullptr != file_buffer)
    {
        delete[] file_buffer;
        file_buffer = nullptr;
    }

    env->ReleaseStringUTFChars(name, path);
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

