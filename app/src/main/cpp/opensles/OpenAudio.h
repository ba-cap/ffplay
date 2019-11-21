#ifndef OPEN_AUDIO_H_INCLUDE
#define OPEN_AUDIO_H_INCLUDE

#include <SLES/OpenSLES.h>
#include <pthread.h>

//class ThreadLocker;
class AudioDevice;

class OpenAudio
{

public:
    OpenAudio() = default;
    ~OpenAudio();

private:
    AudioDevice *mpRecorder{nullptr};
    bool         mIsRecording{false};

    AudioDevice *mpPlayer{nullptr};
    bool         mIsPlaying{false};

    pthread_t    tid_record{0};
    pthread_t    tid_play{0};

private:
    static void *record_routine(void *arg);
    static void *play_routine  (void *arg);

public:

    void startRecord(const char *path);
    void stopRecord();
    inline bool isRecording()
    {
        return mIsRecording;
    }

    void startPlay(const char *path);
    void stopPlay();
    inline bool isPlaying()
    {
        return mIsPlaying;
    }
};


#endif//OPEN_AUDIO_H_INCLUDE
