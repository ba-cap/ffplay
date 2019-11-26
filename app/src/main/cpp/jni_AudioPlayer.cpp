
//
// https://github.com/bluesky466/OpenSLDemo
//

#include <jni.h>

#include "OpenAudio.h"
#include "ALogger.h"

#define NDK_TAG "jni_AudioPlayer"

static OpenAudio *g_OpenAudio = nullptr;


extern "C" JNIEXPORT void JNICALL
Java_dai_android_media_ffplay_RecorderActivity__1init(JNIEnv *env, jobject clazz)
{
    ALOGD(NDK_TAG, "init");
    if(nullptr == g_OpenAudio)
    {
        g_OpenAudio = new OpenAudio;
        ALOGI(NDK_TAG, "create a new OpenAudio");
    }
}

extern "C" JNIEXPORT void JNICALL
Java_dai_android_media_ffplay_RecorderActivity__1destroy(JNIEnv *env, jobject clazz)
{
    ALOGD(NDK_TAG, "destroy");
    if (nullptr != g_OpenAudio)
    {
        delete  g_OpenAudio;
        g_OpenAudio = nullptr;

        ALOGI(NDK_TAG, "free global OpenAudio");
    }
}

extern "C" JNIEXPORT void JNICALL
Java_dai_android_media_ffplay_RecorderActivity__1stopPlay(JNIEnv *env, jobject clazz)
{
    if(nullptr == g_OpenAudio)
    {
        ALOGW(NDK_TAG, "empty OpenAudio instance");
        return;
    }

    g_OpenAudio->stopPlay();
}

extern "C" JNIEXPORT void JNICALL
Java_dai_android_media_ffplay_RecorderActivity__1stopRecord(JNIEnv *env, jobject clazz)
{
    if (nullptr == g_OpenAudio)
    {
        ALOGW(NDK_TAG, "empty OpenAudio instance");
        return;
    }

    g_OpenAudio->stopRecord();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_dai_android_media_ffplay_RecorderActivity__1isPlaying(JNIEnv *env, jobject clazz)
{
    if(nullptr == g_OpenAudio)
    {
        ALOGW(NDK_TAG, "empty OpenAudio instance");
        return JNI_FALSE;
    }
    return g_OpenAudio->isPlaying() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_dai_android_media_ffplay_RecorderActivity__1isRecord(JNIEnv *env, jobject clazz)
{
    if(nullptr == g_OpenAudio)
    {
        ALOGW(NDK_TAG, "empty OpenAudio instance");
        return JNI_FALSE;
    }
    return g_OpenAudio->isRecording() ? JNI_TRUE : JNI_FALSE;
}


extern "C" JNIEXPORT void JNICALL
Java_dai_android_media_ffplay_RecorderActivity__1startRecord(JNIEnv *env, jobject clazz, jstring file)
{
    const char *c_str_file = env->GetStringUTFChars(file, nullptr);
    ALOGD(NDK_TAG, "start record to file: %s.", c_str_file);

    if(nullptr != g_OpenAudio)
    {
        g_OpenAudio->startRecord(c_str_file);
    }

    env->ReleaseStringUTFChars(file, c_str_file);
}

extern "C" JNIEXPORT void JNICALL
Java_dai_android_media_ffplay_RecorderActivity__1startPlay(JNIEnv *env, jobject clazz, jstring file)
{
    const char *c_str_file = env->GetStringUTFChars(file, nullptr);
    ALOGD(NDK_TAG, "start play from file: %s.", c_str_file);

    if(nullptr != g_OpenAudio)
    {
        g_OpenAudio->startPlay(c_str_file);
    }

    env->ReleaseStringUTFChars(file, c_str_file);
}
