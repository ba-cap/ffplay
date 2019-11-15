
#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"

#ifdef __cplusplus
}
#endif

#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_dai_anroid_media_ffplay_MainActivity_getFfplayInfo(JNIEnv *env, jobject clazz) {
    std::string str(avcodec_configuration());
    return env->NewStringUTF(str.c_str());
}


