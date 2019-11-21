
#include "OpenAudio.h"
#include "ALogger.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <pthread.h>
#include <jni.h>


#define NDK_TAG "c_OpenAudio"


static SLuint32 channel_mask(SLuint32 channels)
{
    return channels > 1 ? SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT : SL_SPEAKER_FRONT_CENTER;
}

static SLuint32 convert_sample_rate(int sample_rate)
{
    switch (sample_rate)
    {
        case 8000:
            return SL_SAMPLINGRATE_8;

        case 11025:
            return SL_SAMPLINGRATE_11_025;

        case 22050:
            return SL_SAMPLINGRATE_22_05;

        case 24000:
            return SL_SAMPLINGRATE_24;

        case 32000:
            return SL_SAMPLINGRATE_32;

        case 44100:
            return SL_SAMPLINGRATE_44_1;

        case 48000:
            return SL_SAMPLINGRATE_48;

        case 64000:
            return SL_SAMPLINGRATE_64;

        case 88200:
            return SL_SAMPLINGRATE_88_2;

        case 96000:
            return SL_SAMPLINGRATE_96;

        case 192000:
            return SL_SAMPLINGRATE_192;

        default:
            return 0;
    }
}


class ThreadLocker
{
private:
    pthread_mutex_t  m{};
    pthread_cond_t   c{};
    unsigned char    s{1};

public:
    ThreadLocker()
    {
        pthread_mutex_init( &m, nullptr );
        pthread_cond_init ( &c, nullptr );
    }

    ~ThreadLocker()
    {
        notify();
        pthread_cond_destroy ( &c );
        pthread_mutex_destroy( &m );
    }

    void wait()
    {
        pthread_mutex_lock( &m );
        while ( !s )
        {
            pthread_cond_wait( &c, &m );
        }
        s = 0;
        pthread_mutex_unlock( &m );
    }

    void notify()
    {
        pthread_mutex_lock  ( &m );
        s = 1;
        pthread_cond_signal ( &c );
        pthread_mutex_unlock( &m );
    }
};

static const uint32_t SAMPLE_RATE = 48000;
static const uint8_t  CHANNELS    = 1;
static const uint8_t  PERIOD_TIME = 20;   //ms
static const uint32_t FRAME_SIZE  = SAMPLE_RATE * PERIOD_TIME / 1000;
static const uint32_t BUFFER_NUM  = FRAME_SIZE * CHANNELS;
static const uint32_t BUFFER_SIZE = BUFFER_NUM * sizeof(short);



class AudioDevice
{
private:
    // the locker
    ThreadLocker _locker{};

    // opensles
    SLObjectItf _engineObject{nullptr};
    SLEngineItf _engineInterface{nullptr};

    SLObjectItf _recorderObject{nullptr};
    SLRecordItf _recorderInterface{nullptr};

    SLObjectItf _outputMixObject{nullptr};

    SLObjectItf _playerObject{nullptr};
    SLPlayItf   _playerInterface{nullptr};

    SLAndroidSimpleBufferQueueItf    _queueInterface{nullptr};

    SLDataLocator_AndroidBufferQueue _queue{};
    SLDataFormat_PCM                 _format{};

    SLDataLocator_OutputMix          _outputMix{};
    SLDataLocator_IODevice           _device{};

    SLDataSource   _source{};
    SLDataSink     _sink{};


private:

    static void bufferQueueCallback(SLAndroidSimpleBufferQueueItf caller, void *ctx)
    {
        auto *device = (AudioDevice *)ctx;
        if(nullptr != device)
        {
            device->notify();
        }
    }

    void notify()
    {
        _locker.notify();
    }

    void init()
    {
        // engine object
        slCreateEngine(&_engineObject, 0, nullptr, 0, nullptr, nullptr);
        (*_engineObject)->Realize(_engineObject, SL_BOOLEAN_FALSE);

        // engine interface
        (*_engineObject)->GetInterface(_engineObject, SL_IID_ENGINE, &_engineInterface);
    }

    void destroy()
    {
        // player
        if (nullptr != _playerObject)
        {
            (*_playerObject)->Destroy(_playerObject);
            _playerObject    = nullptr;
            _playerInterface = nullptr;
        }

        // recorder
        if (nullptr != _recorderObject)
        {
            (*_recorderObject)->Destroy(_recorderObject);
            _recorderObject    = nullptr;
            _recorderInterface = nullptr;
        }

        // output mix
        if(nullptr != _outputMixObject)
        {
            (*_outputMixObject)->Destroy(_outputMixObject);
            _outputMixObject = nullptr;
        }

        // queue interface
        if(nullptr != _queueInterface)
        {
            _queueInterface = nullptr;
        }

        // engine
        if(nullptr != _engineObject)
        {
            (*_engineObject)->Destroy(_engineObject);
            _engineObject    = nullptr;
            _engineInterface = nullptr;
        }
    }


public:
    AudioDevice()
    {
        init();
    }

