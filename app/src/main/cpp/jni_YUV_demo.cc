
#include "demo.h"
#include "ALogger.h"

#include <jni.h>
#include <string>
#include <cstring>
#include <EGL/egl.h>
#include <GLES3/gl3.h>


static const char *tag = "jni_yuv";


void play_video_yuv(JNIEnv *env, jobject surface, const char *path)
{
    ALOGD(tag, "play yuv file: %s.", path);

    FILE *file = fopen(path, "rb");
    if(nullptr == file) {
        ALOGE(tag, "open file: %s failed.", path);
        return;
    }
}