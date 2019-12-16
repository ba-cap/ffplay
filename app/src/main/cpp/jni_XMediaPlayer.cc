#include "ALogger.h"

#include <jni.h>

static const char *TAG = "jni_XMediaPlayer";

extern "C"
JNIEXPORT void JNICALL
Java_dai_android_media_xplayer_XMediaPlayer_native_1setDataSource(
    JNIEnv *env, jobject thiz, jstring url)
{
    const char *cUrl = env->GetStringUTFChars(url, nullptr);


    env->ReleaseStringUTFChars(url, cUrl);
}

extern "C"
JNIEXPORT void JNICALL
Java_dai_android_media_xplayer_XMediaPlayer_native_1setSurface(
    JNIEnv *env, jobject thiz, jobject surface)
{
    // TODO: implement native_setSurface()
}