    ~AudioDevice()
    {
        destroy();
    }

    void init_recorder(SLuint32 channels, SLuint32 sampleRate)
    {
        /// source
        _device.locatorType = SL_DATALOCATOR_IODEVICE;
        _device.deviceType  = SL_IODEVICE_AUDIOINPUT;
        _device.deviceID    = SL_DEFAULTDEVICEID_AUDIOINPUT;
        _device.device      = nullptr; // Must be NULL if deviceID parameter is to be used.

        _source.pLocator = &_device;
        _source.pFormat  = nullptr;    // This parameter is ignored if pLocator is SLDataLocator_IODevice.


        /// sink
        _queue.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
        _queue.numBuffers  = 2;

        _format.formatType    = SL_DATAFORMAT_PCM;
        _format.numChannels   = channels;
        _format.samplesPerSec = sampleRate;
        _format.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
        _format.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
        _format.channelMask   = channel_mask(channels);
        _format.endianness    = SL_BYTEORDER_LITTLEENDIAN;

        _sink.pLocator = &_queue;
        _sink.pFormat  = &_format;

        // recorder
        SLInterfaceID id[]       = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
        SLboolean     required[] = { SL_BOOLEAN_TRUE };

        (*_engineInterface)->CreateAudioRecorder( _engineInterface,
                                                  &_recorderObject,
                                                  &_source,
                                                  &_sink,
                                                  1,
                                                  id,
                                                  required );
        (*_recorderObject)->Realize(_recorderObject, SL_BOOLEAN_FALSE);

        // register callback
        (*_recorderObject)->GetInterface    (_recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &_queueInterface);
        (*_queueInterface)->RegisterCallback(_queueInterface, bufferQueueCallback, this);

        // start recording
        (*_recorderObject)   ->GetInterface  (_recorderObject, SL_IID_RECORD, &_recorderInterface);
        (*_recorderInterface)->SetRecordState(_recorderInterface, SL_RECORDSTATE_RECORDING);
    }

    void init_player(SLuint32 channels, SLuint32 sampleRate)
    {
        /// source
        _queue.locatorType    = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
        _queue.numBuffers     = 2;

        _format.formatType    = SL_DATAFORMAT_PCM;
        _format.numChannels   = channels;
        _format.samplesPerSec = sampleRate;
        _format.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
        _format.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
        _format.channelMask   = channel_mask(channels);
        _format.endianness    = SL_BYTEORDER_LITTLEENDIAN;

        _source.pLocator = &_queue;
        _source.pFormat  = &_format;


        /// sink
        (*_engineInterface)->CreateOutputMix( _engineInterface,
                                              &_outputMixObject,
                                              0, nullptr,
                                              nullptr );
        (*_outputMixObject)->Realize(_outputMixObject, SL_BOOLEAN_FALSE);

        _outputMix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
        _outputMix.outputMix   = _outputMixObject;

        _sink.pLocator = &_outputMix;
        _sink.pFormat  = nullptr; // This parameter is ignored if pLocator is SLDataLocator_IODevice or SLDataLocator_OutputMix.

        /// player
        SLInterfaceID id[]       = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
        SLboolean     required[] = { SL_BOOLEAN_TRUE };
        (*_engineInterface)->CreateAudioPlayer( _engineInterface,
                                                &_playerObject,
                                                &_source,
                                                &_sink,
                                                1,
                                                id,
                                                required );
        (*_playerObject)->Realize(_playerObject, SL_BOOLEAN_FALSE);

        /// register callback
        (*_playerObject)  ->GetInterface    (_playerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &_queueInterface);
        (*_queueInterface)->RegisterCallback(_queueInterface, bufferQueueCallback, this);

        /// begin playing
        (*_playerObject)   ->GetInterface(_playerObject, SL_IID_PLAY, &_playerInterface);
        (*_playerInterface)->SetPlayState(_playerInterface, SL_PLAYSTATE_PLAYING);
    }

    bool queue_buffer(short *buffer, SLuint32 size)
    {
        if (nullptr == _queueInterface)
        {
            ALOGE(NDK_TAG, "empty SLAndroidSimpleBufferQueueItf");
            return false;
        }

        _locker.wait();
        (*_queueInterface)->Enqueue( _queueInterface, buffer, size );

        return true;
    }

};


