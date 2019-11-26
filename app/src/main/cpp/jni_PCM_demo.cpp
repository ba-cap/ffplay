#include <sys/types.h>

#include "ALogger.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

static const char *tag = "jni_pcm_demo";

static SLObjectItf  engineSL = nullptr;

SLEngineItf createSL()
{
    SLresult    result;
    SLEngineItf engine;
    result = slCreateEngine(&engineSL, 0, 0, 0, 0, 0 );
    if(SL_RESULT_SUCCESS != result)
    {
        ALOGW(tag, "create SL engine failed");
        return nullptr;
    }

    result = (*engineSL)->Realize(engineSL, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result)
    {
        ALOGW(tag, "realize SL engine failed");
        return nullptr;
    }

    result = (*engineSL)->GetInterface(engineSL, SL_IID_ENGINE, &engine);
    if(SL_RESULT_SUCCESS != result)
    {
        ALOGW(tag, "get engine interface failed");
        return nullptr;
    }

    return engine;
}


static struct pcm_data
{
    const u_char *source;
    size_t        size;
    size_t        pos;
};

void pcm_callback(SLAndroidSimpleBufferQueueItf buffer, void *ctx)
{
    if(nullptr == ctx)
    {
        ALOGE(tag, "bad context");
        return;
    }
    if(nullptr == buffer)
    {
        ALOGE(tag, "buffer queue is empty");
        return;
    }
    struct pcm_data *data = (struct  pcm_data *)ctx;

    ALOGD(tag, "callback>>> %d / %d", data->pos, data->size);

    if (data->pos < data->size)
    {
        size_t real = 1024;
        if ( data->size - data->pos < 1024 )
        {
            real = data->size - data->pos;
        }
        const u_char *buffered = data->source + data->pos;
        data->pos += real;
        (*buffer)->Enqueue(buffer, buffered, real);
    }
    else
    {
        delete data;
    }
}


void play_audio_pcm(const u_char *data, size_t size)
{
    ALOGI(tag, "play_audio_pcm: size=%ld", size);

    struct pcm_data *pcmData = new pcm_data;
    pcmData->source = data;
    pcmData->size   = size;
    pcmData->pos    = 0;

    SLEngineItf  engine = createSL();
    SLObjectItf  mix    = nullptr;
    SLresult     result = 0;

    result = (*engine)->CreateOutputMix(engine, &mix, 0, 0, 0);
    if(result != SL_RESULT_SUCCESS)
    {
        ALOGE(tag, "create output mix failed");
    }

    result = (*mix)->Realize(mix, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        ALOGE(tag, "realize output mix failed");
    }

    SLDataLocator_OutputMix outMix    = { SL_DATALOCATOR_OUTPUTMIX, mix };
    SLDataSink              audioSink = { &outMix, 0 };

    SLDataLocator_AndroidBufferQueue queue = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 10 };
    // audio format
    SLDataFormat_PCM pcm =
    {
        SL_DATAFORMAT_PCM,
        2,
        SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
        SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource dataSource = { &queue, &pcm };

    // create the player
    SLObjectItf  player          = nullptr;
    SLPlayItf    playerInterface = nullptr;
    SLAndroidSimpleBufferQueueItf pcmQueue = nullptr;

    const SLInterfaceID ids[]      = {SL_IID_BUFFERQUEUE};
    const SLboolean     requires[] = {SL_BOOLEAN_TRUE};

    result = (*engine)->CreateAudioPlayer(
                 engine,
                 &player, &dataSource, &audioSink,
                 sizeof(ids) / sizeof(SLInterfaceID), ids,
                 requires );
    if(result != SL_RESULT_SUCCESS)
    {
        ALOGE(tag, "create audio player failed");
    }
    ALOGD(tag, "create audio player success");

    result =  (*player)->Realize(player, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        ALOGE(tag, "get player realize failed");
    }
    ALOGD(tag, "get player realize success");

    result = (*player)->GetInterface(player, SL_IID_PLAY, &playerInterface);
    if(result != SL_RESULT_SUCCESS)
    {
        ALOGE(tag, "get player interface failed");
    }
    ALOGD(tag, "get player interface success");

    result = (*player)->GetInterface(player, SL_IID_BUFFERQUEUE, &pcmQueue);
    if(result != SL_RESULT_SUCCESS)
    {
        ALOGE(tag, "get queue interface failed");
    }
    ALOGD(tag, "get queue interface success");

    (*pcmQueue)->RegisterCallback(pcmQueue, pcm_callback, pcmData);

    (*playerInterface)->SetPlayState(playerInterface, SL_PLAYSTATE_PLAYING);

    // start the callback
    (*pcmQueue)->Enqueue(pcmQueue, "", 1);
}