OpenAudio::~OpenAudio()
{
    if(nullptr != mpPlayer)
    {
        delete mpPlayer;
        mpPlayer = nullptr;
    }

    if (nullptr != mpRecorder)
    {
        delete mpRecorder;
        mpRecorder = nullptr;
    }
}

struct record_info
{
    char        path[128];
    OpenAudio  *audio;
};

void *OpenAudio::record_routine(void *arg)
{
    ALOGI(NDK_TAG, "start record thread@%ld.", pthread_self());
    auto *info = (record_info *)arg;
    if (nullptr == info)
    {
        ALOGE(NDK_TAG, "bad record info");
        return nullptr;
    }
    if (info->audio == nullptr)
    {
        ALOGE(NDK_TAG, "bad audio object");
        return nullptr;
    }

    ALOGI(NDK_TAG, "file name:%s", info->path);

    if(info->audio->mpRecorder == nullptr)
    {
        info->audio->mpRecorder = new AudioDevice;
        info->audio->mpRecorder->init_recorder(CHANNELS, convert_sample_rate(SAMPLE_RATE));
    }

    FILE *pFile  = fopen(info->path, "wb");
    auto *buffer = (short *)calloc(BUFFER_NUM, sizeof(short));

    info->audio->mIsRecording = true;
    while(info->audio->mIsRecording)
    {
        bool ret = info->audio->mpRecorder->queue_buffer(buffer, BUFFER_SIZE);
        if (!ret)
        {
            ALOGW(NDK_TAG, "recorder queue data failed");
            goto _END;
        }
        fwrite((unsigned char *)buffer, BUFFER_SIZE, 1, pFile);
    }

    ALOGD(NDK_TAG, "stop record ....");
    if (nullptr != pFile)
    {
        fflush(pFile);
    }

_END:
    info->audio->mIsRecording = false;

    if(nullptr != buffer)
    {
        free(buffer);
        buffer = nullptr;
    }

    if (nullptr != pFile)
    {
        fclose(pFile);
        pFile = nullptr;
    }

    if(nullptr != info)
    {
        delete info;
        info = nullptr;
    }

    pthread_exit(nullptr);
}


void OpenAudio::startRecord(const char *path)
{
    auto *info = new record_info;
    info->audio = this;
    strcpy(info->path, path);

    pthread_create(&tid_record, nullptr, record_routine, info);
    pthread_detach(tid_record);
}

void OpenAudio::stopRecord()
{
    ALOGD(NDK_TAG, "flag to stop record");
    mIsRecording = false;
}

void *OpenAudio::play_routine(void *arg)
{
    ALOGI(NDK_TAG, "start play thread@%ld.", pthread_self());
    auto *info = (record_info *)arg;
    if (nullptr == info)
    {
        ALOGE(NDK_TAG, "bad record info");
        return nullptr;
    }
    if (info->audio == nullptr)
    {
        ALOGE(NDK_TAG, "bad audio object");
        return nullptr;
    }

    ALOGI(NDK_TAG, "file name:%s", info->path);

    if(info->audio->mpPlayer == nullptr)
    {
        info->audio->mpPlayer = new AudioDevice;
        info->audio->mpPlayer->init_player(CHANNELS, convert_sample_rate(SAMPLE_RATE));
    }

    FILE *pFile  = fopen(info->path, "rb");
    auto *buffer = (short *)calloc(BUFFER_NUM, sizeof(short));

    info->audio->mIsPlaying = true;
    while ( info->audio->mIsPlaying )
    {
        if (feof(pFile))
        {
            ALOGD(NDK_TAG, "end of file: %s.", info->path);
            break;
        }

        if(fread((unsigned char *)buffer, BUFFER_SIZE, 1, pFile) != 1)
        {
            ALOGW(NDK_TAG, "read audio file failed");
            goto _END;
        }

        bool ret = info->audio->mpPlayer->queue_buffer(buffer, BUFFER_SIZE);
        if (!ret)
        {
            ALOGW(NDK_TAG, "play queue data failed");
            goto _END;
        }
    }

    ALOGD(NDK_TAG, "end or stop playing ....");

_END:
    if(nullptr == buffer)
    {
        free(buffer);
        buffer = nullptr;
    }

    if(nullptr != pFile)
    {
        fclose(pFile);
        pFile = nullptr;
    }

    if(nullptr != info)
    {
        delete info;
        info = nullptr;
    }

    pthread_exit(nullptr);
}


void OpenAudio::startPlay(const char *path)
{
    auto *info = new record_info;
    info->audio = this;
    strcpy(info->path, path);

    pthread_create(&tid_play, nullptr, play_routine, info);
    pthread_detach(tid_play);
}

void OpenAudio::stopPlay()
{
    ALOGD(NDK_TAG, "flag to stop play");
    mIsPlaying = false;
}
